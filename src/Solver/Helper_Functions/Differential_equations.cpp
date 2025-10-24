#include "Differential_equations.h"

using DerivFunc = std::function<Eigen::VectorXd(const Eigen::VectorXd&, const Eigen::VectorXd&)>;

/**
 * @brief Finds a local equilibrium point x such that f(x, u) = 0 using a damped Newton-Raphson method with pseudo-inverse for stiff systems.
 *
 * Iteratively solves for the steady-state operating point of a nonlinear system.
 * Robust to stiff and ill-conditioned systems by using the pseudo-inverse (SVD) for the Jacobian
 * and a line search to damp the Newton step. The solution found is local and depends on the initial guess.
 *
 * @param x0 Initial guess for the state vector (Eigen::VectorXd).
 * @param u Input vector (Eigen::VectorXd) for the system.
 * @param f Function object representing the system equations f(x, u) (returns Eigen::VectorXd).
 * @param tol Convergence tolerance for the residual and Newton step.
 * @param max_iter Maximum number of Newton iterations.
 * @return Eigen::VectorXd The equilibrium state vector x such that f(x, u) ≈ 0.
 *
 * @throws std::invalid_argument if x0 or u are empty.
 * @note For nonlinear systems with multiple equilibria, the result depends on the initial guess x0.
 */
Eigen::VectorXd findEquilibrium(const Eigen::VectorXd& x0, const Eigen::VectorXd& u, DerivFunc f,
    double tol, int max_iter) {

    Eigen::VectorXd x = x0;
    const int n = x.size();
    if (n == 0 || u.size() == 0) {
        throw std::invalid_argument("Initial state and input vectors must be non-empty.");
    }

    const double eps = 1e-6;
    double lambda = 1e-3;

    for (int iter = 0; iter < max_iter; ++iter) {
        VectorXd fx = f(x, u);
        double fx_norm = fx.norm();

        //std::cout << "Iter " << iter
        //    << " | ||f(x)|| = " << fx_norm
        //    << " | lambda = " << lambda << std::endl;

        if (fx_norm < tol) {
            std::cout << "Converged (residual tolerance) in " << iter << " iterations.\n";
            return x;
        }

        // Numerical Jacobian (central difference)
        MatrixXd J(fx.size(), n);
        for (int i = 0; i < n; ++i) {
            VectorXd x_eps_plus = x;
            VectorXd x_eps_minus = x;
            x_eps_plus(i) += eps;
            x_eps_minus(i) -= eps;

            VectorXd f_plus = f(x_eps_plus, u);
            VectorXd f_minus = f(x_eps_minus, u);

            J.col(i) = (f_plus - f_minus) / (2 * eps);
        }

        // Levenberg-Marquardt step: (JᵀJ + λI) dx = -Jᵀ f
        MatrixXd JTJ = J.transpose() * J;
        VectorXd JTr = J.transpose() * fx;
        MatrixXd H = JTJ + lambda * MatrixXd::Identity(n, n);

        // Solve for dx
        VectorXd dx = -H.ldlt().solve(JTr);

        if (dx.norm() < tol) {
            std::cout << "Converged (small step) in " << iter << " iterations.\n";
            return x;
        }

        VectorXd x_trial = x + dx;
        VectorXd fx_trial = f(x_trial, u);
        double fx_trial_norm = fx_trial.norm();

        // Compute gain ratio rho
        double denom = dx.transpose() * (lambda * dx - JTr);
        if (std::abs(denom) < 1e-12) denom = (denom < 0 ? -1e-12 : 1e-12); // prevent div0
        double rho = (fx.squaredNorm() - fx_trial.squaredNorm()) / denom;

        if (rho > 0) {
            // Accept step, reduce lambda
            x = x_trial;
            lambda *= std::max(1.0 / 3.0, 1.0 - std::pow(2 * rho - 1, 3));
            lambda = std::max(lambda, 1e-12);
        }
        else {
            // Reject step, increase lambda
            lambda *= 10.0;
        }
    }
	std::cerr << "Solver did not converge after " << max_iter << " iterations. Final residual: " << f(x, u).norm() << "\n";
    return x;  // Return last iterate anyway
}

// Wrapper around Eigen's Levenberg–Marquardt
Eigen::VectorXd findEquilibriumLM(
    const Eigen::VectorXd& x0,
    const Eigen::VectorXd& u,
    DerivFunc f,
    double tol,
    int max_iter)
{
    // Wrap into functor
    SystemFunctor functor(f, u, static_cast<int>(x0.size()));

    // Use numerical differentiation to compute Jacobian
    Eigen::NumericalDiff<SystemFunctor> numDiff(functor);

    // Levenberg-Marquardt solver
    Eigen::LevenbergMarquardt<Eigen::NumericalDiff<SystemFunctor>, double> lm(numDiff);

    // Tolerances and max function evaluations
    lm.parameters.ftol = tol;       // function tolerance
    lm.parameters.xtol = tol;       // parameter tolerance
    lm.parameters.maxfev = max_iter;

    Eigen::VectorXd x = x0;
    Eigen::LevenbergMarquardtSpace::Status status = lm.minimize(x);

    // Print a short summary
    std::cout << "LM solver status: " << int(status)
        << " | final residual norm: " << f(x, u).norm() << std::endl;

    return x;
}


/**
 * @brief Numerically computes the Jacobian matrices A = ∂f/∂x and B = ∂f/∂u using finite differences.
 *
 * Evaluates the system function f at perturbed values of x and u to estimate the partial derivatives.
 *
 * @param x State vector at which to compute the Jacobians.
 * @param u Input vector at which to compute the Jacobians.
 * @param f Function object representing the system equations f(x, u).
 * @param eps Perturbation size for finite differences.
 * @return Pair of Jacobian matrices (A, B).
 */
std::pair<Eigen::MatrixXd, Eigen::MatrixXd> computeJacobians(
    const Eigen::VectorXd& x, const Eigen::VectorXd& u, DerivFunc f, double eps) {
    const int n_x = x.size();
    const int n_u = u.size();
    const int n_f = f(x, u).size();

    Eigen::MatrixXd A(n_f, n_x);  // ∂f/∂x
    Eigen::MatrixXd B(n_f, n_u);  // ∂f/∂u

    Eigen::VectorXd f_base = f(x, u);

    // Compute ∂f/∂x
    for (int i = 0; i < n_x; ++i) {
        Eigen::VectorXd x_eps = x;
        x_eps(i) += eps;
        Eigen::VectorXd f_eps = f(x_eps, u);
        A.col(i) = (f_eps - f_base) / eps;
    }

    // Compute ∂f/∂u
    for (int j = 0; j < n_u; ++j) {
        Eigen::VectorXd u_eps = u;
        u_eps(j) += eps;
        Eigen::VectorXd f_eps = f(x, u_eps);
        B.col(j) = (f_eps - f_base) / eps;
    }

    return { A, B };
}

/**
 * @brief Generates the state-space matrices (A, B, C, D) for a single delay using a 3rd-order Padé approximation.
 *
 * Approximates a time delay in the system using Padé coefficients and returns the corresponding matrices.
 *
 * @param tdelay Time delay to approximate.
 * @return Pair of pairs: ((A, B), (C, D)) matrices for the delay system.
 */
void padeDelaySystem3(double tdelay, MatrixXd& Adelay, MatrixXd& Bdelay, MatrixXd& Cdelay, MatrixXd& Ddelay) {
    const double T = tdelay;

    // Padé 3rd-order approximation coefficients
    double a3 = T * T * T;
    double a2 = 12 * T * T;
    double a1 = 60 * T;
    double a0 = 120.0;

    double b3 = -T * T * T;
    double b2 = 12 * T * T;
    double b1 = -60 * T;
    double b0 = 120.0;

    // A matrix (companion form)
    Adelay << -a2 / a3, -a1 / a3, -a0 / a3,
        1, 0, 0,
        0, 1, 0;

    // B matrix
    Bdelay << 1, 0, 0;

    // C matrix
    Cdelay << (b2 - b3 * a2 / a3) / a3,
        (b1 - b3 * a1 / a3) / a3,
        (b0 - b3 * a0 / a3) / a3;

    // D matrix
    Ddelay << b3 / a3;
}

/**
 * @brief Generates block state-space matrices for multiple delayed signals using 3rd-order Padé approximation.
 *
 * Constructs block matrices for N delayed signals, each approximated by a 3rd-order Padé delay.
 *
 * @param tdelay Time delay to approximate.
 * @param num_signals Number of delayed signals.
 * @return Pair of pairs: ((A, B), (C, D)) block matrices for the multi-delay system.
 */
void padeDelaySystemMulti3(double tdelay, MatrixXd& Adelay, MatrixXd& Bdelay, MatrixXd& Cdelay, MatrixXd& Ddelay, int num_signals) {
    const double T = tdelay;

    // Padé 3rd-order approximation coefficients
    double a3 = T * T * T;
    double a2 = 12 * T * T;
    double a1 = 60 * T;
    double a0 = 120.0;

    double b3 = -T * T * T;
    double b2 = 12 * T * T;
    double b1 = -60 * T;
    double b0 = 120.0;

    // Single A, B, C, D matrices
	Eigen::MatrixXd A(3, 3);
    A << -a2 / a3, -a1 / a3, -a0 / a3,
        1, 0, 0,
        0, 1, 0;

	Eigen::MatrixXd B(3, 1);
    B << 1, 0, 0;

    Eigen::MatrixXd C(1, 3);
    C << (b2 - b3 * a2 / a3) / a3,
        (b1 - b3 * a1 / a3) / a3,
        (b0 - b3 * a0 / a3) / a3;

	Eigen::MatrixXd D(1, 1);
    D << b3 / a3;

    // Dimensions
    const int n = 3 * num_signals;

    for (int i = 0; i < num_signals; ++i) {
        int row = i * 3;
        int col = i;

        Adelay.block(row, row, 3, 3) = A;
        Bdelay.block(row, col, 3, 1) = B;
        Cdelay.block(i, row, 1, 3) = C;
        Ddelay(i, col) = D(0, 0);
    }
}


/**
 * @brief Generates the state-space matrices (A, B, C, D) for a single delay using a 2nd-order Padé approximation.
 *
 * Approximates a time delay in the system using Padé coefficients and returns the corresponding matrices.
 *
 * @param tdelay Time delay to approximate.
 * @return Pair of pairs: ((A, B), (C, D)) matrices for the delay system.
 */
void padeDelaySystem2(double tdelay, MatrixXd& Adelay, MatrixXd& Bdelay, MatrixXd& Cdelay, MatrixXd& Ddelay) {
    const double T = tdelay;

    // Padé 2nd-order approximation coefficients
    double a2 = T * T;
    double a1 = 6 * T;
    double a0 = 12.0;

    double b2 = -T * T;
    double b1 = 6 * T;
    double b0 = -12.0;

    // A matrix (companion form)
    Adelay << -a1 / a2, -a0 / a2,
        1, 0;

    // B matrix
    Bdelay << 1, 0;

    // C matrix
    Cdelay << (b1 - b2 * a1 / a2) / a2,
        (b0 - b2 * a0 / a2) / a2;

    // D matrix
    Ddelay << b2 / a2;
}

/**
 * @brief Generates block state-space matrices for multiple delayed signals using 2nd-order Padé approximation.
 *
 * Constructs block matrices for N delayed signals, each approximated by a 2nd-order Padé delay.
 *
 * @param tdelay Time delay to approximate.
 * @param num_signals Number of delayed signals.
 * @return Pair of pairs: ((A, B), (C, D)) block matrices for the multi-delay system.
 */
void padeDelaySystemMulti2(double tdelay, MatrixXd& Adelay, MatrixXd& Bdelay, MatrixXd& Cdelay, MatrixXd& Ddelay, int num_signals) {
    const double T = tdelay;

    // Padé 2nd-order approximation coefficients
    double a2 = T * T;
    double a1 = 6 * T;
    double a0 = 12.0;

    double b2 = -T * T;
    double b1 = 6 * T;
    double b0 = -12.0;

    // Single A, B, C, D matrices
    Eigen::MatrixXd A(2, 2);
    A << -a1 / a2, -a0 / a2,
        1, 0;

    Eigen::MatrixXd B(2, 1);
    B << 1, 0;

    Eigen::MatrixXd C(1, 2);
    C << (b1 - b2 * a1 / a2) / a2,
        (b0 - b2 * a0 / a2) / a2;

    Eigen::MatrixXd D(1, 1);
    D << b2 / a2;

    // Dimensions
    const int n = 2 * num_signals;

    // Initialize block matrices

    for (int i = 0; i < num_signals; ++i) {
        int row = i * 2;
        int col = i;

        Adelay.block(row, row, 2, 2) = A;
        Bdelay.block(row, col, 2, 1) = B;
        Cdelay.block(i, row, 1, 2) = C;
        Ddelay(i, col) = D(0, 0);
    }
}