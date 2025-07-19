#include "Load_base.h"

// Power flow computation for AC networks
void Load_base::computePowerFlowAC(std::map<std::string, double>& branchData,
    std::map<std::string, double>& globalParams) const {

    //// Compute impedance at operational frequency
    //std::complex<double> s = globalParams["omega"] * std::complex<double>(0, 1);

    //std::complex<double> Z_eq(0, 0);
    //for (size_t i = 0; i < R.size(); i++) {
    //    std::complex<double> Z_phase = std::complex<double>(R[i], globalParams["omega"] * L[i]);
    //    if (C[i] != 0) {
    //        Z_phase += std::complex<double>(0, -1.0 / (globalParams["omega"] * C[i]));
    //    }
    //    Z_eq += Z_phase;
    //}
    //Z_eq /= static_cast<double>(R.size());  // Averaging across phases

    branchData["br_r"] = 0; // std::real(Z_eq);
    branchData["br_x"] = 0; // std::imag(Z_eq);
    branchData["g_fr"] = 0;
    branchData["b_fr"] = 0;
    branchData["g_to"] = 0;
    branchData["b_to"] = 0;
}


// Power flow computation for DC networks
void Load_base::computePowerFlowDC(std::map<std::string, double>& branchDCData,
    std::map<std::string, double>& globalParams) const {
    branchDCData["l"] = 0.0;
    branchDCData["c"] = 0.0;

    //std::complex<double> Z_eq(0, 0);
    //for (size_t i = 0; i < R.size(); i++) {
    //    Z_eq += std::complex<double>(R[i], 0.0);  // DC only considers resistance
    //}
    //Z_eq /= static_cast<double>(R.size());  // Averaging across phases

    branchDCData["r"] = 0; // std::real(Z_eq);
}
