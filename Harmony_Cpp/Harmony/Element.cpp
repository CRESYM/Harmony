// element.cpp

#include "element.h"
#include "Constants.h"
#include "Load.h"
#include "Generator.h"
#include "Impedance.h"
#include <iostream>

using namespace SymEngine;

// Destructor
Element::~Element() {}

// Implementation of compute_y_parameters
void Element::compute_y_parameters(double frequency) {
    std::cout << "This should be overridden in derived classes." << std::endl;
}

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

// Load-specific Y-parameters
void Load::compute_y_parameters(double frequency) {

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
}

// Implementation of compute_y_parameters for Impedance
void Impedance::compute_y_parameters(double frequency) {

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
}