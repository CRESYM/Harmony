#include "Generator.h"

// Constructor
Generator::Generator(const std::string& symbol, int pins, const std::vector<double>& values)
    : Source_base(symbol, pins) {

    if (values.size() == 4) {
        R_f = values[0];
        L_f = values[1];
        X_d = values[2];
        T_f = values[3];
    }
    else {
        throw std::invalid_argument("Invalid number of values for generator, must be 4!");
    }

    RCP<const Basic> s = mul(j, omega);

    // Conversion to SymEngine real double data type
    RCP<const Basic> R_f_val = real_double(R_f);
    RCP<const Basic> L_f_val = real_double(L_f);
    RCP<const Basic> X_d_val = real_double(X_d);
    RCP<const Basic> T_f_val = real_double(T_f);

    RCP<const Basic> Z_f = add(R_f_val, mul(s, L_f_val));
    RCP<const Basic> Y_f = div(real_double(1), Z_f);
    RCP<const Basic> Z_d = mul(I, X_d_val);
    RCP<const Basic> H_f = div(real_double(1), add(mul(T_f_val, s), real_double(1)));

    // Y parameters
    RCP<const Basic> Y11 = Y_f;
    RCP<const Basic> Y12 = neg(div(H_f, Z_d));
    RCP<const Basic> Y21 = real_double(0);
    RCP<const Basic> Y22 = neg(div(real_double(1), Z_d));
    for (int i = 0; i < pins; i++) {
        Y_matrix.set(i, i, Y11);  // Y11
        Y_matrix.set(i, pins + i, Y12);  // Y12
        Y_matrix.set(pins + i, i, Y21);  // Y21 (symmetrical to Y12)
        Y_matrix.set(pins + i, pins + i, Y22);  // Y22
    }
}

