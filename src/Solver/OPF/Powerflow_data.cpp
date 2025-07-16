#include "powerflow.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <vector>


Eigen::MatrixXd PowerFlow::readCSVtoCpp(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::vector<std::vector<double>> data;
    std::string line;

    while (std::getline(file, line)) {
        std::stringstream lineStream(line);
        std::vector<double> row;
        std::string cell;

        while (std::getline(lineStream, cell, ',')) {
            try {
                row.push_back(std::stod(cell));
            }
            catch (const std::invalid_argument&) {
                std::cerr << "Invalid number in file " << filename
                    << ": '" << cell << "'. Setting it to 0.0." << std::endl;
                row.push_back(0.0);
            }
        }

        if (!row.empty()) {
            data.push_back(row);
        }
    }

    if (data.empty()) {
        return Eigen::MatrixXd();
    }

    size_t ncols = data[0].size();
    for (size_t i = 1; i < data.size(); ++i) {
        if (data[i].size() != ncols) {
            throw std::runtime_error("Inconsistent number of columns in file: " + filename);
        }
    }

    Eigen::MatrixXd matrix(data.size(), ncols);
    for (size_t i = 0; i < data.size(); ++i) {
        for (size_t j = 0; j < ncols; ++j) {
            matrix(i, j) = data[i][j];
        }
    }

    return matrix;
}


std::unordered_map<std::string, Eigen::MatrixXd> PowerFlow::create_ac(const std::string& case_name) {
    std::unordered_map<std::string, Eigen::MatrixXd> ac;

    try {
        ac["baseMVA"] = readCSVtoCpp("data/" + case_name + "_baseMVA_ac.csv");
        ac["bus"] = readCSVtoCpp("data/" + case_name + "_bus_ac.csv");
        ac["branch"] = readCSVtoCpp("data/" + case_name + "_branch_ac.csv");
        ac["generator"] = readCSVtoCpp("data/" + case_name + "_gen_ac.csv");
        ac["gencost"] = readCSVtoCpp("data/" + case_name + "_gencost_ac.csv");
    }
    catch (const std::exception& e) {
        std::cerr << "Error in create_ac: " << e.what() << std::endl;
        throw std::runtime_error("Failed to load AC grid data. Ensure all required CSV files exist.");
    }

    return ac;
}


std::unordered_map<std::string, Eigen::MatrixXd> PowerFlow::create_dc(const std::string& case_name) {
    std::unordered_map<std::string, Eigen::MatrixXd> dc;

    try {
        dc["baseMW"] = readCSVtoCpp("data/" + case_name + "_baseMW_dc.csv");
        dc["pol"] = readCSVtoCpp("data/" + case_name + "_pol_dc.csv");
        dc["bus"] = readCSVtoCpp("data/" + case_name + "_bus_dc.csv");
        dc["branch"] = readCSVtoCpp("data/" + case_name + "_branch_dc.csv");
        dc["converter"] = readCSVtoCpp("data/" + case_name + "_conv_dc.csv");
    }
    catch (const std::exception& e) {
        std::cerr << "Error in create_dc: " << e.what() << std::endl;
        throw std::runtime_error("Failed to load DC grid data. Ensure all required CSV files exist.");
    }

    return dc;
}
