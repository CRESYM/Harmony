#ifndef _WRITER_H_
#define _WRITER_H_

#include "../../Constants.h"

extern void write_file(const std::vector<double>& time, const std::vector<Eigen::MatrixXd>& values, const std::vector<std::string>& headers, const std::string& filename);


#endif // _WRITER_H_