#include "Transformer.h"
#include "Element.h"
#include "Constants.h"
#include <iostream>


using namespace SymEngine;

// Constructor
//Transformer::Transformer(const std::string& symbol, int pins, std::vector<double> values) : Element(symbol, pins, pins) {
Transformer::Transformer(const std::string& symbol, int pins, const std::vector<double>& values)
    : Element(symbol, pins, pins) {
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
    // Initialize Y_matrix with 2x2 dimensions
     Y_matrix = DenseMatrix(2, 2);  // Initializes a 2x2 matrix with default zero values
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

    // Compute the primary and secondary admittances
    //RCP<const Basic> Y_p = div(real_double(1), add(R_p, mul(j, X_p)));
    //RCP<const Basic> Y_s = div(real_double(1), add(R_s, mul(j, X_s)));
    // 
    // Compute primary and secondary impedances: Z_p = R_p + j * X_p
    RCP<const Basic> Z_p = add(R_p, mul(j, X_p));
    RCP<const Basic> Z_s = add(R_s, mul(j, X_s));

    // Compute admittances: Y_p = 1 / Z_p, Y_s = 1 / Z_s
    RCP<const Basic> Y_p = div(one, Z_p);
    RCP<const Basic> Y_s = div(one, Z_s);

    // Y11 = Y_primary + a^2 * Y_s
    RCP<const Basic> Y11 = add(Y_p, mul(pow(a_val, real_double(2)), Y_s));

    // Y12 = Y21 = -a * Y_s
    RCP<const Basic> Y12 = neg(mul(a_val, Y_s));
    RCP<const Basic> Y21 = Y12;

    // Y22 = Y_s
    RCP<const Basic> Y22 = Y_s;

    // Compute Y-parameters
   // Y_matrix.set(0, 0, Y_p);  // Y11
    //Y_matrix.set(0, 1, mul(real_double(-a), Y_p));  // Y12
    //Y_matrix.set(1, 0, Y_matrix.get(0, 1));  // Y21 (symmetrical to Y12)
    //Y_matrix.set(1, 1, Y_s);  // Y22

    // Assign the computed Y-parameters to the transformer Y-matrix
    Y_matrix.set(0, 0, Y11);  // Y11
    Y_matrix.set(0, 1, Y12);  // Y12
    Y_matrix.set(1, 0, Y21);  // Y21 (symmetrical to Y12)
    Y_matrix.set(1, 1, Y22);  // Y22
    std::cout << "Y_matrix dimensions2: " << Y_matrix.nrows() << " x " << Y_matrix.ncols() << std::endl;



    // Print the Y-parameters
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            double Y_val_abs = eval_double(*abs(Y_matrix.get(i, j)));
            std::cout << "|Transformer Y" << (i + 1) << (j + 1) << "|: " << Y_val_abs << " S" << std::endl;
        }
    }
}

 