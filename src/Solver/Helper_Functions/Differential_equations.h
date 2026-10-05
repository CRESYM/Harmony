#ifndef _DIFFERENTIAL_EQUATIONS_H_
#define _DIFFERENTIAL_EQUATIONS_H_

/**
 * @file Differential_equations.h
 * @brief Equilibrium finding, Padé delays, and discretization utilities.
 *
 * Wraps KINSOL (nonlinear equilibrium solvers), finite-difference Jacobian
 * computation, Padé delay approximations, and Tustin (bilinear) discretization
 * of state-space models.
 */

#include "core/Constants.h"

/// Right-hand side of dx/dt = f(t, x, u).
using RHSFunc = std::function<Eigen::VectorXd(double t,
    const Eigen::VectorXd& x, const Eigen::VectorXd& u)>;

/// Jacobian J = ∂f/∂x(t, x, u).
using JacFunc = std::function<Eigen::MatrixXd(double t,
    const Eigen::VectorXd& x, const Eigen::VectorXd& u)>;

/**
 * @brief Nonlinear solver strategy for KINSOL equilibrium finding.
 */
enum class KINSOLStrategy {
    Newton,
    LineSearch,
    Picard,
    FixedPoint
};

/**
 * @brief Convergence and scaling settings for KINSOL.
 */
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

/**
 * @brief Robust equilibrium finder with a cascade of solver strategies.
 *
 * Attempts Newton, then LineSearch, relaxed warmup, and Picard + Newton
 * until convergence or all strategies are exhausted.
 *
 * @param rhs ODE right-hand side.
 * @param x0 Initial guess for the state vector.
 * @param u Fixed input vector.
 * @param jac Optional analytical Jacobian.
 * @return Equilibrium state vector.
 */
Eigen::VectorXd findEquilibriumRobust(
    const RHSFunc& rhs,
    const Eigen::VectorXd& x0,
    const Eigen::VectorXd& u,
    const JacFunc& jac = nullptr);

/**
 * @brief Computes ∂f/∂x and ∂f/∂u by central finite differences.
 * @param rhs ODE right-hand side.
 * @param x State at which to evaluate.
 * @param u Input at which to evaluate.
 * @param t Evaluation time (default 0).
 * @param eps Finite-difference step size (default 1e-8).
 * @return Pair (df/dx, df/du).
 */
std::pair<Eigen::MatrixXd, Eigen::MatrixXd> computeJacobians(
    const RHSFunc& rhs,
    const Eigen::VectorXd& x,
    const Eigen::VectorXd& u,
    double t = 0.0,
    double eps = 1e-8);

/**
 * @brief Padé delay approximation for multiple independent signals (order 3).
 * @param tdelay Transport delay (s).
 * @param A Output state matrix.
 * @param B Output input matrix.
 * @param C Output output matrix.
 * @param D Output feed-through matrix.
 * @param num_signals Number of delayed input channels.
 */
void padeDelaySystemMulti3(double tdelay, Eigen::MatrixXd& A, Eigen::MatrixXd& B,
    Eigen::MatrixXd& C, Eigen::MatrixXd& D, int num_signals);

/**
 * @brief Padé delay approximation for multiple independent signals (order 2).
 * @param tdelay Transport delay (s).
 * @param A Output state matrix.
 * @param B Output input matrix.
 * @param C Output output matrix.
 * @param D Output feed-through matrix.
 * @param num_signals Number of delayed input channels.
 */
void padeDelaySystemMulti2(double tdelay, Eigen::MatrixXd& A, Eigen::MatrixXd& B,
    Eigen::MatrixXd& C, Eigen::MatrixXd& D, int num_signals);

/**
 * @brief Discretizes a continuous-time state-space model using the Tustin (bilinear) method.
 * @param A Continuous-time state matrix.
 * @param B Continuous-time input matrix.
 * @param C Continuous-time output matrix.
 * @param D Continuous-time feed-through matrix.
 * @param Ts Sample period (s).
 * @param Ad Output discrete-time state matrix.
 * @param Bd Output discrete-time input matrix.
 * @param Cd Output discrete-time output matrix.
 * @param Dd Output discrete-time feed-through matrix.
 */
void discretizeABCD(
    const Eigen::MatrixXd& A, const Eigen::MatrixXd& B,
    const Eigen::MatrixXd& C, const Eigen::MatrixXd& D,
    double Ts,
    Eigen::MatrixXd& Ad, Eigen::MatrixXd& Bd,
    Eigen::MatrixXd& Cd, Eigen::MatrixXd& Dd);


#endif
