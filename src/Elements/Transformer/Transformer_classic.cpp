/**
 * @file Transformer_classic.cpp
 * @brief Implementation of Classic two-winding transformer with mutual inductance.
 */
#include "Transformer_classic.h"

Transformer_classic::Transformer_classic(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values)
	: Transformer_base(symbol, location, pins, values) {
    if (values.size() == 5) {
        R = { values[0], values[2] };  // Primary and secondary resistances
        L = { values[1], values[3] };  // Primary and secondary inductances
        M = values[4];  // Mutual inductance
    }
    else {
        throw std::invalid_argument("Invalid number of values, must be 5 (R_p, L_p, R_s, L_s, M)!");
    }

    // Define basic components
    RCP<const Basic> Z_p = add(real_double(R[0]), mul(j, mul(omega, real_double(L[0]))));
    RCP<const Basic> Z_s = add(real_double(R[1]), mul(j, mul(omega, real_double(L[1]))));
    RCP<const Basic> Z_m = mul(j, mul(omega, real_double(M)));

    // Denominator
    RCP<const Basic> denominator = sub(mul(Z_p, Z_s), mul(Z_m, Z_m));

    RCP<const Basic> Y_11 = div(Z_s, denominator);
    RCP<const Basic> Y_12 = div(neg(Z_m), denominator);
    RCP<const Basic> Y_21 = Y_12;
    RCP<const Basic> Y_22 = div(Z_p, denominator);

    // Compute Y-parameters
    for (int i = 0; i < pins; i++) {
        Y_matrix.set(i, i, Y_11);  // Y11
        Y_matrix.set(i, pins + i, Y_12);  // Y12
        Y_matrix.set(pins + i, i, Y_21);  // Y21 (symmetrical to Y12)
        Y_matrix.set(pins + i, pins + i, Y_22);  // Y22
    }
}

Transformer_classic::~Transformer_classic() = default;