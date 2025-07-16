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

    for (int iter = 0; iter < max_iter; ++iter) {
        Eigen::VectorXd f_val = f(x, u);
        double norm_f = f_val.norm();
        if (norm_f < tol) {
            std::cout << "Equilibrium found in " << iter << " iterations. Residual = " << norm_f << "\n";
            return x;
        }

        // Jacobian ∂f/∂x (numerical, adaptive epsilon)
        double eps_base = 1e-7;
        Eigen::MatrixXd J(n, n);
        for (int i = 0; i < n; ++i) {
            Eigen::VectorXd x_eps = x;
            double eps = eps_base * std::max(1.0, std::abs(x(i)));
            x_eps(i) += eps;
            J.col(i) = (f(x_eps, u) - f_val) / eps;
        }

        // Use pseudo-inverse for stiff/ill-conditioned Jacobian
        Eigen::JacobiSVD<Eigen::MatrixXd> svd(J, Eigen::ComputeThinU | Eigen::ComputeThinV);
        double pinvtol = 1e-8;
        Eigen::VectorXd S = svd.singularValues();
        Eigen::MatrixXd S_inv = Eigen::MatrixXd::Zero(n, n);
        for (int i = 0; i < n; ++i) {
            if (S(i) > pinvtol) S_inv(i, i) = 1.0 / S(i);
        }
        Eigen::MatrixXd J_pinv = svd.matrixV() * S_inv * svd.matrixU().transpose();
        Eigen::VectorXd dx = J_pinv * (-f_val);

        // Damped Newton step (line search)
        double alpha = 1.0;
        Eigen::VectorXd x_new;
        double norm_f_new;
        for (int ls = 0; ls < 10; ++ls) {
            x_new = x + alpha * dx;
            norm_f_new = f(x_new, u).norm();
            if (norm_f_new < norm_f) {
                break; // Accept step
            }
            alpha *= 0.5; // Reduce step size
        }
        x = x_new;

        if (dx.norm() < tol) {
            std::cout << "Converged (Newton step small) in " << iter << " iterations.\n";
            return x;
        }

        if (iter == max_iter - 1) {
            std::cerr << "WARNING: Equilibrium solver did not converge after " << max_iter << " iterations. Final residual: " << norm_f << "\n";
        }
    }

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
std::pair<std::pair<Eigen::MatrixXd, Eigen::MatrixXd>, std::pair<Eigen::MatrixXd, Eigen::MatrixXd>>
    padeDelaySystem3(double tdelay) {
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
    Eigen::MatrixXd A(3, 3);
    A << -a2 / a3, -a1 / a3, -a0 / a3,
        1, 0, 0,
        0, 1, 0;

    // B matrix
    Eigen::MatrixXd B(3, 1);
    B << 1, 0, 0;

    // C matrix
    Eigen::MatrixXd C(1, 3);
    C << (b2 - b3 * a2 / a3) / a3,
        (b1 - b3 * a1 / a3) / a3,
        (b0 - b3 * a0 / a3) / a3;

    // D matrix
    Eigen::MatrixXd D(1, 1);
    D << b3 / a3;

    return { {A, B}, {C, D} };
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
std::pair<std::pair<Eigen::MatrixXd, Eigen::MatrixXd>, std::pair<Eigen::MatrixXd, Eigen::MatrixXd>>
    padeDelaySystemMulti3(double tdelay, int num_signals) {
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

    // Initialize block matrices
    Eigen::MatrixXd Adelay = Eigen::MatrixXd::Zero(n, n);
    Eigen::MatrixXd Bdelay = Eigen::MatrixXd::Zero(n, num_signals);
    Eigen::MatrixXd Cdelay = Eigen::MatrixXd::Zero(num_signals, n);
    Eigen::MatrixXd Ddelay = Eigen::MatrixXd::Zero(num_signals, num_signals);

    for (int i = 0; i < num_signals; ++i) {
        int row = i * 3;
        int col = i;

        Adelay.block(row, row, 3, 3) = A;
        Bdelay.block(row, col, 3, 1) = B;
        Cdelay.block(i, row, 1, 3) = C;
        Ddelay(i, col) = D(0, 0);
    }

    return { {Adelay, Bdelay}, {Cdelay, Ddelay} };
}


/**
 * @brief Generates the state-space matrices (A, B, C, D) for a single delay using a 2nd-order Padé approximation.
 *
 * Approximates a time delay in the system using Padé coefficients and returns the corresponding matrices.
 *
 * @param tdelay Time delay to approximate.
 * @return Pair of pairs: ((A, B), (C, D)) matrices for the delay system.
 */
std::pair<std::pair<Eigen::MatrixXd, Eigen::MatrixXd>, std::pair<Eigen::MatrixXd, Eigen::MatrixXd>>
padeDelaySystem2(double tdelay) {
    const double T = tdelay;

    // Padé 2nd-order approximation coefficients
    double a2 = T * T;
    double a1 = 6 * T;
    double a0 = 12.0;

    double b2 = -T * T;
    double b1 = 6 * T;
    double b0 = -12.0;

    // A matrix (companion form)
    Eigen::MatrixXd A(2, 2);
    A << -a1 / a2, -a0 / a2,
        1, 0;

    // B matrix
    Eigen::MatrixXd B(2, 1);
    B << 1, 0;

    // C matrix
    Eigen::MatrixXd C(1, 2);
    C << (b1 - b2 * a1 / a2) / a2,
        (b0 - b2 * a0 / a2) / a2;

    // D matrix
    Eigen::MatrixXd D(1, 1);
    D << b2 / a2;

    return { {A, B}, {C, D} };
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
std::pair<std::pair<Eigen::MatrixXd, Eigen::MatrixXd>, std::pair<Eigen::MatrixXd, Eigen::MatrixXd>>
padeDelaySystemMulti2(double tdelay, int num_signals) {
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
    Eigen::MatrixXd Adelay = Eigen::MatrixXd::Zero(n, n);
    Eigen::MatrixXd Bdelay = Eigen::MatrixXd::Zero(n, num_signals);
    Eigen::MatrixXd Cdelay = Eigen::MatrixXd::Zero(num_signals, n);
    Eigen::MatrixXd Ddelay = Eigen::MatrixXd::Zero(num_signals, num_signals);

    for (int i = 0; i < num_signals; ++i) {
        int row = i * 2;
        int col = i;

        Adelay.block(row, row, 2, 2) = A;
        Bdelay.block(row, col, 2, 1) = B;
        Cdelay.block(i, row, 1, 2) = C;
        Ddelay(i, col) = D(0, 0);
    }

    return { {Adelay, Bdelay}, {Cdelay, Ddelay} };
}