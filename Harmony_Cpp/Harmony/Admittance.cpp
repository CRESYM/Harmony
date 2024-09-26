#include "Admittance.h"
#include <stdexcept>
#include <iostream> // For printing matrix information

using namespace std;
using namespace SymEngine;

// Constructor for vector-based values
Admittance::Admittance(const std::string& symbol, int pins, const std::vector<RCP<Symbol>>& values)
    : Element(symbol, pins, pins), Y_matrix(pins, pins) { // Initialize DenseMatrix
    if (values.size() != 0) {
        if (pins > 0) {
            if (values.size() == 1) { // Single value case
                for (int i = 0; i < pins; i++) {
                    Y_matrix.set(i, i, values[0]); // Set diagonal
                }
            }
            else if (values.size() == pins) { // Diagonal values specified
                for (int i = 0; i < pins; i++) {
                    Y_matrix.set(i, i, values[i]);
                }
            }
            else if (values.size() == pins * pins) { // Full matrix specified
                for (int i = 0; i < pins; i++) {
                    for (int j = 0; j < pins; j++) {
                        Y_matrix.set(i, j, values[i * pins + j]);
                    }
                }
            }
            else {
                throw invalid_argument("Invalid number of admittance vector entries: " + to_string(values.size()));
            }
        }
        else {
            throw invalid_argument("Invalid number of pins, must be greater than 0!");
        }
    }
}

// Single-phase constructor
Admittance::Admittance(const std::string& symbol, int pins, const RCP<const Basic>& admittanceValue)
    : Element(symbol, pins, pins), Y_matrix(pins, pins) {
    for (int i = 0; i < pins; i++) {
        Y_matrix.set(i, i, admittanceValue); // Set diagonal elements
    }
}

// Three-phase constructor
Admittance::Admittance(const std::string& symbol, int pins, const DenseMatrix& admittanceMatrix)
    : Element(symbol, pins, pins), Y_matrix(admittanceMatrix) {}

// Destructor
Admittance::~Admittance() {
    // No need for manual memory management
}

// Method to compute Y-parameters
void Admittance::compute_y_parameters(double frequency) {
    // Output the symbolic representation of the Y-parameters
    cout << "Y-parameters for Admittance (symbolic representation):" << endl;
    for (size_t i = 0; i < Y_matrix.nrows(); ++i) {
        for (size_t j = 0; j < Y_matrix.ncols(); ++j) {
            cout << Y_matrix.get(i, j)->__str__() << " ";
        }
        cout << endl;
    }
}

Admittance::~Admittance() {
    // No need to manually free memory for std::vector or DenseMatrix
    std::cout << "Admittance object for destroyed." << std::endl;
}