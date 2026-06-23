#ifndef _DIFFERENTIAL_EQUATIONS_H_
#define _DIFFERENTIAL_EQUATIONS_H_

/**
 * @file Differential_equations.h
 * @brief Numerical integration, equilibrium finding, and discretization utilities.
 *
 * Wraps CVODE (stiff ODE integration), KINSOL (nonlinear equilibrium solvers),
 * finite-difference Jacobian computation, Padé delay approximations, and
 * Tustin (bilinear) discretization of state-space models.
 */

#include "../../Constants.h"

/// Right-hand side of dx/dt = f(t, x, u).
using RHSFunc = std::function<Eigen::VectorXd(double t,
    const Eigen::VectorXd& x, const Eigen::VectorXd& u)>;

/// Jacobian J = ∂f/∂x(t, x, u).
using JacFunc = std::function<Eigen::MatrixXd(double t,
    const Eigen::VectorXd& x, const Eigen::VectorXd& u)>;

/**
 * @brief Tolerance and step-size settings for CVODE integration.
 */
struct CVODEConfig {
    double rtol = 1e-8;
    double atol = 1e-10;
    double dt_max = 1e-3;
    bool use_analytical_jac = false;
};

/**
 * @brief Time histories returned by CVODE integration.
 */
struct CVODEResult {
    std::vector<double> time;
    std::vector<Eigen::VectorXd> states;
};

/**
 * @brief Integrates an ODE system using CVODE (BDF, stiff).
 * @param rhs Right-hand side f(t, x, u).
 * @param x0 Initial state vector.
 * @param inputFn Time-varying input u(t).
 * @param t0 Start time.
 * @param tEnd End time.
 * @param dt_output Output sampling interval.
 * @param cfg Tolerance and Jacobian settings.
 * @param jac Optional analytical Jacobian (nullptr → finite differences).
 * @return Sampled time and state histories.
 */
CVODEResult integrate(
    const RHSFunc& rhs,
    const Eigen::VectorXd& x0,
    const std::function<Eigen::VectorXd(double t)>& inputFn,
    double t0, double tEnd, double dt_output,
    const CVODEConfig& cfg = {},
    const JacFunc& jac = nullptr);

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
 * @brief Finds a steady-state equilibrium dx/dt = 0 for fixed input u.
 * @param rhs ODE right-hand side.
 * @param x0 Initial guess for the state vector.
 * @param u Fixed input vector.
 * @param cfg Solver strategy and tolerance settings.
 * @param jac Optional analytical Jacobian.
 * @return Equilibrium state vector.
 */
Eigen::VectorXd findEquilibrium(
    const RHSFunc& rhs,
    const Eigen::VectorXd& x0,
    const Eigen::VectorXd& u,
    const KINSOLConfig& cfg = {},
    const JacFunc& jac = nullptr);

/**
 * @brief Robust equilibrium finder with a cascade of solver strategies.
 *
 * Attempts LineSearch, then Newton, relaxed warmup, and Picard + Newton
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
 * @brief First-order Padé delay approximation (single signal, order 3).
 * @param tdelay Transport delay (s).
 * @param A Output state matrix.
 * @param B Output input matrix.
 * @param C Output output matrix.
 * @param D Output feed-through matrix.
 */
void padeDelaySystem3(double tdelay, Eigen::MatrixXd& A, Eigen::MatrixXd& B,
    Eigen::MatrixXd& C, Eigen::MatrixXd& D);

/**
 * @brief First-order Padé delay approximation for multiple independent signals.
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
 * @brief Second-order Padé delay approximation (single signal, order 2).
 * @param tdelay Transport delay (s).
 * @param A Output state matrix.
 * @param B Output input matrix.
 * @param C Output output matrix.
 * @param D Output feed-through matrix.
 */
void padeDelaySystem2(double tdelay, Eigen::MatrixXd& A, Eigen::MatrixXd& B,
    Eigen::MatrixXd& C, Eigen::MatrixXd& D);

/**
 * @brief Second-order Padé delay approximation for multiple independent signals.
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
