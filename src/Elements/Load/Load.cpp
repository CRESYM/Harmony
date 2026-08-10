/**
 * @file Load.cpp
 * @brief Implementation of the Load series R-L-C element.
 */
// Load.cpp
#include "Load.h"

/*
 * Load Constructor
 *
 * Initializes the load element based on the number of pins and the provided R, L, and C values.
 * Depending on the input vector size, the constructor either initializes all phases with uniform values,
 * or with phase-specific values for R, L, and C.
 */
Load::Load(const std::string& symbol, const std::string& location, int pins, std::vector<double> values) : Load_base(symbol, location, pins, pins) {
    if (pins <= 0) {
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
        RCP<const Basic> j_omega_C;
        if (C[i] != 0) {
            j_omega_C = mul(j, div(real_double(-1), mul(omega, C_val)));
		}
        else {
			j_omega_C = real_double(0);  // No capacitive effect if C is zero
        }
        

        // Impedance
        RCP<const Basic> Z_RLC = add(add(R_val, j_omega_L), j_omega_C);

        // Admittance
        Y_matrix.set(i, i, div(real_double(1), Z_RLC));
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
void Load::computePowerFlow(std::map<std::string, double>& busAC,
    std::map<std::string, double>& global_params) const {

    std::string area = element_location.substr(0, 2);
    if ((area[0] == 'A' || area[0] == 'a') && (area[1] == 'c' || area[1] == 'C')) {

        // --- Retrieve basic parameters ---
        double V_LL = global_params["ACbaseKV"] * 1e3;   // convert kV → V
        double omega = global_params["omega"];           // rad/s
        double V_phase = V_LL / std::sqrt(3.0);          // phase voltage (V)

        // --- Series RLC model ---
        double Rl = (R[0] == 0.0) ? 1e-12 : R[0];
        double Xl = (L[0] == 0.0) ? 0.0 : omega * L[0];
        double Xc = (C[0] == 0.0) ? 0.0 : -1.0 / (omega * C[0]);
        double X = Xl + Xc;

        double Z_real = Rl;
        double Z_imag = X;
        double Z_mag2 = Z_real * Z_real + Z_imag * Z_imag;

        // --- Power calculation (in W / var) ---
        double Pd_W = 3.0 * V_phase * V_phase * Z_real / Z_mag2;
        double Qd_var = 3.0 * V_phase * V_phase * Z_imag / Z_mag2;

        // --- Convert to MW / Mvar for storage ---
        double Pd = Pd_W / 1e6;
        double Qd = Qd_var / 1e6;

        busAC["Pd"] += Pd;
        busAC["Qd"] += Qd;

    }
    else {
        throw std::runtime_error("Invalid network type specified in global parameters.");
    }
}


