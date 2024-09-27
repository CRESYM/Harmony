#include "Transformer.h"
#include "Element.h"
#include "Constants.h"
#include <iostream>


using namespace SymEngine;

// Constructor
Transformer::Transformer(const std::string& symbol, int pins, std::vector<double> values) : Element(symbol, pins, pins) {
    if (pins != 2)  // Typically a transformer has 2 pins (primary and secondary)
        throw std::invalid_argument("Invalid number of pins, transformer must have 2 pins!");

    if (values.size() == 5) {
        R = { values[0], values[2] };  // Primary and secondary resistances
        X = { values[1], values[3] };  // Primary and secondary reactances
        a = values[4];  // Turns ratio
    }
    else {
        throw std::invalid_argument("Invalid number of values, must be 5 (R_primary, X_primary, R_secondary, X_secondary, a)!");
    }

    // Check if the values are properly initialized
    for (int i = 0; i < 2; ++i) {
        if (R[i] == 0 || X[i] == 0 || a == 0) {
            std::cerr << "Transformer parameters not initialized correctly for winding " << i + 1 << "!" << std::endl;
            return;
        }
        else {
            std::cerr << "Transformer parameters initialized correctly for winding " << i + 1 << "!" << std::endl;
        }
    }
}

// Function to print the transformer values
void Transformer::printElementValues() {
    printElementInfo();
    std::cout << "Primary Resistance: " << R[0] << " ohms" << std::endl;
    std::cout << "Primary Reactance: " << X[0] << " ohms" << std::endl;
    std::cout << "Secondary Resistance: " << R[1] << " ohms" << std::endl;
    std::cout << "Secondary Reactance: " << X[1] << " ohms" << std::endl;
    std::cout << "Turns Ratio: " << a << std::endl;
}

// Y-parameter computation using SymEngine
void Transformer::compute_y_parameters(double frequency) {
    std::cout << "Computing Y-parameters for a transformer...\n";

    RCP<const Basic> omega = mul(real_double(2), mul(PI, real_double(frequency)));
    RCP<const Basic> j = I;  // Imaginary unit

    // Matrix for Y-parameters (2x2)
    std::vector<std::vector<RCP<const Basic>>> Y_matrix(2, std::vector<RCP<const Basic>>(2));

    // Primary side
    RCP<const Basic> R_p = real_double(R[0]);
    RCP<const Basic> X_p = real_double(X[0]);

    // Secondary side
    RCP<const Basic> R_s = real_double(R[1]);
    RCP<const Basic> X_s = real_double(X[1]);

    RCP<const Basic> a_val = real_double(a);

    // Compute the primary and secondary admittances
    RCP<const Basic> Y_p = div(real_double(1), add(R_p, mul(j, X_p)));
    RCP<const Basic> Y_s = div(real_double(1), add(R_s, mul(j, X_s)));

    // Compute Y-parameters
    Y_matrix[0][0] = Y_p;  // Y11
    Y_matrix[0][1] = mul(real_double(-a), Y_p);  // Y12
    Y_matrix[1][0] = Y_matrix[0][1];  // Y21 (symmetrical to Y12)
    Y_matrix[1][1] = Y_s;  // Y22

    // Print the Y-parameters
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            double Y_val_abs = eval_double(*abs(Y_matrix[i][j]));
            std::cout << "|Transformer Y" << (i + 1) << (j + 1) << "|: " << Y_val_abs << " S" << std::endl;
        }
    }
}

 