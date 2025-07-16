#include "makeYbus.h"
#include <complex>
#include <Eigen/Dense>
#include <Eigen/Sparse>

Eigen::SparseMatrix<std::complex<double>> makeYbus(double baseMVA, const Eigen::MatrixXd& bus, const Eigen::MatrixXd& branch) {
    int nb = bus.rows();
    int nl = branch.rows();

    Eigen::VectorXd stat = branch.col(10);
    Eigen::VectorXcd Ys = stat.array() / (branch.col(2).array() + std::complex<double>(0, 1) * branch.col(3).array());
    Eigen::VectorXd Bc = stat.array() * branch.col(4).array();

    Eigen::VectorXd tap = Eigen::VectorXd::Ones(nl);
    for (int i = 0; i < nl; ++i) {
        if (branch(i, 8) != 0) {
            tap(i) = branch(i, 8);
        }
    }

    Eigen::VectorXd shifts = branch.col(9).cast<double>();
    Eigen::VectorXcd tap_shifted = tap.array() * (std::complex<double>(0, 3.141592653 / 180.0) * shifts.array()).exp();

    Eigen::VectorXcd Ytt = Ys + std::complex<double>(0, 0.5) * Bc;
    Eigen::VectorXcd Yff = Ytt.array() / (tap_shifted.array() * tap_shifted.array().conjugate());
    Eigen::VectorXcd Yft = -Ys.array() / tap_shifted.array().conjugate();
    Eigen::VectorXcd Ytf = -Ys.array() / tap_shifted.array();

    Eigen::VectorXcd Ysh = (bus.col(4).array() + std::complex<double>(0, 1) * bus.col(5).array()) / baseMVA;

    // Bus indices
    Eigen::VectorXi f = branch.col(0).cast<int>().array() - 1;
    Eigen::VectorXi t = branch.col(1).cast<int>().array() - 1;

    // Build Ybus
    Eigen::SparseMatrix<std::complex<double>> Ybus(nb, nb);
    for (int i = 0; i < nl; ++i) {
        Ybus.coeffRef(f(i), f(i)) += Yff(i);
        Ybus.coeffRef(t(i), t(i)) += Ytt(i);
        Ybus.coeffRef(f(i), t(i)) += Yft(i);
        Ybus.coeffRef(t(i), f(i)) += Ytf(i);
    }
    for (int i = 0; i < nb; ++i) {
        Ybus.coeffRef(i, i) += Ysh(i);
    }

    return Ybus;
}
