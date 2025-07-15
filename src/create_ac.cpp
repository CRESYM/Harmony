#include "create_ac.h"
#include "csv_reader.h"
#include <Eigen/Dense>
#include <unordered_map>
#include <string>
#include <iostream>
#include<fstream>
#include <stdexcept>

/*
 * Function: create_ac
 * -------------------
 * Loads AC grid data from CSV files based on the provided case name prefix.

 * Expected CSV files (located in the same directory):
 *   - <case_name>_baseMVA_ac.csv
 *   - <case_name>_bus_ac.csv
 *   - <case_name>_branch_ac.csv
 *   - <case_name>_gen_ac.csv
 *   - <case_name>_gencost_ac.csv

 * Return unordered_map with the following keys:
 *   "baseMVA"   : Scalar base MVA value.
 *   "bus"       : Matrix containing AC bus data.
 *   "branch"    : Matrix containing AC branch data.
 *   "generator" : Matrix containing AC generator data.
 *   "gencost"   : Matrix containing AC generator cost data.
 */

std::unordered_map<std::string, Eigen::MatrixXd> create_ac(const std::string& case_name) {
    std::unordered_map<std::string, Eigen::MatrixXd> ac;
    // Set data directory prefix
    const std::string data_dir = "data/";
    // List of required CSV file names
    const std::string baseMVA_file = data_dir + case_name + "_baseMVA_ac.csv";
    const std::string bus_file = data_dir + case_name + "_bus_ac.csv";
    const std::string branch_file = data_dir + case_name + "_branch_ac.csv";
    const std::string gen_file = data_dir + case_name + "_gen_ac.csv";
    const std::string gencost_file = data_dir + case_name + "_gencost_ac.csv";

    // Check missing files, if not, then save as the matrix
    try {

        std::ifstream ifs_baseMVA(baseMVA_file);
        if (!ifs_baseMVA.good()) {
            throw std::runtime_error("Missing required file: " + baseMVA_file);
        }
        ac["baseMVA"] = readCSVtoCpp(baseMVA_file);

        std::ifstream ifs_bus(bus_file);
        if (!ifs_bus.good()) {
            throw std::runtime_error("Missing required file: " + bus_file);
        }
        ac["bus"] = readCSVtoCpp(bus_file);

        std::ifstream ifs_branch(branch_file);
        if (!ifs_branch.good()) {
            throw std::runtime_error("Missing required file: " + branch_file);
        }
        ac["branch"] = readCSVtoCpp(branch_file);

        std::ifstream ifs_gen(gen_file);
        if (!ifs_gen.good()) {
            throw std::runtime_error("Missing required file: " + gen_file);
        }
        ac["generator"] = readCSVtoCpp(gen_file);

        std::ifstream ifs_gencost(gencost_file);
        if (!ifs_gencost.good()) {
            throw std::runtime_error("Missing required file: " + gencost_file);
        }
        ac["gencost"] = readCSVtoCpp(gencost_file);
    }
    catch (const std::exception& e) {
        std::cerr << "Error in create_ac: " << e.what() << std::endl;
        throw std::runtime_error("Failed to load AC grid data. Please verify that the required CSV files exist.");
    }

    return ac;
}
