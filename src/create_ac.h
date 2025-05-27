#ifndef CREATE_AC_H
#define CREATE_AC_H

#include <unordered_map>
#include <Eigen/Dense>
#include <string>

std::unordered_map<std::string, Eigen::MatrixXd> create_ac(const std::string& case_name);

#endif // CREATE_AC_H

