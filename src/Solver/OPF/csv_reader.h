#ifndef CSV_READER_H
#define CSV_READER_H

#include <Eigen/Dense>
#include <string>

Eigen::MatrixXd readCSVtoCpp(const std::string& filename);

#endif // CSV_READER_H

