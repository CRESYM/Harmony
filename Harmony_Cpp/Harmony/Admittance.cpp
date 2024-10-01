#include "Admittance.h"


Admittance::Admittance(const std::string& symbol, int pins, DenseMatrix values) :
Element(symbol, pins, pins) {
    if (values.ncols() != 0)  // if there are entries
    {
        if (pins > 0) { // Check for valid number of pins
            if (values.ncols() == 1) {
                for (int i = 0; i < pins; i++)
                    Y_matrix.set(i, i, values.get(0, 0));
            }
            else if (values.ncols() == pins) {
                for (int i = 0; i < pins; i++)
                    Y_matrix.set(i, i, values.get(0, i));
            }
            else if (values.ncols() == pins * pins) {
                for (int i = 0; i < pins; i++)
                    for (int j = 0; j < pins; j++)
                        Y_matrix.set(i, j, values.get(0, i + j));
            }
            else
                throw invalid_argument("Invalid number of admittance vector entries: " + values.ncols());
        }
    }
    else
        throw invalid_argument("Invalid number of pins, must be greater than 0!");
}

// Destructor
Admittance::~Admittance() {
    // No need for manual memory management for DenseMatrix or other standard library components
}

