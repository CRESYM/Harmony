/**
 * @file Transformer_real.cpp
 * @brief Implementation of Real transformer model with turns ratio and phase shift.
 */
#include "Transformer_real.h"

// Constructor
Transformer_real::Transformer_real(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values)
    : Transformer_base(symbol, location, pins, values) {
    RCP<const Basic> Y_m = zero; // magnetization admittance

    if (values.size() == 6) {
        R = { values[0], values[2] };  // Primary and secondary resistances
        L = { values[1], values[3] };  // Primary and secondary reactances
        a = values[4];  // Turns ratio
        phi = values[5]; // Phase shift
    }
    else if (values.size() == 8) {
        R = { values[0], values[2], values[4] };  // Primary, secondary and magnetization resistances
        L = { values[1], values[3], values[5] };  // Primary, secondary and magnetization reactances
        a = values[6];  // Turns ratio
        phi = values[7]; // Phase shift
        Y_m = add(div(integer(1), real_double(R[2])), div(integer(1), mul(j, mul(omega, real_double(L[2])))));
    }
    else {
        throw std::invalid_argument("Invalid number of values, must be 6 (R_P, L_P, R_S, L_S, a, phi) or 8 (R_P, L_P, R_S, L_S, R_m, L_m, a, phi)!");
    }

    // Check if the values are properly initialized
    for (int i = 0; i < 2; ++i) {
        if ((R[i] == 0 && L[i] == 0) || a == 0) {
            std::cerr << "Transformer parameters not initialized correctly for winding " << i + 1 << "!" << std::endl;
            return;
        }
    }

    // Y parameters matrix
    // Define symbolic resistances and reactances for primary and secondary windings
    // and calculate primary and secondary side impedances
    RCP<const Basic> Z_p = add(real_double(R[0]), mul(j,mul(omega, real_double(L[0]))));
    RCP<const Basic> Z_s = add(real_double(R[1]), mul(j, mul(omega, real_double(L[1]))));
    RCP<const Basic> phaseFactor = exp(mul(neg(j), real_double(phi)));  // Phase shift factor
    RCP<const Basic> a_val = mul(real_double(a), phaseFactor); // Turns ratio symbol

    RCP<const Basic> Y_11 = div(integer(1), add(Z_p, div(integer(1), add(Y_m, div(integer(1), mul(a_val, mul(a_val, Z_s)))))));
    RCP<const Basic> Y_12 = div(neg(a_val), add(Z_p, mul(mul(a_val, mul(a_val, Z_s)), add(mul(Y_m, Z_p), integer(1)))));
    RCP<const Basic> Y_21 = Y_12;
    RCP<const Basic> Y_22 = div(integer(1), add(Z_s, div(integer(1), mul(a_val, mul(a_val, add(Y_m, div(integer(1), Z_p)))))));

    // Compute Y-parameters
    for (int i = 0; i < pins; i++) {
        Y_matrix.set(i, i, Y_11);  // Y11
        Y_matrix.set(i, pins + i, Y_12);  // Y12
        Y_matrix.set(pins + i, i, Y_21);  // Y21 (symmetrical to Y12)
        Y_matrix.set(pins + i, pins + i, Y_22);  // Y22
    }

}

// Destructor
Transformer_real::~Transformer_real() {
    std::cout << "Transformer object for " << getElementSymbol() << " destroyed." << std::endl;
}


 