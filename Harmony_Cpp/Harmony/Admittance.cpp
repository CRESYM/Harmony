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

