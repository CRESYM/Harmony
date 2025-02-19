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
    if (pins == 0)
        throw std::invalid_argument("Invalid number of pins, must be greater than 0!");

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
            std::cerr << "Load parameters initialized correctly for phase " << i + 1 << "!" << std::endl;
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


//// Function to calculate power flow through the load for each phase
//std::complex<double> Load::compute_power_flow(int phase) {
//    if (phase < 0 || phase >= R.size()) {
//        throw std::out_of_range("Invalid phase index");
//    }
//
//    // Calculate the impedance for the phase
//    std::complex<double> impedance = get_impedance(phase);
//
//    // Compute the current for this phase: I = V / Z
//    std::complex<double> current = voltage_values[phase] / impedance;
//
//    // Compute the power: S = V * I*
//    return voltage_values[phase] * std::conj(current);
//}
//
//// Function to calculate the impedance for a given phase
//std::complex<double> Load::get_impedance(int phase) const {
//    double R_value = getResistance(phase);
//    double L_value = getInductance(phase);
//    double C_value = getCapacitance(phase);
//
//    double omega = 2 * M_PI * 50;  // Example for 50 Hz frequency, change as needed
//
//    // Calculate impedance for R + jωL - j / ωC
//    std::complex<double> impedance(R_value, omega * L_value);
//    impedance -= std::complex<double>(0, 1.0 / (omega * C_value));
//
//    return impedance;
//}
//    


