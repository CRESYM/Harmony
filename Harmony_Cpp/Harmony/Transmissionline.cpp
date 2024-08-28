#include "TransmissionLine.h"
#include "Element.h"
#include <iostream>

using namespace SymEngine;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void TransmissionLine::compute_y_parameters(double R, double L, double G, double C, double length, double frequency) {
    double omega = 2 * M_PI * frequency;

    RCP<const Symbol> R_sym = symbol("R");
    RCP<const Symbol> L_sym = symbol("L");
    RCP<const Symbol> G_sym = symbol("G");
    RCP<const Symbol> C_sym = symbol("C");
    RCP<const Symbol> omega_sym = symbol("omega");

    RCP<const Basic> R_val = real_double(R);
    RCP<const Basic> L_val = real_double(L);
    RCP<const Basic> G_val = real_double(G);
    RCP<const Basic> C_val = real_double(C);
    RCP<const Basic> omega_val = real_double(omega);

    RCP<const Basic> I_val = I;

    RCP<const Basic> omega_L = mul(omega_val, L_val);
    RCP<const Basic> omega_C = mul(omega_val, C_val);
    RCP<const Basic> R_plus_i_omega_L = add(R_val, mul(I, omega_L));
    RCP<const Basic> G_plus_i_omega_C = add(G_val, mul(I, omega_C));

    std::cout << "omega: " << omega << std::endl;
    std::cout << "omega * L: " << *omega_L << std::endl;
    std::cout << "omega * C: " << *omega_C << std::endl;
    std::cout << "R + j*omega*L: " << *R_plus_i_omega_L << std::endl;
    std::cout << "G + j*omega*C: " << *G_plus_i_omega_C << std::endl;

    RCP<const Basic> Z0 = sqrt(div(R_plus_i_omega_L, G_plus_i_omega_C));
    RCP<const Basic> gamma = sqrt(mul(R_plus_i_omega_L, G_plus_i_omega_C));

    std::cout << "Characteristic Impedance (Z0): " << *Z0 << std::endl;
    std::cout << "Propagation Constant (gamma): " << *gamma << std::endl;

    RCP<const Basic> gamma_l = mul(gamma, real_double(length));
    RCP<const Basic> cosh_gamma_l = cosh(gamma_l);
    RCP<const Basic> sinh_gamma_l = sinh(gamma_l);

    std::cout << "gamma * length: " << *gamma_l << std::endl;
    std::cout << "cosh(gamma * length): " << *cosh_gamma_l << std::endl;
    std::cout << "sinh(gamma * length): " << *sinh_gamma_l << std::endl;

    RCP<const Basic> Z0_sinh_gamma_l = mul(Z0, sinh_gamma_l);
    RCP<const Basic> Y11 = div(neg(cosh_gamma_l), Z0_sinh_gamma_l);
    RCP<const Basic> Y12 = div(real_double(1), Z0_sinh_gamma_l);
    RCP<const Basic> Y21 = div(real_double(1), Z0_sinh_gamma_l);
    RCP<const Basic> Y22 = div(cosh_gamma_l, Z0_sinh_gamma_l);

    std::cout << "Z0_sinh_gamma_l: " << *Z0_sinh_gamma_l << std::endl;
    std::cout << "Y11 (symbolic): " << *Y11 << std::endl;
    std::cout << "Y12 (symbolic): " << *Y12 << std::endl;
    std::cout << "Y21 (symbolic): " << *Y21 << std::endl;
    std::cout << "Y22 (symbolic): " << *Y22 << std::endl;

    RCP<const Basic> Y11_eval = evalf(*Y11, 53);
    RCP<const Basic> Y12_eval = evalf(*Y12, 53);
    RCP<const Basic> Y21_eval = evalf(*Y21, 53);
    RCP<const Basic> Y22_eval = evalf(*Y22, 53);

    double Y11_val = rcp_static_cast<const RealDouble>(Y11_eval)->as_double();
    double Y12_val = rcp_static_cast<const RealDouble>(Y12_eval)->as_double();
    double Y21_val = rcp_static_cast<const RealDouble>(Y21_eval)->as_double();
    double Y22_val = rcp_static_cast<const RealDouble>(Y22_eval)->as_double();

    std::cout << "Transmission Line Y11: " << Y11_val << " S" << std::endl;
    std::cout << "Transmission Line Y12: " << Y12_val << " S" << std::endl;
    std::cout << "Transmission Line Y21: " << Y21_val << " S" << std::endl;
    std::cout << "Transmission Line Y22: " << Y22_val << " S" << std::endl;
}

