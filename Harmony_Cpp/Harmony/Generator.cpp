#include "Generator.h"
#include "Constants.h"

#include <iostream>

using namespace SymEngine;

/*void Generator::compute_y_parameters(double frequency) {
    // Check if parameters are initialized
    if (R_f.size() < 3 || L_f.size() < 3 || X_d.size() < 3 || T_f.size() < 3) {
        std::cerr << "Generator parameters not initialized correctly!" << std::endl;
        return;
    }
    // Use the constant PI from Constants.h
    RCP<const Basic> omega = mul(real_double(2), mul(PI, real_double(frequency)));
    RCP<const Basic> s = mul(I, omega);

    // Assuming R_f, L_f, X_d, T_f are arrays or vectors for three phases
    RCP<const Basic> Y[3][3]; // Y-matrix for three phases

    for (int phase = 0; phase < 3; ++phase) {
        RCP<const Basic> R_f_val = real_double(R_f[phase]);
        RCP<const Basic> L_f_val = real_double(L_f[phase]);
        RCP<const Basic> Z_f = add(R_f_val, mul(s, L_f_val));

        RCP<const Basic> Y_f = div(real_double(1), Z_f);

        RCP<const Basic> X_d_val = real_double(X_d[phase]); // Assuming X_d is also an array
        RCP<const Basic> Z_d = mul(I, X_d_val);

        RCP<const Basic> T_f_val = real_double(T_f[phase]); // Assuming T_f is also an array
        RCP<const Basic> H_f = div(real_double(1), add(mul(T_f_val, s), real_double(1)));

        // Computing Y parameters for each phase
        RCP<const Basic> Y11 = Y_f;
        RCP<const Basic> Y12 = neg(div(H_f, Z_d));
        RCP<const Basic> Y21 = real_double(0);
        RCP<const Basic> Y22 = neg(div(real_double(1), Z_d));

        // You might want to print Y parameters for debugging
        std::cout << "Phase " << phase + 1 << ": "
            << "Y11 = " << eval_double(*Y11) << ", "
            << "Y12 = " << eval_double(*Y12) << ", "
            << "Y21 = " << eval_double(*Y21) << ", "
            << "Y22 = " << eval_double(*Y22) << std::endl;
    }
}*/
