#ifndef _DIFFERENTIAL_EQUATIONS_H_
#define	_DIFFERENTIAL_EQUATIONS_H_


#include "../../Constants.h"

//extern MatrixXd equilibrium(const VectorXd& init_x, const VectorXd& init_u, function<MatrixXd (const VectorXd&, const VectorXd&)> f);

using DerivFunc = std::function<Eigen::VectorXd(const Eigen::VectorXd&, const Eigen::VectorXd&)>;
extern Eigen::VectorXd findEquilibrium(const Eigen::VectorXd& x0, const Eigen::VectorXd& u, DerivFunc f,
    double tol = 1e-2, int max_iter = 1e6);

using DerivFunc = std::function<Eigen::VectorXd(const Eigen::VectorXd&, const Eigen::VectorXd&)>;
extern std::pair<Eigen::MatrixXd, Eigen::MatrixXd> computeJacobians(
    const Eigen::VectorXd& x, const Eigen::VectorXd& u, DerivFunc f, double eps = 1e-8);


// Using Eigen's Levenberg–Marquardt
using DerivFunc = std::function<Eigen::VectorXd(const Eigen::VectorXd&, const Eigen::VectorXd&)>;

// --- Functor base expected by Eigen's NumericalDiff / LM --------------------
template <typename _Scalar, int NX = Eigen::Dynamic, int NY = Eigen::Dynamic>
struct Functor {
    using Scalar = _Scalar;
    enum {
        InputsAtCompileTime = NX,
        ValuesAtCompileTime = NY
    };
    using InputType = Eigen::Matrix<Scalar, InputsAtCompileTime, 1>;
    using ValueType = Eigen::Matrix<Scalar, ValuesAtCompileTime, 1>;
    using JacobianType = Eigen::Matrix<Scalar, ValuesAtCompileTime, InputsAtCompileTime>;

    int m_inputs, m_values;
    Functor(int inputs, int values) : m_inputs(inputs), m_values(values) {}
    int inputs() const { return m_inputs; }
    int values() const { return m_values; }
};
// ---------------------------------------------------------------------------

// Wrap the user-supplied derivative function f(x,u) into an Eigen functor
struct SystemFunctor : Functor<double> {
    DerivFunc f;
    Eigen::VectorXd u;

    SystemFunctor(DerivFunc f_, const Eigen::VectorXd& u_, int n_inputs)
        : Functor<double>(n_inputs, n_inputs), f(std::move(f_)), u(u_) {}

    // required call operator: compute residuals fvec = f(x, u)
    int operator()(const Eigen::VectorXd& x, Eigen::VectorXd& fvec) const {
        fvec = f(x, u);
        return 0;
    }
};

// Wrapper around Eigen's Levenberg–Marquardt
Eigen::VectorXd findEquilibriumLM(
    const Eigen::VectorXd& x0,
    const Eigen::VectorXd& u,
    DerivFunc f,
    double tol = 1e-6,
    int max_iter = 1e5);

void padeDelaySystem3(double tdelay, MatrixXd& Adelay, MatrixXd& Bdelay, MatrixXd& Cdelay, MatrixXd& Ddelay);
void padeDelaySystemMulti3(double tdelay, MatrixXd& Adelay, MatrixXd& Bdelay, MatrixXd& Cdelay, MatrixXd& Ddelay, int num_signals);

void padeDelaySystem2(double tdelay, MatrixXd& Adelay, MatrixXd& Bdelay, MatrixXd& Cdelay, MatrixXd& Ddelay);
void padeDelaySystemMulti2(double tdelay, MatrixXd& Adelay, MatrixXd& Bdelay, MatrixXd& Cdelay, MatrixXd& Ddelay, int num_signals);


#endif