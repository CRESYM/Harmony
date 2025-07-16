#ifndef _DIFFERENTIAL_EQUATIONS_H_
#define	_DIFFERENTIAL_EQUATIONS_H_


#include "../../Constants.h"

//extern MatrixXd equilibrium(const VectorXd& init_x, const VectorXd& init_u, function<MatrixXd (const VectorXd&, const VectorXd&)> f);

using DerivFunc = std::function<Eigen::VectorXd(const Eigen::VectorXd&, const Eigen::VectorXd&)>;
extern Eigen::VectorXd findEquilibrium(const Eigen::VectorXd& x0, const Eigen::VectorXd& u, DerivFunc f,
    double tol = 1e-6, int max_iter = 10000);

using DerivFunc = std::function<Eigen::VectorXd(const Eigen::VectorXd&, const Eigen::VectorXd&)>;
extern std::pair<Eigen::MatrixXd, Eigen::MatrixXd> computeJacobians(
    const Eigen::VectorXd& x, const Eigen::VectorXd& u, DerivFunc f, double eps = 1e-8);

extern std::pair<std::pair<Eigen::MatrixXd, Eigen::MatrixXd>, std::pair<Eigen::MatrixXd, Eigen::MatrixXd>> padeDelaySystem3(double tdelay);
extern std::pair<std::pair<Eigen::MatrixXd, Eigen::MatrixXd>, std::pair<Eigen::MatrixXd, Eigen::MatrixXd>> padeDelaySystemMulti3(double tdelay, int num_signals);

extern std::pair<std::pair<Eigen::MatrixXd, Eigen::MatrixXd>, std::pair<Eigen::MatrixXd, Eigen::MatrixXd>> padeDelaySystem2(double tdelay);
extern std::pair<std::pair<Eigen::MatrixXd, Eigen::MatrixXd>, std::pair<Eigen::MatrixXd, Eigen::MatrixXd>> padeDelaySystemMulti2(double tdelay, int num_signals);


#endif