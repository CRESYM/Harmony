#include "Differential_equations.h"

// Solves f(x,u) = 0 for a steady-state operating point x, using Newton-Raphson
using DerivFunc = std::function<Eigen::VectorXd(const Eigen::VectorXd&, const Eigen::VectorXd&)>;
Eigen::VectorXd findEquilibrium(const Eigen::VectorXd& x0, const Eigen::VectorXd& u, DerivFunc f,
    double tol, int max_iter) {
    Eigen::VectorXd x = x0;
    const int n = x.size();

    for (int iter = 0; iter < max_iter; ++iter) {
        Eigen::VectorXd f_val = f(x, u);
        double norm_f = f_val.norm();
        if (norm_f < tol) {
            std::cout << "Equilibrium found in " << iter << " iterations. Residual = " << norm_f << "\n";
            break;
        }

        // Jacobian ∂f/∂x (numerical)
        double eps = 1e-10;
        Eigen::MatrixXd J(n, n);
        for (int i = 0; i < n; ++i) {
            Eigen::VectorXd x_eps = x;
            x_eps(i) += eps;
            J.col(i) = (f(x_eps, u) - f_val) / eps;
        }

        Eigen::VectorXd dx = J.fullPivLu().solve(-f_val);
        x += dx;

        if (dx.norm() < tol) {
            std::cout << "Converged (Newton step small) in " << iter << " iterations.\n";
            break;
        }

        if (iter == max_iter - 1) {
            std::cerr << "WARNING: Equilibrium solver did not converge.\n";
        }
    }

    return x;
}

using DerivFunc = std::function<Eigen::VectorXd(const Eigen::VectorXd&, const Eigen::VectorXd&)>;
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


// Pade approximation for the differential equation
std::pair<std::pair<Eigen::MatrixXd, Eigen::MatrixXd>, std::pair<Eigen::MatrixXd, Eigen::MatrixXd>>
    padeDelaySystem(double tdelay) {
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

// Pade approximation for multiple delayed signals
// Build block state-space for N delayed signals using 3rd-order Padé
std::pair<std::pair<Eigen::MatrixXd, Eigen::MatrixXd>, std::pair<Eigen::MatrixXd, Eigen::MatrixXd>>
    padeDelaySystemMulti(double tdelay, int num_signals) {
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
