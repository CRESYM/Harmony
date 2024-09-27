#include "Admittance.h"
#include <stdexcept>
#include <iostream> // For printing matrix information

using namespace std;
using namespace SymEngine;

// Unified constructor for both single-phase and three-phase systems
Admittance::Admittance(const std::string& symbol, int pins, const DenseMatrix& admittanceMatrix)
    : Element(symbol, pins, pins), Y_matrix(admittanceMatrix) {
    // Determine if the admittance matrix is three-phase
    is_three_phase = (admittanceMatrix.nrows() == 3 && admittanceMatrix.ncols() == 3);

    // Print information based on the type of Admittance created
    if (is_three_phase) {
        std::cout << "Three-phase Admittance created for element: " << symbol << std::endl;
    }
    else {
        std::cout << "Single-phase Admittance created for element: " << symbol << std::endl;
    }
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

// Static helper function to create a zero matrix
DenseMatrix Admittance::createZeroMatrix(int size) {
    DenseMatrix zeroMatrix(size, size);
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            zeroMatrix.set(i, j, zero); // Use SymEngine's symbolic `zero`
        }
    }
    return zeroMatrix;
}

