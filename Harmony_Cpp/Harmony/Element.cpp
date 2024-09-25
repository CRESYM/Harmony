// element.cpp

#include "Element.h"
#include "Constants.h"
#include "Load.h"
#include "Generator.h"
#include "Impedance.h"
#include <iostream>
#include <vector>


using namespace SymEngine;

// Destructor
Element::~Element() {}

// Implementation of compute_y_parameters for single-phase (default implementation)
void Element::compute_y_parameters(double frequency) {
    std::cout << "This should be overridden in derived classes." << std::endl;
}

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


// Implementation of compute_y_parameters for Single-phaseImpedance
//void Impedance::compute_y_parameters(double frequency) {

    /*// Compute angular frequency
    RCP<const Basic> omega = mul(real_double(2), mul(PI, real_double(frequency)));
    RCP<const Basic> j = I;

    // Calculate the impedance
    RCP<const Basic> Z = add(real_double(R), mul(j, real_double(X)));

    // Calculate the admittance
    RCP<const Basic> Y = div(real_double(1), Z);

    // Extract real and imaginary parts by evaluating the expression
    RCP<const Number> Y_eval = rcp_static_cast<const Number>(evalf(*Y, 53));
    double Y_val = eval_double(*Y_eval);

    // If the imaginary part is needed, you can compute it separately
    RCP<const Basic> Y_real = add(real_double(0), mul(j, real_double(0)));  // Replace with appropriate logic if needed

    // Print the results
    std::cout << "|Impedance Y|: " << Y_val << " S" << std::endl;
    std::cout << "|Impedance Y_real|: " << eval_double(*Y_real) << " S" << std::endl; // Adjust this line if necessary
    std::cout << "|Impedance Y_param3|: " << 0.0 << " S" << std::endl;
    std::cout << "|Impedance Y_param4|: " << 0.0 << " S" << std::endl;*/
//}
// 
//Three-phase
void Impedance::compute_y_parameters(double frequency) {
    // Compute angular frequency
    double omega = 2 * M_PI * frequency;

    // Assuming Z_matrix holds the impedance for a 3-phase system
    std::vector<std::vector<double>> Y_matrix(3, std::vector<double>(3, 0));

    // Inverse of the impedance matrix will give the admittance matrix (Y = Z^-1)
    // This requires matrix inversion (for simplicity, using a numerical library would be helpful)

    // TODO: Implement matrix inversion (you can use Eigen library for this)

    // For now, assuming diagonal matrix (impedance only between the same phases):
    for (int i = 0; i < 3; i++) {
        Y_matrix[i][i] = 1 / Z_matrix[i][i];  // Y = 1/Z
    }

    // Output the computed Y-parameters for each phase
    for (int i = 0; i < 3; i++) {
        std::cout << "|Y_matrix[" << (i + 1) << "," << (i + 1) << "]|: " << Y_matrix[i][i] << " S" << std::endl;
    }
}
