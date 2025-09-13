#include "Transmissionline.h"

TransmissionLine::TransmissionLine(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values)
    : Element(symbol, location, pins, pins){
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
    RCP<const Basic> Z0_tanh_gamma_l = div(Z0_sinh_gamma_l, cosh_gamma_l);

    // Compute Y-parameters
    RCP<const Basic> Y11 = div(integer(1), Z0_tanh_gamma_l);
    RCP<const Basic> Y12 = div(real_double(-1), Z0_sinh_gamma_l);
    RCP<const Basic> Y21 = Y12;
    RCP<const Basic> Y22 = Y11;
    for (int i = 0; i < pins; i++) {
        Y_matrix.set(i, i, Y11);  // Y11
        Y_matrix.set(i, pins + i, Y12);  // Y12
        Y_matrix.set(pins + i, i, Y21);  // Y21 (symmetrical to Y12)
        Y_matrix.set(pins + i, pins + i, Y22);  // Y22
    }
    
}

void TransmissionLine::computePowerFlowAC(std::map<std::string, double>& branchData,
    const std::map<std::string, double>& globalParams) const
{
    using cd = std::complex<double>;
    const double omega_num = globalParams.at("omega");
    const double Zbase = globalParams.at("Z_base");

    cd Y11 = substitute_symbol(Y_matrix.get(0, 0), omega, globalParams.at("omega"));
    cd Y12 = substitute_symbol(Y_matrix.get(0, m_pins), omega, globalParams.at("omega"));

    cd Zs = -cd(1.0) / Y12 / globalParams.at("Z_base");    
    cd Yend = (Y11 + Y12);             

    branchData["transformer"] = 0;
    branchData["tap"] = 1.0;
    branchData["shift"] = 0.0;
    branchData["c_rating_a"] = 1.0;

    branchData["r"] = std::real(Zs);
    branchData["x"] = std::imag(Zs);

    branchData["g_fr"] = std::real(Yend);
    branchData["b_fr"] = std::imag(Yend);

    branchData["g_to"] = std::real(Yend);
    branchData["b_to"] = std::imag(Yend);

	branchData["grid"] = (int)element_location[2] - '0'; // Example of setting grid based on element_location

    for (auto& [key, value] : element_OPF_info) 
        branchData[key] = value;
}

void TransmissionLine::computePowerFlowDC(std::map<std::string, double>& branchDCData,
    const std::map<std::string, double>& globalParams) const
{
    using cd = std::complex<double>;

    cd Y12 = substitute_symbol(Y_matrix.get(0, m_pins), omega, 0.0);

    cd Zs = -cd(1.0) / Y12 / globalParams.at("Z_base");

    branchDCData["r"] = std::real(Zs);
    branchDCData["x"] = 0.0;
    branchDCData["b"] = 0.0;

	branchDCData["grid"] = (int)element_location[2] - '0'; // Example of setting grid based on element_location

    for (auto& [key, value] : element_OPF_info)
        branchDCData[key] = value;
}




   

