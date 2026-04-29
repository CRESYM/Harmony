#ifndef _DIFFERENTIAL_EQUATIONS_H_
#define _DIFFERENTIAL_EQUATIONS_H_

#include "../../Constants.h"


// ===================================================================
//  Function signatures
// ===================================================================

/// dx/dt = f(t, x, u)
using RHSFunc = std::function<Eigen::VectorXd(double t,
    const Eigen::VectorXd& x, const Eigen::VectorXd& u)>;

/// J = df/dx(t, x, u)
using JacFunc = std::function<Eigen::MatrixXd(double t,
    const Eigen::VectorXd& x, const Eigen::VectorXd& u)>;


// ===================================================================
//  CVODE — stiff BDF integrator
// ===================================================================

struct CVODEConfig {
    double rtol = 1e-8;
    double atol = 1e-10;
    double dt_max = 1e-3;
    bool use_analytical_jac = false;
};

struct CVODEResult {
    std::vector<double> time;
    std::vector<Eigen::VectorXd> states;
};

CVODEResult integrate(
    const RHSFunc& rhs,
    const Eigen::VectorXd& x0,
    const std::function<Eigen::VectorXd(double t)>& inputFn,
    double t0, double tEnd, double dt_output,
    const CVODEConfig& cfg = {},
    const JacFunc& jac = nullptr);


// ===================================================================
//  KINSOL — nonlinear equilibrium solver
// ===================================================================

enum class KINSOLStrategy {
    Newton,
    LineSearch,
    Picard,
    FixedPoint
};

struct KINSOLConfig {
    double ftol = 1e-10;
    double stol = 1e-10;
    int max_iter = 200;
    bool use_analytical_jac = false;
    KINSOLStrategy strategy = KINSOLStrategy::LineSearch;
    double maa = 0;
    double damping = 1.0;
    Eigen::VectorXd x_scale;
    Eigen::VectorXd f_scale;
};

/// Single-strategy solve
Eigen::VectorXd findEquilibrium(
    const RHSFunc& rhs,
    const Eigen::VectorXd& x0,
    const Eigen::VectorXd& u,
    const KINSOLConfig& cfg = {},
    const JacFunc& jac = nullptr);

/// Robust cascade: LineSearch → Newton → relaxed warmup → Picard + Newton
Eigen::VectorXd findEquilibriumRobust(
    const RHSFunc& rhs,
    const Eigen::VectorXd& x0,
    const Eigen::VectorXd& u,
    const JacFunc& jac = nullptr);


// ===================================================================
//  Jacobian computation — central differences, adaptive step
// ===================================================================

std::pair<Eigen::MatrixXd, Eigen::MatrixXd> computeJacobians(
    const RHSFunc& rhs,
    const Eigen::VectorXd& x,
    const Eigen::VectorXd& u,
    double t = 0.0,
    double eps = 1e-8);


// ===================================================================
//  Padé delay approximations
// ===================================================================

void padeDelaySystem3(double tdelay, Eigen::MatrixXd& A, Eigen::MatrixXd& B,
    Eigen::MatrixXd& C, Eigen::MatrixXd& D);
void padeDelaySystemMulti3(double tdelay, Eigen::MatrixXd& A, Eigen::MatrixXd& B,
    Eigen::MatrixXd& C, Eigen::MatrixXd& D, int num_signals);
void padeDelaySystem2(double tdelay, Eigen::MatrixXd& A, Eigen::MatrixXd& B,
    Eigen::MatrixXd& C, Eigen::MatrixXd& D);
void padeDelaySystemMulti2(double tdelay, Eigen::MatrixXd& A, Eigen::MatrixXd& B,
    Eigen::MatrixXd& C, Eigen::MatrixXd& D, int num_signals);


// ===================================================================
//  Discretization (Tustin / bilinear)
// ===================================================================

void discretizeABCD(
    const Eigen::MatrixXd& A, const Eigen::MatrixXd& B,
    const Eigen::MatrixXd& C, const Eigen::MatrixXd& D,
    double Ts,
    Eigen::MatrixXd& Ad, Eigen::MatrixXd& Bd,
    Eigen::MatrixXd& Cd, Eigen::MatrixXd& Dd);


#endif