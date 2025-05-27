#include "csv_reader.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>

/*
   Function: csv_reader
   -------------------
   Reads a CSV file and returns its contents as an Eigen::MatrixXd.

*/

Eigen::MatrixXd readCSVtoCpp(const std::string& filename) {

    // Open the input file stream
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::vector<std::vector<double>> data;
    std::string line;

    // Read the file line by line
    while (std::getline(file, line)) {
        std::stringstream lineStream(line);
        std::vector<double> row;
        std::string cell;

        while (std::getline(lineStream, cell, ',')) {
            try {
                row.push_back(std::stod(cell));
            }
            catch (const std::invalid_argument& e) {
                std::cerr << "Invalid number in file " << filename
                    << ": '" << cell << "'. Setting it to 0.0." << std::endl;
                row.push_back(0.0);
            }
        }

        // Only add non-empty rows to the data vector
        if (!row.empty()) {
            data.push_back(row);
        }
    }

    // If no data was read, return an empty matrix
    if (data.empty()) {
        return Eigen::MatrixXd();
    }

    // Verify that all rows have the same number of columns
    const size_t ncols = data[0].size();
    for (size_t i = 1; i < data.size(); ++i) {
        if (data[i].size() != ncols) {
            throw std::runtime_error("Inconsistent number of columns in file: " + filename);
        }
    }

    // Construct an Eigen::MatrixXd from the 2D vector data
    Eigen::MatrixXd matrix(data.size(), ncols);
    for (size_t i = 0; i < data.size(); ++i) {
        for (size_t j = 0; j < ncols; ++j) {
            matrix(i, j) = data[i][j];
        }
    }

    return matrix;
}
