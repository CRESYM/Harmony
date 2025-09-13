#include "Impedance_base.h"

// Power flow computation for AC networks
void Impedance_base::computePowerFlowAC(std::map<std::string, double>& branchData,
    std::map<std::string, double>& globalParams) const {
    int key = branchData.size();  // Unique branch identifier
    branchData["transformer"] = 0;
    branchData["tap"] = 1.0;
    branchData["shift"] = 0.0;
    branchData["c_rating_a"] = 1.0;
	branchData["grid"] = (int)element_location[2] - '0'; // Example of setting grid based on element_location

    // Compute Y parameters at operational frequency
    std::complex<double> s = globalParams["omega"] * std::complex<double>(0, 1);

    // Convert SymEngine expression to double
    complex<double> Y_00 = substitute_symbol(Y_matrix.get(0, 0), omega, globalParams["omega"]);

    if (Y_00 == std::complex<double>(0, 0)) {
        throw std::runtime_error("Y_matrix(0,0) is zero, division by zero error.");
    }

    std::complex<double> Z_eq = std::complex<double>(1.0) / Y_00 / globalParams["Z_base"];

    branchData["r"] = std::real(Z_eq);
    branchData["x"] = std::imag(Z_eq);
    branchData["b"] = 0;

    for (auto& [key, value] : element_OPF_info)
        branchData[key] = value;
}

// Power flow computation for DC networks
void Impedance_base::computePowerFlowDC(std::map<std::string, double>& branchDCData,
    std::map<std::string, double>& globalParams) const {
    branchDCData["x"] = 0.0;
    branchDCData["b"] = 0.0;
	branchDCData["grid"] = (int)element_location[2] - '0'; // Example of setting grid based on element_location

    // Convert SymEngine expression to double
    complex<double> Y_00 = substitute_symbol(Y_matrix.get(0, 0), omega, 0);

    std::complex<double> Z_eq = std::complex<double>(1.0) / Y_00 / globalParams["Z_base"];

    branchDCData["r"] = std::real(Z_eq);

    for (auto& [key, value] : element_OPF_info)
        branchDCData[key] = value;
}