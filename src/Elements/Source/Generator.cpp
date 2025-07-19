#include "Generator.h"

// Constructor
Generator::Generator(const std::string& symbol, int pins, const std::vector<double>& values)
    : Element(symbol, pins, pins) {

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

// Power flow computation for AC networks
void Generator::computePowerFlowAC(std::map<std::string, double>& branchData,
    std::map<std::string, double>& globalParams) const {
    branchData["generator"] = 1;

    // Compute generator impedance at operational frequency
    std::complex<double> s = globalParams["omega"] * std::complex<double>(0, 1);
    std::complex<double> Z_eq(R_f, X_d);  // Generator impedance

    branchData["br_r"] = std::real(Z_eq);
    branchData["br_x"] = std::imag(Z_eq);
    branchData["g_fr"] = 0;
    branchData["b_fr"] = 0;
    branchData["g_to"] = 0;
    branchData["b_to"] = 0;
}

// Power flow computation for DC networks
void Generator::computePowerFlowDC(std::map<std::string, double>& branchDCData,
    std::map<std::string, double>& globalParams) const {
    branchDCData["l"] = 0.0;
    branchDCData["c"] = 0.0;

    // Compute generator impedance at DC (zero frequency)
    std::complex<double> Z_eq(R_f, 0.0);  // DC resistance

    branchDCData["r"] = std::real(Z_eq);
}


