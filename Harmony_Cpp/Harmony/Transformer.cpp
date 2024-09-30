#include "Transformer.h"
#include "Element.h"
#include "Constants.h"
#include <iostream>


using namespace SymEngine;

// Constructor
Transformer::Transformer(const std::string& symbol, int pins, const std::vector<double>& values)
    : Element(symbol, pins, pins) {

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

// Destructor
Transformer::~Transformer() {
    std::cout << "Transformer object for " << getElementSymbol() << " destroyed." << std::endl;
}

// Y-parameter computation using SymEngine
void Transformer::compute_y_parameters(double frequency) {
    std::cout << "Computing Y-parameters for a transformer...\n";
    std::cout << "Computing Y-parameters for Transformer (" << getElementSymbol() << ") at " << frequency << " Hz:" << std::endl;

    // Calculate the imaginary unit 'j' and the angular frequency 'omega = 2 * pi * frequency'
    RCP<const Basic> omega = mul(real_double(2), mul(PI, real_double(frequency)));
    RCP<const Basic> j = I;  // Imaginary unit

    // Define symbolic resistances and reactances for primary and secondary windings
    // Primary side
    RCP<const Basic> R_p = real_double(R[0]);
    RCP<const Basic> X_p = real_double(X[0]);

    // Secondary side
    RCP<const Basic> R_s = real_double(R[1]);
    RCP<const Basic> X_s = real_double(X[1]);
    RCP<const Basic> a_val = real_double(a); // Turns ratio symbol

    // Check if any resistance or reactance is equivalent to zero
    RCP<const Basic> zero = integer(0);
    if (eval_double(*R_p) == 0 || eval_double(*X_p) == 0 || eval_double(*R_s) == 0 || eval_double(*X_s) == 0) {
        std::cerr << "Error: Zero impedance encountered!" << std::endl;
        return; // Early exit to avoid further computation
    }

    // Compute the primary and secondary admittances
    RCP<const Basic> Y_p = div(real_double(1), add(R_p, mul(j, X_p)));
    RCP<const Basic> Y_s = div(real_double(1), add(R_s, mul(j, X_s)));

    // Compute Y-parameters
    Y_matrix.set(0, 0, Y_p);  // Y11
    Y_matrix.set(0, 1, mul(real_double(-a), Y_p));  // Y12
    //Y_matrix.set(0, 1, neg(mul(a_val, Y_s)));  // Y12
    Y_matrix.set(1, 0, Y_matrix.get(0, 1));  // Y21 (symmetrical to Y12)
    Y_matrix.set(1, 1, Y_s);  // Y22

    std::cout << "Y_matrix dimensions2: " << Y_matrix.nrows() << " x " << Y_matrix.ncols() << std::endl;

}

 