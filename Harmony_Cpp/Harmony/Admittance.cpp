#include "Admittance.h"
#include <stdexcept>
#include <iostream> // For printing matrix information

using namespace std;
using namespace SymEngine;

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

