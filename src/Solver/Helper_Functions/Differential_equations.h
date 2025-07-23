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

void padeDelaySystem3(double tdelay, MatrixXd& Adelay, MatrixXd& Bdelay, MatrixXd& Cdelay, MatrixXd& Ddelay);
void padeDelaySystemMulti3(double tdelay, MatrixXd& Adelay, MatrixXd& Bdelay, MatrixXd& Cdelay, MatrixXd& Ddelay, int num_signals);

void padeDelaySystem2(double tdelay, MatrixXd& Adelay, MatrixXd& Bdelay, MatrixXd& Cdelay, MatrixXd& Ddelay);
void padeDelaySystemMulti2(double tdelay, MatrixXd& Adelay, MatrixXd& Bdelay, MatrixXd& Cdelay, MatrixXd& Ddelay, int num_signals);


#endif