/**
 * @file Differential_equations.cpp
 * @brief Implementation of Equilibrium finding, Padé delays, and discretization utilities.
 */
#include "Differential_equations.h"

#include <kinsol/kinsol.h>
#include <nvector/nvector_serial.h>
#include <sunlinsol/sunlinsol_dense.h>
#include <sunmatrix/sunmatrix_dense.h>
#include <sundials/sundials_context.h>
#include <sundials/sundials_types.h>

#include <cmath>
#include <iostream>
#include <stdexcept>


// ===================================================================
//  Helpers: Eigen <-> SUNDIALS
// ===================================================================

static Eigen::VectorXd nv2eigen(N_Vector v, int n) {
    Eigen::VectorXd x(n);
    sunrealtype* d = N_VGetArrayPointer(v);
    for (int i = 0; i < n; ++i) x(i) = static_cast<double>(d[i]);
    return x;
}

static void eigen2nv(N_Vector v, const Eigen::VectorXd& x) {
    sunrealtype* d = N_VGetArrayPointer(v);
    for (int i = 0; i < static_cast<int>(x.size()); ++i)
        d[i] = static_cast<sunrealtype>(x(i));
}

static void eigen2sun(SUNMatrix J, const Eigen::MatrixXd& M) {
    int n = static_cast<int>(M.rows());
    for (int j = 0; j < n; ++j)
        for (int i = 0; i < n; ++i)
            SM_ELEMENT_D(J, i, j) = static_cast<sunrealtype>(M(i, j));
}


// ===================================================================
//  Callback user data
// ===================================================================

struct KINSOLUserData {
    const RHSFunc* rhs;
    const JacFunc* jac;
    Eigen::VectorXd u;
    int n;
};


// ===================================================================
//  KINSOL callbacks
// ===================================================================

static int kinsol_res_cb(N_Vector u, N_Vector fval, void* ud) {
    auto* d = static_cast<KINSOLUserData*>(ud);
    eigen2nv(fval, (*d->rhs)(0.0, nv2eigen(u, d->n), d->u));
    return 0;
}

static int kinsol_jac_cb(N_Vector u, N_Vector, SUNMatrix J, void* ud,
    N_Vector, N_Vector) {
    auto* d = static_cast<KINSOLUserData*>(ud);
    eigen2sun(J, (*d->jac)(0.0, nv2eigen(u, d->n), d->u));
    return 0;
}

static int kinsol_fd_jac_cb(N_Vector u, N_Vector, SUNMatrix J, void* ud,
    N_Vector, N_Vector) {
    auto* d = static_cast<KINSOLUserData*>(ud);
    auto [A, B] = computeJacobians(*d->rhs, nv2eigen(u, d->n), d->u, 0.0);
    (void)B;
    eigen2sun(J, A);
    return 0;
}

static KINSOLConfig withAutoScaling(
    const KINSOLConfig& cfg,
    const RHSFunc& rhs,
    const Eigen::VectorXd& x0,
    const Eigen::VectorXd& u)
{
    KINSOLConfig scaled = cfg;
    if (scaled.x_scale.size() != x0.size()) {
        scaled.x_scale = x0.unaryExpr([](double v) {
            return std::max(1.0, std::abs(v));
        });
    }
    if (scaled.f_scale.size() != x0.size()) {
        const Eigen::VectorXd f0 = rhs(0.0, x0, u);
        scaled.f_scale = f0.cwiseAbs().cwiseMax(1.0);
    }
    return scaled;
}


// ===================================================================
//  KINSOL single-strategy solve (internal)
// ===================================================================

static Eigen::VectorXd kinsolSolve(
    const RHSFunc& rhs, const Eigen::VectorXd& x0,
    const Eigen::VectorXd& u, const KINSOLConfig& cfg,
    const JacFunc& jac)
{
    int n = static_cast<int>(x0.size());

    SUNContext ctx;
    if (SUNContext_Create(SUN_COMM_NULL, &ctx) != 0)
        throw std::runtime_error("SUNContext_Create failed");

    N_Vector y = N_VNew_Serial(static_cast<sunindextype>(n), ctx);
    N_Vector xs = N_VNew_Serial(static_cast<sunindextype>(n), ctx);
    N_Vector fs = N_VNew_Serial(static_cast<sunindextype>(n), ctx);
    eigen2nv(y, x0);

    if (cfg.x_scale.size() == n) eigen2nv(xs, cfg.x_scale);
    else N_VConst(1.0, xs);
    if (cfg.f_scale.size() == n) eigen2nv(fs, cfg.f_scale);
    else N_VConst(1.0, fs);

    void* kin = KINCreate(ctx);
    KINSOLUserData ud = { &rhs, &jac, u, n };
    KINSetUserData(kin, &ud);
    KINInit(kin, kinsol_res_cb, y);
    KINSetFuncNormTol(kin, static_cast<sunrealtype>(cfg.ftol));
    KINSetScaledStepTol(kin, static_cast<sunrealtype>(cfg.stol));
    KINSetNumMaxIters(kin, cfg.max_iter);

    if (cfg.damping < 1.0 && cfg.damping > 0.0)
        KINSetDamping(kin, static_cast<sunrealtype>(cfg.damping));
    if (cfg.strategy == KINSOLStrategy::FixedPoint && cfg.maa > 0)
        KINSetMAA(kin, static_cast<long>(cfg.maa));

    SUNMatrix A = nullptr;
    SUNLinearSolver LS = nullptr;
    if (cfg.strategy != KINSOLStrategy::FixedPoint) {
        A = SUNDenseMatrix(static_cast<sunindextype>(n), static_cast<sunindextype>(n), ctx);
        LS = SUNLinSol_Dense(y, A, ctx);
        KINSetLinearSolver(kin, LS, A);
        if (cfg.use_analytical_jac && jac)
            KINSetJacFn(kin, kinsol_jac_cb);
        else
            KINSetJacFn(kin, kinsol_fd_jac_cb);
    }

    int strat;
    switch (cfg.strategy) {
    case KINSOLStrategy::Newton:     strat = KIN_NONE;       break;
    case KINSOLStrategy::LineSearch:  strat = KIN_LINESEARCH; break;
    case KINSOLStrategy::Picard:     strat = KIN_PICARD;     break;
    case KINSOLStrategy::FixedPoint: strat = KIN_FP;         break;
    default:                         strat = KIN_LINESEARCH; break;
    }

    int flag = KINSol(kin, y, strat, xs, fs);

    Eigen::VectorXd result;
    if (flag >= 0) {
        result = nv2eigen(y, n);
        long nfe = 0, nni = 0;
        KINGetNumFuncEvals(kin, &nfe);
        KINGetNumNonlinSolvIters(kin, &nni);
        std::cout << "[KINSOL] Evals:" << nfe << " Iters:" << nni << "\n";
    }

    N_VDestroy(y); N_VDestroy(xs); N_VDestroy(fs);
    KINFree(&kin);
    if (LS) SUNLinSolFree(LS);
    if (A)  SUNMatDestroy(A);
    SUNContext_Free(&ctx);

    if (flag < 0) {
        std::string msg = "KINSOL flag " + std::to_string(flag);
        if (flag == KIN_LINESEARCH_NONCONV) msg += " (line search failed)";
        else if (flag == KIN_MAXITER_REACHED) msg += " (max iterations)";
        throw std::runtime_error(msg);
    }
    return result;
}


// ===================================================================
//  PUBLIC: findEquilibriumRobust — cascade
// ===================================================================

Eigen::VectorXd findEquilibriumRobust(
    const RHSFunc& rhs, const Eigen::VectorXd& x0,
    const Eigen::VectorXd& u, const JacFunc& jac)
{
    if (x0.size() == 0 || u.size() == 0)
        throw std::invalid_argument("x0 and u must be non-empty.");

    // 1. Newton (MMC JSON/GFL often fails KIN_LINESEARCH then recovers here)
    try {
        KINSOLConfig c;
        c.strategy = KINSOLStrategy::Newton;
        c.max_iter = 300;
        c = withAutoScaling(c, rhs, x0, u);
        std::cout << "[Robust] Newton...\n";
        return kinsolSolve(rhs, x0, u, c, jac);
    }
    catch (const std::exception& e) {
        std::cout << "[Robust] Newton failed: " << e.what() << "\n";
    }

    // 2. LineSearch
    try {
        KINSOLConfig c;
        c.strategy = KINSOLStrategy::LineSearch;
        c.max_iter = 300;
        c = withAutoScaling(c, rhs, x0, u);
        std::cout << "[Robust] LineSearch...\n";
        return kinsolSolve(rhs, x0, u, c, jac);
    }
    catch (const std::exception& e) {
        std::cout << "[Robust] LineSearch failed: " << e.what() << "\n";
    }

    // 3. Relaxed warmup → tight
    try {
        KINSOLConfig cw;
        cw.strategy = KINSOLStrategy::LineSearch;
        cw.ftol = 1e-4;
        cw.stol = 1e-4;
        cw.max_iter = 500;
        cw = withAutoScaling(cw, rhs, x0, u);
        std::cout << "[Robust] Relaxed warmup...\n";
        auto xw = kinsolSolve(rhs, x0, u, cw, jac);

        KINSOLConfig ct;
        ct.strategy = KINSOLStrategy::Newton;
        ct.max_iter = 200;
        ct = withAutoScaling(ct, rhs, xw, u);
        std::cout << "[Robust] Tight finish...\n";
        return kinsolSolve(rhs, xw, u, ct, jac);
    }
    catch (const std::exception& e) {
        std::cout << "[Robust] Warmup/Newton failed: " << e.what() << "\n";
    }

    // 4. Damped Picard → Newton
    try {
        KINSOLConfig cp;
        cp.strategy = KINSOLStrategy::Picard;
        cp.damping = 0.5;
        cp.max_iter = 2000;
        cp.ftol = 1e-8;
        cp.stol = 1e-8;
        cp = withAutoScaling(cp, rhs, x0, u);
        std::cout << "[Robust] Damped Picard...\n";
        auto xp = kinsolSolve(rhs, x0, u, cp, jac);

        KINSOLConfig ct;
        ct.strategy = KINSOLStrategy::Newton;
        ct.max_iter = 100;
        ct = withAutoScaling(ct, rhs, xp, u);
        return kinsolSolve(rhs, xp, u, ct, jac);
    }
    catch (const std::exception& e) {
        std::cout << "[Robust] Picard/Newton failed: " << e.what() << "\n";
    }

    throw std::runtime_error("[KINSOL] All strategies exhausted.");
}


// ===================================================================
//  PUBLIC: computeJacobians — central differences, adaptive step
// ===================================================================

std::pair<Eigen::MatrixXd, Eigen::MatrixXd> computeJacobians(
    const RHSFunc& rhs, const Eigen::VectorXd& x,
    const Eigen::VectorXd& u, double t, double eps)
{
    const int nx = static_cast<int>(x.size());
    const int nu = static_cast<int>(u.size());
    const int nf = static_cast<int>(rhs(t, x, u).size());

    Eigen::MatrixXd A(nf, nx), B(nf, nu);

    for (int i = 0; i < nx; ++i) {
        double h = eps * std::max(1.0, std::abs(x(i)));
        Eigen::VectorXd xp = x, xm = x;
        xp(i) += h; xm(i) -= h;
        A.col(i) = (rhs(t, xp, u) - rhs(t, xm, u)) / (2.0 * h);
    }
    for (int j = 0; j < nu; ++j) {
        double h = eps * std::max(1.0, std::abs(u(j)));
        Eigen::VectorXd up = u, um = u;
        up(j) += h; um(j) -= h;
        B.col(j) = (rhs(t, x, up) - rhs(t, x, um)) / (2.0 * h);
    }
    return { A, B };
}


// ===================================================================
//  Padé delay
// ===================================================================

void padeDelaySystemMulti3(double T, MatrixXd& Ad, MatrixXd& Bd, MatrixXd& Cd, MatrixXd& Dd, int ns) {
    double a3 = T * T * T, a2 = 12 * T * T, a1 = 60 * T, a0 = 120;
    double b3 = -T * T * T, b2 = 12 * T * T, b1 = -60 * T, b0 = 120;
    Eigen::MatrixXd A(3, 3); A << -a2 / a3, -a1 / a3, -a0 / a3, 1, 0, 0, 0, 1, 0;
    Eigen::MatrixXd B(3, 1); B << 1, 0, 0;
    Eigen::MatrixXd C(1, 3); C << (b2 - b3 * a2 / a3) / a3, (b1 - b3 * a1 / a3) / a3, (b0 - b3 * a0 / a3) / a3;
    Eigen::MatrixXd D(1, 1); D << b3 / a3;
    for (int i = 0; i < ns; ++i) { Ad.block(i * 3, i * 3, 3, 3) = A; Bd.block(i * 3, i, 3, 1) = B; Cd.block(i, i * 3, 1, 3) = C; Dd(i, i) = D(0, 0); }
}

void padeDelaySystemMulti2(double T, MatrixXd& Ad, MatrixXd& Bd, MatrixXd& Cd, MatrixXd& Dd, int ns) {
    double a2 = T * T, a1 = 6 * T, a0 = 12, b2 = -T * T, b1 = 6 * T, b0 = -12;
    Eigen::MatrixXd A(2, 2); A << -a1 / a2, -a0 / a2, 1, 0;
    Eigen::MatrixXd B(2, 1); B << 1, 0;
    Eigen::MatrixXd C(1, 2); C << (b1 - b2 * a1 / a2) / a2, (b0 - b2 * a0 / a2) / a2;
    Eigen::MatrixXd D(1, 1); D << b2 / a2;
    for (int i = 0; i < ns; ++i) { 
        Ad.block(i * 2, i * 2, 2, 2) = A; 
        Bd.block(i * 2, i, 2, 1) = B; 
        Cd.block(i, i * 2, 1, 2) = C; 
        Dd(i, i) = D(0, 0); 
    }
}


// ===================================================================
//  Discretization (Tustin / bilinear)
// ===================================================================

void discretizeABCD(
    const Eigen::MatrixXd& A, const Eigen::MatrixXd& B,
    const Eigen::MatrixXd& C, const Eigen::MatrixXd& D, double Ts,
    Eigen::MatrixXd& Ad, Eigen::MatrixXd& Bd,
    Eigen::MatrixXd& Cd, Eigen::MatrixXd& Dd)
{
    if (Ts <= 0) throw std::invalid_argument("Ts must be positive.");
    if (A.rows() == 0 || A.cols() == 0) throw std::runtime_error("A is empty.");
    if (A.rows() != A.cols()) throw std::runtime_error("A must be square.");
    if (B.rows() != A.rows()) throw std::runtime_error("B rows != A rows.");
    if (C.cols() != A.cols()) throw std::runtime_error("C cols != A cols.");
    if (D.rows() != C.rows() || D.cols() != B.cols()) throw std::runtime_error("D mismatch.");

    const int n = static_cast<int>(A.rows());
    Eigen::MatrixXd I = Eigen::MatrixXd::Identity(n, n);
    Eigen::PartialPivLU<Eigen::MatrixXd> S(I - 0.5 * Ts * A);
    Ad = S.solve(I + 0.5 * Ts * A);
    Bd = S.solve(B) * std::sqrt(Ts);
    Cd = std::sqrt(Ts) * C * S.solve(I);
    Dd = D + C * S.solve(0.5 * Ts * B);
}