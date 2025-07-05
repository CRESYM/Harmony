#ifndef MAKEYBUS_H
#define MAKEYBUS_H

#include <Eigen/Sparse>
#include <Eigen/Dense>

Eigen::SparseMatrix<std::complex<double>> makeYbus(double baseMVA, const Eigen::MatrixXd& bus, const Eigen::MatrixXd& branch);

#endif
