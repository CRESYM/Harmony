// Load.cpp

#include "Element.h"
#include "Load.h"
#include "Constants.h"
#include <iostream>

// Load-specific Y-parameters for three-phase

void Load::compute_y_parameters(double frequency) {
    std::cout << "Computing Y-parameters for a three-phase load...\n";

    // Check for initialization
    for (int i = 0; i < 3; ++i) {
        if (R[i] == 0 || L[i] == 0 || C[i] == 0) {
            std::cerr << "Load parameters not initialized correctly for phase " << i + 1 << "!" << std::endl;
            return;
        }
        else {
            std::cerr << "Load parameters initialized correctly for phase " << i + 1 << "!" << std::endl;
        }
    }

    RCP<const Basic> omega = mul(real_double(2), mul(PI, real_double(frequency)));
    RCP<const Basic> j = I;

    // Matrix for three-phase Y-parameters
    std::vector<std::vector<RCP<const Basic>>> Y_matrix(3, std::vector<RCP<const Basic>>(3));

    // Loop over phases to compute Y-parameters
    for (int i = 0; i < 3; ++i) {
        RCP<const Basic> R_val = real_double(R[i]);
        RCP<const Basic> L_val = real_double(L[i]);
        RCP<const Basic> C_val = real_double(C[i]);

        // Print debug info
        std::cout << "Phase " << i + 1 << " -> R: " << eval_double(*R_val)
            << ", L: " << eval_double(*L_val)
            << ", C: " << eval_double(*C_val) << std::endl;

        // jωL
        RCP<const Basic> j_omega_L = mul(j, mul(omega, L_val));
        // Calculate the capacitive reactance: j/(ωC)
        RCP<const Basic> j_omega_C = mul(j, div(real_double(-1), mul(omega, C_val)));

        // Impedance
        RCP<const Basic> Z_RLC = add(add(R_val, j_omega_L), j_omega_C);

        // Ensure Z_RLC is not zero
        if (Z_RLC.get() == nullptr || eval_double(*Z_RLC) == 0.0) {
            std::cerr << "Z_RLC is zero for phase " << i + 1 << std::endl;
            continue; // Skip to next phase
        }

        // Admittance
        Y_matrix[i][i] = div(real_double(1), Z_RLC);
    }

    // Print the Y-parameters for three-phase load
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            double Y_val_abs = eval_double(*abs(Y_matrix[i][j]));
            std::cout << "|Three-phase Load Y" << (i + 1) << (j + 1) << "|: " << Y_val_abs << " S" << std::endl;
        }
    }
}

// Load-specific Y-parameters
//Single-phase
/*void Load::compute_y_parameters(double frequency) {

    RCP<const Basic> omega = mul(real_double(2), mul(PI, real_double(frequency)));


    RCP<const Basic> R_val = real_double(R);
    RCP<const Basic> L_val = real_double(L);
    RCP<const Basic> C_val = real_double(C);
    RCP<const Basic> j = I;

    RCP<const Basic> Z_RLC = add(add(R_val, mul(j, mul(omega, L_val))), mul(j, div(real_double(-1), mul(omega, C_val))));

    RCP<const Basic> Y_RLC = div(real_double(1), Z_RLC);

    RCP<const Basic> Y_param1 = Y_RLC;
    RCP<const Basic> Y_param2 = real_double(0);
    RCP<const Basic> Y_param3 = real_double(0);
    RCP<const Basic> Y_param4 = real_double(0);

    RCP<const Number> Y_param1_eval = rcp_static_cast<const Number>(evalf(*Y_param1, 53));
    RCP<const Number> Y_param2_eval = rcp_static_cast<const Number>(evalf(*Y_param2, 53));
    RCP<const Number> Y_param3_eval = rcp_static_cast<const Number>(evalf(*Y_param3, 53));
    RCP<const Number> Y_param4_eval = rcp_static_cast<const Number>(evalf(*Y_param4, 53));

    RCP<const Basic> Y_param1_abs = abs(Y_param1_eval);

    double Y_param1_abs_val = eval_double(*Y_param1_abs);

    double Y_param2_abs = 0.0;
    double Y_param3_abs = 0.0;
    double Y_param4_abs = 0.0;

    std::cout << "|Load Y_param1|: " << Y_param1_abs_val << " S" << std::endl;
    std::cout << "|Load Y_param2|: " << Y_param2_abs << " S" << std::endl;
    std::cout << "|Load Y_param3|: " << Y_param3_abs << " S" << std::endl;
    std::cout << "|Load Y_param4|: " << Y_param4_abs << " S" << std::endl;
}*/
