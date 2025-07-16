#ifndef SOLVE_HMO_OPF_H
#define SOLVE_HMO_OPF_H

#include <unordered_map>
#include <string>
#include <Eigen/Dense>   

void solveHmo_opf(
    std::unordered_map<std::string, Eigen::MatrixXd>& dataOPF,
    bool vscControl = true,
    bool writeTxt = false,
    bool plotResult = true);

#endif  /* SOLVE_HMO_OPF_H */
