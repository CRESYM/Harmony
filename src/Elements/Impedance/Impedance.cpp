#include "Impedance.h"

/*
 * Impedance Constructor
 *
 * Initializes the impedance element by assigning the admittance matrix (Y_matrix) based on
 * the number of pins and impedance values provided. Depending on whether one value or multiple
 * values are provided, the constructor creates either identical or phase-specific impedance entries.
 */
Impedance::Impedance(const std::string& symbol, int pins, DenseMatrix values): 
    Element(symbol, pins, pins) {
    // Check if impedance values are provided
    if (values.ncols() != 0)  // if there are entries
    {
        if (pins > 0) { // Check for valid number of pins
            // Case 1: Single impedance value provided for all phases
            if (values.ncols() == 1) {
                for (int i = 0; i < pins; i++) {
                    Y_matrix.set(i, i, div(integer(1), values.get(0, 0)));
                }
               
            }
            // Case 2: Multiple values provided for each phase's impedance
            else if (values.ncols() == pins) {
                for (int i = 0; i < pins; i++) {
                    Y_matrix.set(i, i, div(integer(1), values.get(0, i)));
                }
            }
            // Error: The number of impedance values provided doesn't match the number of phases
            else
                throw invalid_argument("Invalid number of admittance vector entries: " + values.ncols());
        }
    }
    else
        throw invalid_argument("Invalid number of pins, must be greater than 0!");

    // Fill in the complete Y parameters
    for (int i = 0; i < pins; i++) {
        for (int j = 0; j < pins; j++) {
            Y_matrix.set(pins + i, j, sub(zero, Y_matrix.get(i, j)));
            Y_matrix.set(pins + i, pins + j, Y_matrix.get(i, j));
            Y_matrix.set(i, pins + j, sub(zero, Y_matrix.get(i, j)));
        }
    }
}

Impedance::Impedance(const std::string& symbol, int pins, const std::vector<double>& values) 
	:Element(symbol, pins, pins) {
    // Check if impedance values are provided
    if (values.size() != 0)  // if there are entries
    {
        if (pins > 0) { // Check for valid number of pins
            // Multiple values provided for each phase's impedance
            if (values.size() == pins) {
                for (int i = 0; i < pins; i++) {
                    Y_matrix.set(i, i, div(integer(1), real_double(values[i])));
                }
            }
            // Error: The number of impedance values provided doesn't match the number of phases
            else
                throw invalid_argument("Invalid number of admittance vector entries: " + values.size());
        }
    }
    else
        throw invalid_argument("Invalid number of pins, must be greater than 0!");

    // Fill in the complete Y parameters
    for (int i = 0; i < pins; i++) {
        for (int j = 0; j < pins; j++) {
            Y_matrix.set(pins + i, j, sub(zero, Y_matrix.get(i, j)));
            Y_matrix.set(pins + i, pins + j, Y_matrix.get(i, j));
            Y_matrix.set(i, pins + j, sub(zero, Y_matrix.get(i, j)));
        }
    }

}

Impedance::Impedance(const std::string& symbol, int pins, const double values)
    :Element(symbol, pins, pins) {
    // Check if impedance values are provided
    if (pins > 0) { // Check for valid number of pins
        // Multiple values provided for each phase's impedance
        for (int i = 0; i < pins; i++) {
            Y_matrix.set(i, i, div(integer(1), real_double(values)));
        }

    }
    else
        throw invalid_argument("Invalid number of pins, must be greater than 0!");

    // Fill in the complete Y parameters
    for (int i = 0; i < pins; i++) {
        for (int j = 0; j < pins; j++) {
            Y_matrix.set(pins + i, j, sub(zero, Y_matrix.get(i, j)));
            Y_matrix.set(pins + i, pins + j, Y_matrix.get(i, j));
            Y_matrix.set(i, pins + j, sub(zero, Y_matrix.get(i, j)));
        }
    }

}

Impedance::Impedance(const std::string& symbol, int pins, const std::vector<complex<double>>& values)
    :Element(symbol, pins, pins) {
    // Check if impedance values are provided
    if (values.size() != 0)  // if there are entries
    {
        if (pins > 0) { // Check for valid number of pins
            // Multiple values provided for each phase's impedance
            if (values.size() == pins) {
                for (int i = 0; i < pins; i++) {
					double real_part = real(values[i]);
					double imag_part = imag(values[i]);
                    Y_matrix.set(i, i, div(integer(1), addSym(real_double(real_part), 
                        mul(j, real_double(imag_part)))));
                }
            }
            // Error: The number of impedance values provided doesn't match the number of phases
            else
                throw invalid_argument("Invalid number of admittance vector entries: " + values.size());
        }
    }
    else
        throw invalid_argument("Invalid number of pins, must be greater than 0!");

    // Fill in the complete Y parameters
    for (int i = 0; i < pins; i++) {
        for (int j = 0; j < pins; j++) {
            Y_matrix.set(pins + i, j, sub(zero, Y_matrix.get(i, j)));
            Y_matrix.set(pins + i, pins + j, Y_matrix.get(i, j));
            Y_matrix.set(i, pins + j, sub(zero, Y_matrix.get(i, j)));
        }
    }

}

Impedance::Impedance(const std::string& symbol, int pins, const complex<double> values)
    :Element(symbol, pins, pins) {
    // Check if impedance values are provided
    if (pins > 0) { // Check for valid number of pins
        // Multiple values provided for each phase's impedance
        for (int i = 0; i < pins; i++) {
			double real_part = real(values);
			double imag_part = imag(values);
            Y_matrix.set(i, i, div(integer(1), addSym(real_double(real_part),
                mul(j, real_double(imag_part)))));
        }

    }
    else
        throw invalid_argument("Invalid number of pins, must be greater than 0!");

    // Fill in the complete Y parameters
    for (int i = 0; i < pins; i++) {
        for (int j = 0; j < pins; j++) {
            Y_matrix.set(pins + i, j, sub(zero, Y_matrix.get(i, j)));
            Y_matrix.set(pins + i, pins + j, Y_matrix.get(i, j));
            Y_matrix.set(i, pins + j, sub(zero, Y_matrix.get(i, j)));
        }
    }

}

// Destructor
Impedance::~Impedance() {
    // Clean-up if needed (Smart pointers handle most memory management)
}

// Power flow computation for AC networks
void Impedance::computePowerFlowAC(std::map<std::string, std::map<std::string, double>>& branchData,
    std::map<std::string, double>& globalParams) const {
    int key = branchData.size();  // Unique branch identifier
    branchData[std::to_string(key)]["transformer"] = 0;
    branchData[std::to_string(key)]["tap"] = 1.0;
    branchData[std::to_string(key)]["shift"] = 0.0;
    branchData[std::to_string(key)]["c_rating_a"] = 1.0;

    // Compute Y parameters at operational frequency
    std::complex<double> s = globalParams["omega"] * std::complex<double>(0, 1);

    // Convert SymEngine expression to double
    double Y_00_real = SymEngine::eval_double(*Y_matrix.get(0, 0));
    std::complex<double> Y_00(Y_00_real, 0.0);

    if (Y_00 == std::complex<double>(0, 0)) {
        throw std::runtime_error("Y_matrix(0,0) is zero, division by zero error.");
    }

    std::complex<double> Z_eq = std::complex<double>(1.0) / Y_00 / globalParams["Z_base"];

    branchData[std::to_string(key)]["br_r"] = std::real(Z_eq);
    branchData[std::to_string(key)]["br_x"] = std::imag(Z_eq);
    branchData[std::to_string(key)]["g_fr"] = 0;
    branchData[std::to_string(key)]["b_fr"] = 0;
    branchData[std::to_string(key)]["g_to"] = 0;
    branchData[std::to_string(key)]["b_to"] = 0;
}

// Power flow computation for DC networks
void Impedance::computePowerFlowDC(std::map<std::string, std::map<std::string, double>>& branchDCData,
    std::map<std::string, double>& globalParams) const {
    int key = branchDCData.size();  // Unique DC branch identifier
    branchDCData[std::to_string(key)]["l"] = 0.0;
    branchDCData[std::to_string(key)]["c"] = 0.0;

    // Compute Y parameters at low frequency (DC)
    std::complex<double> s = std::complex<double>(0, 1e-6);

    // Convert SymEngine expression to double
    double Y_00_real = SymEngine::eval_double(*Y_matrix.get(0, 0));
    std::complex<double> Y_00(Y_00_real, 0.0);

    if (Y_00 == std::complex<double>(0, 0)) {
        throw std::runtime_error("Y_matrix(0,0) is zero, division by zero error.");
    }

    std::complex<double> Z_eq = std::complex<double>(1.0) / Y_00 / globalParams["Z_base"];

    branchDCData[std::to_string(key)]["r"] = std::real(Z_eq);
}