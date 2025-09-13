#include "Impedance.h"

/*
 * Impedance Constructor
 *
 * Initializes the impedance element by assigning the admittance matrix (Y_matrix) based on
 * the number of pins and impedance values provided. Depending on whether one value or multiple
 * values are provided, the constructor creates either identical or phase-specific impedance entries.
 */
Impedance::Impedance(const std::string& symbol, const std::string& location, int pins, DenseMatrix values):
    Impedance_base(symbol, location, pins, pins) {
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

Impedance::Impedance(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values)
	:Impedance_base(symbol, location, pins, pins) {
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

Impedance::Impedance(const std::string& symbol, const std::string& location, int pins, const double values)
    :Impedance_base(symbol, location, pins, pins) {
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

Impedance::Impedance(const std::string& symbol, const std::string& location, int pins, const std::vector<complex<double>>& values)
    :Impedance_base(symbol, location, pins, pins) {
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

Impedance::Impedance(const std::string& symbol, const std::string& location, int pins, const complex<double> values)
    :Impedance_base(symbol, location, pins, pins) {
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

