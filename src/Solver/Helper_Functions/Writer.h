#ifndef _WRITER_H_
#define _WRITER_H_

/**
 * @file Writer.h
 * @brief CSV file output for time-series simulation data.
 */

#include "../../Constants.h"

/**
 * @brief Writes time-series matrix data to a CSV file.
 * @param time Time axis values (one per row).
 * @param values One matrix per column group; row count must match @p time.
 * @param headers Column header strings (one per column in each matrix block).
 * @param filename Output file path.
 */
extern void write_file(const std::vector<double>& time, const std::vector<Eigen::MatrixXd>& values, const std::vector<std::string>& headers, const std::string& filename);


#endif // _WRITER_H_
