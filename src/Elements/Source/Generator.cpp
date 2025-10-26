#include "Generator.h"

// Constructor
Generator::Generator(const std::string& symbol, const std::string& location, int pins, double V, const std::vector<double>& values)
    : Source_base(symbol, location, pins, V) {

    if (values.size() == 3) {
        R_f = values[0];
        T_f = values[1];
        X_d = values[2];
        L_f = T_f * R_f;
		X_m = X_d; // Default value 
    }
    else {
        throw std::invalid_argument("Invalid number of values for generator, must be 3!");
    }

	// Initialize Y_matrix size in abc frame
	Y_matrix = createZeroMatrix(2 * pins, 2 * pins);

    // Conversion to SymEngine real double data type
    RCP<const Basic> R_f_val = real_double(R_f);
	RCP<const Basic> L_f_val = real_double(L_f);
    RCP<const Basic> X_d_val = real_double(X_d);
	RCP<const Basic> X_m_val = real_double(X_m);

    RCP<const Basic> Z_f = add(R_f_val, mul(s, L_f_val));
    RCP<const Basic> Z_d = mul(I, X_d_val);
	RCP<const Basic> Z_m = mul(I, real_double(X_m));
    RCP<const Basic> D = add(mul(I, mul(X_d_val, Z_f)), mul(X_m_val, X_m_val));

    // Y parameters
    RCP<const Basic> Y11 = div(Z_f, D);
    RCP<const Basic> Y12 = div(mul(I, neg(X_m_val)), D);
    RCP<const Basic> Y21 = Y12;
    RCP<const Basic> Y22 = div(mul(I, X_d_val), D);
    for (int i = 0; i < pins; i++) {
        Y_matrix.set(i, i, Y11);  // Y11
        Y_matrix.set(i, pins + i, Y12);  // Y12
        Y_matrix.set(pins + i, i, Y21);  // Y21 (symmetrical to Y12)
        Y_matrix.set(pins + i, pins + i, Y22);  // Y22
    }
}

