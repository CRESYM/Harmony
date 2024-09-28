#include "Admittance.h"
#include <stdexcept>
#include <iostream> // For printing matrix information

using namespace std;
using namespace SymEngine;

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
    std::cout << "Admittance object for " << getElementSymbol() << " destroyed." << std::endl;
}

// Method to compute Y-parameters (symbolic representation)
void Admittance::compute_y_parameters(double frequency) {
    std::cout << "Y-parameters for Admittance element (" << getElementSymbol() << "):" << std::endl;

    // Print the Y-matrix for the Admittance element
    for (size_t i = 0; i < Y_matrix.nrows(); ++i) {
        for (size_t j = 0; j < Y_matrix.ncols(); ++j) {
            std::cout << Y_matrix.get(i, j)->__str__() << " ";
        }
        std::cout << std::endl;
    }
}

