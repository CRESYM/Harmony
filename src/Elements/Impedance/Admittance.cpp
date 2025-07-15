#include "Admittance.h"

/*
 * Admittance Constructor
 *
 * Initializes the admittance element by filling the Y_matrix based on the number of pins and
 * the provided admittance values. The constructor supports three configurations:
 * - Case 1: Single admittance value for all phases
 * - Case 2: Vector of admittance values for phase-specific entries
 * - Case 3: Full admittance matrix for multi-phase systems
 */

Admittance::Admittance(const std::string& symbol, int pins, DenseMatrix values) :
Element(symbol, pins, pins) {
    // Check if admittance values are provided
    if (values.ncols() != 0)  // if there are entries
    {
        if (pins > 0) { // Check for valid number of pins
            // Case 1: Single admittance value, applies to all phases
            if (values.ncols() == 1) {
                for (int i = 0; i < pins; i++)
                    Y_matrix.set(i, i, values.get(0, 0));
            }
            // Case 2: Vector of admittance values for each phase (diagonal matrix)
            else if (values.ncols() == pins) {
                for (int i = 0; i < pins; i++)
                    Y_matrix.set(i, i, values.get(0, i));
            }
            // Case 3: Full matrix of admittance values
            else if (values.ncols() == pins * pins) {
                for (int i = 0; i < pins; i++)
                    for (int j = 0; j < pins; j++)
                        Y_matrix.set(i, j, values.get(0, i + j));
            }
            // Error: The number of values doesn't match any expected configuration
            else
                throw invalid_argument("Invalid number of admittance vector entries: " + values.ncols());
        }
    }
    else
        throw invalid_argument("Invalid number of pins, must be greater than 0!");

    // Fill in the complete Y parameters
    for (int i = 0; i < pins; i++)
        for (int j = 0; j < pins; j++) {
            Y_matrix.set(pins+ i, j, sub(zero, Y_matrix.get(i, j)));
            Y_matrix.set(pins + i, pins+ j, Y_matrix.get(i, j));
            Y_matrix.set(i, pins + j, sub(zero, Y_matrix.get(i, j)));
        }
}

// Destructor
Admittance::~Admittance() {
    // No need for manual memory management for DenseMatrix or other standard library components
}

// Power flow computation for AC networks
void Admittance::computePowerFlowAC(std::map<std::string, std::map<std::string, double>>& branchData,
    std::map<std::string, double>& globalParams) const {
    int key = branchData.size();  // Unique branch identifier
    branchData[std::to_string(key)]["transformer"] = 0;
    branchData[std::to_string(key)]["tap"] = 1.0;
    branchData[std::to_string(key)]["shift"] = 0.0;
    branchData[std::to_string(key)]["c_rating_a"] = 1.0;

    // Convert SymEngine expression to double
    double Y_00_real = SymEngine::eval_double(*Y_matrix.get(0, 0));
    std::complex<double> Y_00(Y_00_real, 0.0);

    if (Y_00 == std::complex<double>(0, 0)) {
        throw std::runtime_error("Y_matrix(0,0) is zero, division by zero error.");
    }

    std::complex<double> Z_eq = std::complex<double>(1.0) / Y_00 / globalParams["Z_base"];

    branchData[std::to_string(key)]["br_r"] = std::real(Z_eq);
    branchData[std::to_string(key)]["br_x"] = std::imag(Z_eq);
    branchData[std::to_string(key)]["g_fr"] = std::real(Y_00);
    branchData[std::to_string(key)]["b_fr"] = std::imag(Y_00);
    branchData[std::to_string(key)]["g_to"] = std::real(Y_00);
    branchData[std::to_string(key)]["b_to"] = std::imag(Y_00);
}

// Power flow computation for DC networks
void Admittance::computePowerFlowDC(std::map<std::string, std::map<std::string, double>>& branchDCData,
    std::map<std::string, double>& globalParams) const {
    int key = branchDCData.size();  // Unique DC branch identifier
    branchDCData[std::to_string(key)]["l"] = 0.0;
    branchDCData[std::to_string(key)]["c"] = 0.0;

    // Convert SymEngine expression to double
    double Y_00_real = SymEngine::eval_double(*Y_matrix.get(0, 0));
    std::complex<double> Y_00(Y_00_real, 0.0);

    if (Y_00 == std::complex<double>(0, 0)) {
        throw std::runtime_error("Y_matrix(0,0) is zero, division by zero error.");
    }

    std::complex<double> Z_eq = std::complex<double>(1.0) / Y_00 / globalParams["Z_base"];

    branchDCData[std::to_string(key)]["r"] = std::real(Z_eq);
}
