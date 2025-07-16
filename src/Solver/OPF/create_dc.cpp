#include "create_dc.h"
#include "csv_reader.h"
#include <string>
#include <iostream>
#include <unordered_map>
#include <Eigen/Dense>
#include <fstream>
#include <stdexcept>

/*
 * Function: create_dc
 * -------------------
 * Loads DC grid data from CSV files based on the provided case name prefix.
 *
 * Expected files (located in the same directory):
 *   - <case_name>_baseMW_dc.csv
 *   - <case_name>_pol_dc.csv
 *   - <case_name>_bus_dc.csv
 *   - <case_name>_branch_dc.csv
 *   - <case_name>_conv_dc.csv

 * Data from these files is stored in an unordered_map with the following keys:
 *   "baseMW"   : Scalar base MW value.
 *   "pol"      : DC grid pole value.
 *   "bus"      : Matrix containing DC bus data.
 *   "branch"   : Matrix containing DC branch data.
 *   "converter": Matrix containing DC converter data.
 */

std::unordered_map<std::string, Eigen::MatrixXd> create_dc(const std::string& case_name) {
    std::unordered_map<std::string, Eigen::MatrixXd> dc;

    // List of required CSV file names
    const std::string baseMW_file = case_name + "_baseMW_dc.csv";
    const std::string pol_file = case_name + "_pol_dc.csv";
    const std::string bus_file = case_name + "_bus_dc.csv";
    const std::string branch_file = case_name + "_branch_dc.csv";
    const std::string conv_file = case_name + "_conv_dc.csv";

    // Check missing files, if not, then save as the matrix
    try {
        std::ifstream ifs_baseMW(baseMW_file);
        if (!ifs_baseMW.good()) {
            throw std::runtime_error("Missing required file: " + baseMW_file);
        }
        dc["baseMW"] = readCSVtoCpp(baseMW_file);

        std::ifstream ifs_pol(pol_file);
        if (!ifs_pol.good()) {
            throw std::runtime_error("Missing required file: " + pol_file);
        }
        dc["pol"] = readCSVtoCpp(pol_file);

        std::ifstream ifs_bus(bus_file);
        if (!ifs_bus.good()) {
            throw std::runtime_error("Missing required file: " + bus_file);
        }
        dc["bus"] = readCSVtoCpp(bus_file);

        std::ifstream ifs_branch(branch_file);
        if (!ifs_branch.good()) {
            throw std::runtime_error("Missing required file: " + branch_file);
        }
        dc["branch"] = readCSVtoCpp(branch_file);

        std::ifstream ifs_conv(conv_file);
        if (!ifs_conv.good()) {
            throw std::runtime_error("Missing required file: " + conv_file);
        }
        dc["converter"] = readCSVtoCpp(conv_file);
    }
    catch (const std::exception& e) {
        // Print the error message and rethrow the exception for further handling.
        std::cerr << "Error in create_dc: " << e.what() << std::endl;
        throw std::runtime_error("Failed to load DC grid data. Please verify that all required CSV files exist.");
    }

    return dc;
}
