#ifndef CREATE_DC_H
#define CREATE_DC_H

#include <unordered_map>
#include <Eigen/Dense>
#include <string>

std::unordered_map<std::string, Eigen::MatrixXd> create_dc(const std::string& case_name);

#endif