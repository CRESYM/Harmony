#include "TransmissionLine.h"

TransmissionLine::TransmissionLine(const std::string& symbol, int pins, const std::vector<double>& values) 
    : Element(symbol, pins, pins){
    if (values.size() == 5) {
        R_tl = values[0]; // Resistance per unit length (ohms/m)
        L_tl = values[1]; // Inductance per unit length (H/m)
        G_tl = values[2]; // Conductance per unit length (S/m)
        C_tl = values[3]; // Capacitance per unit length (F/m)
        length = values[4]; // Length of the transmission line (m)
    }
    else {
        throw std::invalid_argument("Invalid number of values, must be 5 (resistance, inductance, conductance, capacitance and length)!");
    }

    RCP<const Basic> R_val = real_double(R_tl);
    RCP<const Basic> L_val = real_double(L_tl);
    RCP<const Basic> G_val = real_double(G_tl);
    RCP<const Basic> C_val = real_double(C_tl);

    RCP<const Basic> omega_L = mul(omega, L_val);
    RCP<const Basic> omega_C = mul(omega, C_val);
    RCP<const Basic> R_plus_i_omega_L = add(R_val, mul(I, omega_L));
    RCP<const Basic> G_plus_i_omega_C = add(G_val, mul(I, omega_C));


    RCP<const Basic> Z0 = sqrt(div(R_plus_i_omega_L, G_plus_i_omega_C));
    RCP<const Basic> gamma = sqrt(mul(R_plus_i_omega_L, G_plus_i_omega_C));

    RCP<const Basic> gamma_l = mul(gamma, real_double(length));
    RCP<const Basic> cosh_gamma_l = cosh(gamma_l);
    RCP<const Basic> sinh_gamma_l = sinh(gamma_l);
    RCP<const Basic> Z0_sinh_gamma_l = mul(Z0, sinh_gamma_l);

    // Compute Y-parameters
    RCP<const Basic> Y11 = div(neg(cosh_gamma_l), Z0_sinh_gamma_l);
    RCP<const Basic> Y12 = div(real_double(1), Z0_sinh_gamma_l);
    RCP<const Basic> Y21 = div(real_double(1), Z0_sinh_gamma_l);
    RCP<const Basic> Y22 = div(cosh_gamma_l, Z0_sinh_gamma_l);
    for (int i = 0; i < pins; i++) {
        Y_matrix.set(i, i, Y11);  // Y11
        Y_matrix.set(i, pins + i, Y12);  // Y12
        Y_matrix.set(pins + i, i, Y21);  // Y21 (symmetrical to Y12)
        Y_matrix.set(pins + i, pins + i, Y22);  // Y22
    }
    
}



   

