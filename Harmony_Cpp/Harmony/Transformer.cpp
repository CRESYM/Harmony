#include "Transformer.h"

// Constructor
Transformer::Transformer(const std::string& symbol, int pins, const std::vector<double>& values)
    : Element(symbol, pins, pins) {

    if (values.size() == 5) {
        R = { values[0], values[2] };  // Primary and secondary resistances
        L = { values[1], values[3] };  // Primary and secondary reactances
        a = values[4];  // Turns ratio
    }
    else {
        throw std::invalid_argument("Invalid number of values, must be 5 (R_primary, X_primary, R_secondary, X_secondary, a)!");
    }

    // Check if the values are properly initialized
    for (int i = 0; i < 2; ++i) {
        if (R[i] == 0 || L[i] == 0 || a == 0) {
            std::cerr << "Transformer parameters not initialized correctly for winding " << i + 1 << "!" << std::endl;
            return;
        }
    }

    // Y parameters matrix
    // Define symbolic resistances and reactances for primary and secondary windings
    // and calculate primary and secondary side impedances
    RCP<const Basic> Z_p = add(real_double(R[0]), mul(j,mul(omega, real_double(L[0]))));
    RCP<const Basic> Z_s = add(real_double(R[1]), mul(j, mul(omega, real_double(L[1]))));
    RCP<const Basic> a_val = real_double(a); // Turns ratio symbol

    RCP<const Basic> Y_11 = div(integer(1), add(Z_p, mul(a_val, mul(a_val, Z_s))));
    RCP<const Basic> Y_12 = mul(a_val, Y_11);
    RCP<const Basic> Y_21 = neg(Y_12);
    RCP<const Basic> Y_22 = mul(a_val, mul(a_val, Y_11));

    // Compute Y-parameters
    for (int i = 0; i < pins; i++) {
        Y_matrix.set(i, i, Y_11);  // Y11
        Y_matrix.set(i, pins + i, Y_12);  // Y12
        Y_matrix.set(pins + i, i, Y_21);  // Y21 (symmetrical to Y12)
        Y_matrix.set(pins + i, pins + i, Y_22);  // Y22
    }

}

// Destructor
Transformer::~Transformer() {
    std::cout << "Transformer object for " << getElementSymbol() << " destroyed." << std::endl;
}


 