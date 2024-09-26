#include "Generator.h"
#include "Constants.h"

#include <iostream>

using namespace SymEngine;

//// Generator-specific Y-parameters for Single-phase
void Generator::compute_y_parameters(double frequency) {
    // Use the constant PI from Constants.h
    RCP<const Basic> omega = mul(real_double(2), mul(PI, real_double(frequency)));

    RCP<const Basic> s = mul(I, omega);

    RCP<const Basic> R_f_val = real_double(R_f);
    RCP<const Basic> L_f_val = real_double(L_f);
    RCP<const Basic> Z_f = add(R_f_val, mul(s, L_f_val));

    RCP<const Basic> Y_f = div(real_double(1), Z_f);

    RCP<const Basic> X_d_val = real_double(X_d);
    RCP<const Basic> Z_d = mul(I, X_d_val);

    RCP<const Basic> T_f_val = real_double(T_f);
    RCP<const Basic> H_f = div(real_double(1), add(mul(T_f_val, s), real_double(1)));

    RCP<const Basic> Y11 = Y_f;
    RCP<const Basic> Y12 = neg(div(H_f, Z_d));
    RCP<const Basic> Y21 = real_double(0);
    RCP<const Basic> Y22 = neg(div(real_double(1), Z_d));

    RCP<const Number> Y11_eval = rcp_static_cast<const Number>(evalf(*Y11, 53));
    RCP<const Number> Y12_eval = rcp_static_cast<const Number>(evalf(*Y12, 53));
    RCP<const Number> Y21_eval = rcp_static_cast<const Number>(evalf(*Y21, 53));
    RCP<const Number> Y22_eval = rcp_static_cast<const Number>(evalf(*Y22, 53));

    RCP<const Basic> Y11_abs = abs(Y11_eval);
    RCP<const Basic> Y12_abs = abs(Y12_eval);
    RCP<const Basic> Y21_abs = abs(Y21_eval);
    RCP<const Basic> Y22_abs = abs(Y22_eval);

    double Y11_abs_val = eval_double(*Y11_abs);
    double Y12_abs_val = eval_double(*Y12_abs);
    double Y21_abs_val = eval_double(*Y21_abs);
    double Y22_abs_val = eval_double(*Y22_abs);

    std::cout << "|Generator Y11|: " << Y11_abs_val << " S" << std::endl;
    std::cout << "|Generator Y12|: " << Y12_abs_val << " S" << std::endl;
    std::cout << "|Generator Y21|: " << Y21_abs_val << " S" << std::endl;
    std::cout << "|Generator Y22|: " << Y22_abs_val << " S" << std::endl;
}

// Generator-specific Y-parameters for three-phase
/*void Generator::compute_y_parameters(double frequency) {
    std::cout << "Generator Y-parameters calculation started..." << std::endl;
    // Step 1: Compute omega and s
    std::cout << "Computing omega and s..." << std::endl;

    // Check if frequency is a valid value (optional, depending on requirements)
    if (frequency <= 0) {
        std::cerr << "Invalid frequency value!" << std::endl;
        return;
    }

    // Check if parameters are initialized
    if (R_f.size() < 3 || L_f.size() < 3 || X_d.size() < 3 || T_f.size() < 3) {
        std::cerr << "Generator parameters not initialized correctly!" << std::endl;
        return;
    }

    // Use the constant PI from Constants.h
    RCP<const Basic> omega = mul(real_double(2), mul(PI, real_double(frequency)));
    RCP<const Basic> s = mul(I, omega);

    // Ensure R_f, L_f are properly used
    for (int phase = 0; phase < 3; ++phase) {
        // Get values for the current phase
        std::cout << "Computing for phase " << phase + 1 << "..." << std::endl;
        std::cout << "R_f: " << R_f[phase] << ", L_f: " << L_f[phase]
            << ", X_d: " << X_d[phase] << ", T_f: " << T_f[phase] << std::endl;

        RCP<const Basic> R_f_val = real_double(R_f[phase]);
        if (R_f_val.get() == nullptr || eval_double(*R_f_val) == 0.0) {
            std::cerr << "R_f_val is zero for phase " << phase + 1 << std::endl;
            continue; // Skip to next phase
        }
        else {
            std::cerr << "R_f_val is not zero for phase " << phase + 1 << std::endl;
        }

        RCP<const Basic> L_f_val = real_double(L_f[phase]);
        RCP<const Basic> Z_f = add(R_f_val, mul(s, L_f_val));
        if (Z_f.get() == nullptr || eval_double(*Z_f) == 0.0) {
            std::cerr << "Z_f is zero for phase " << phase + 1 << std::endl;
            continue; // Skip to next phase
        }
        else {
            std::cerr << "Z_f is not zero for phase " << phase + 1 << std::endl;
        }

        RCP<const Basic> Y_f = div(real_double(1), Z_f);
        RCP<const Basic> X_d_val = real_double(X_d[phase]);
        RCP<const Basic> Z_d = mul(I, X_d_val);
        RCP<const Basic> T_f_val = real_double(T_f[phase]);
        RCP<const Basic> H_f = div(real_double(1), add(mul(T_f_val, s), real_double(1)));

        // Computing Y parameters for each phase
        RCP<const Basic> Y11 = Y_f;
        RCP<const Basic> Y12 = neg(div(H_f, Z_d));
        RCP<const Basic> Y21 = real_double(0);
        RCP<const Basic> Y22 = neg(div(real_double(1), Z_d));


        // Print Y parameters for debugging
        std::cout << "Phase " << phase + 1 << ": "
                  << "Y11 = " << eval_double(*Y11) << ", "
                  << "Y12 = " << eval_double(*Y12) << ", "
                  << "Y21 = " << eval_double(*Y21) << ", "
                  << "Y22 = " << eval_double(*Y22) << std::endl;
    }
}*/
