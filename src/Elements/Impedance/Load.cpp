// Load.cpp
#include "Load.h"

/*
 * Load Constructor
 *
 * Initializes the load element based on the number of pins and the provided R, L, and C values.
 * Depending on the input vector size, the constructor either initializes all phases with uniform values,
 * or with phase-specific values for R, L, and C.
 */
Load::Load(const std::string& symbol, int pins, std::vector<double> values) : Element(symbol, pins, pins) {
    if (pins <= 0){
        throw std::invalid_argument("Invalid number of pins, must be greater than 0!");
    }

    int capacity = values.capacity();

    // Case 1: Uniform R, L, C across all phases (3 values)
    if (capacity == 3) {
        R = std::vector<double>(pins, values[0]);
        L = std::vector<double>(pins, values[1]);
        C = std::vector<double>(pins, values[2]);
    }
    // Case 2: Phase-specific R, L, C values (3 * pins values)
    else if (capacity == 3 * pins) {
        for (int i = 0; i < pins; i++) {
            R.push_back(values[i]);
            L.push_back(values[pins + i]);
            C.push_back(values[2 * pins + 1]);
        }
    }
    else
        throw std::invalid_argument("Invalid number of values, it should be equal to the number pins, or 3 x number of pins.");
    
    // Check for initialization
    for (int i = 0; i < pins; ++i) {
        if (R[i] == 0 && L[i] == 0 && C[i] == 0) {
            std::cerr << "Load parameters not initialized correctly for phase " << i + 1 << "!" << std::endl;
            return;
        }
        else if (R[i] < 0 || L[i] < 0 || C[i] < 0) {
            std::cerr << "Load parameters not initialized correctly for phase " << i + 1 << "!" << std::endl;
            return;
        }
        else {
            // std::cerr << "Load parameters initialized correctly for phase " << i + 1 << "!" << std::endl;
        }
    }

    // Loop over phases to compute Y-parameters
    for (int i = 0; i < input_pins; ++i) {
        RCP<const Basic> R_val = real_double(R[i]);
        RCP<const Basic> L_val = real_double(L[i]);
        RCP<const Basic> C_val = real_double(C[i]);


        // Calculate the impedance part from inductance: jωL
        RCP<const Basic> j_omega_L = mul(j, mul(omega, L_val));
        // Calculate the capacitive impedance: j/(ωC)
        RCP<const Basic> j_omega_C = mul(j, div(real_double(-1), mul(omega, C_val)));

        // Impedance
        RCP<const Basic> Z_RLC = add(add(R_val, j_omega_L), j_omega_C);

        // Admittance
        Y_matrix.set(i,i, div(real_double(1), Z_RLC));
    }

    // Fill in the complete Y parameters (for symmetrical matrix representation)
    for (int i = 0; i < pins; i++)
        for (int j = 0; j < pins; j++) {
            Y_matrix.set(pins + i, j, sub(zero, Y_matrix.get(i, j)));
            Y_matrix.set(pins + i, pins + j, Y_matrix.get(i, j));
            Y_matrix.set(i, pins + j, sub(zero, Y_matrix.get(i, j)));
        }
}


// Power flow computation for AC networks
void Load::computePowerFlowAC(std::map<std::string, std::map<std::string, double>>& branchData,
    std::map<std::string, double>& globalParams) const {
    int key = branchData.size();  // Unique branch identifier
    branchData[std::to_string(key)]["load"] = 1;

    // Compute impedance at operational frequency
    std::complex<double> s = globalParams["omega"] * std::complex<double>(0, 1);

    std::complex<double> Z_eq(0, 0);
    for (size_t i = 0; i < R.size(); i++) {
        std::complex<double> Z_phase = std::complex<double>(R[i], globalParams["omega"] * L[i]);
        if (C[i] != 0) {
            Z_phase += std::complex<double>(0, -1.0 / (globalParams["omega"] * C[i]));
        }
        Z_eq += Z_phase;
    }
    Z_eq /= static_cast<double>(R.size());  // Averaging across phases

    branchData[std::to_string(key)]["br_r"] = std::real(Z_eq);
    branchData[std::to_string(key)]["br_x"] = std::imag(Z_eq);
    branchData[std::to_string(key)]["g_fr"] = 0;
    branchData[std::to_string(key)]["b_fr"] = 0;
    branchData[std::to_string(key)]["g_to"] = 0;
    branchData[std::to_string(key)]["b_to"] = 0;
}

Load::Load(const std::string& name, int phases) : Element(name, phases) {
    static std::unordered_map<std::string, std::vector<std::string>> default_info = {
        {"LOAD01", {"LOAD01", "1", "345", "0",    "0",    "0"}},
        {"LOAD02", {"LOAD02", "1", "345", "5950", "37.9", "0"}},
        {"LOAD03", {"LOAD03", "1", "345", "2650", "25.2", "0"}},
        {"LOAD04", {"LOAD04", "1", "345", "2976", "75.7", "0"}},
        {"LOAD05", {"LOAD05", "1", "345", "1984", "37.9", "0"}}
    };

    auto it = default_info.find(name);
    element_info = (it != default_info.end()) ? it->second : std::vector<std::string>{ name, "1", "345", "0", "0", "0" };
}

std::vector<std::string> Load::getElementInfo() const {
    return element_info;
}


// Power flow computation for DC networks
void Load::computePowerFlowDC(std::map<std::string, std::map<std::string, double>>& branchDCData,
    std::map<std::string, double>& globalParams) const {
    int key = branchDCData.size();  // Unique DC branch identifier
    branchDCData[std::to_string(key)]["l"] = 0.0;
    branchDCData[std::to_string(key)]["c"] = 0.0;

    std::complex<double> Z_eq(0, 0);
    for (size_t i = 0; i < R.size(); i++) {
        Z_eq += std::complex<double>(R[i], 0.0);  // DC only considers resistance
    }
    Z_eq /= static_cast<double>(R.size());  // Averaging across phases

    branchDCData[std::to_string(key)]["r"] = std::real(Z_eq);
}


