// Load.cpp

#include "Load.h"

Load::Load(const std::string& symbol, int pins, std::vector<double> values) : Element(symbol, pins, pins) {
    if (pins == 0)
        throw std::invalid_argument("Invalid number of pins, must be greater than 0!");

    int capacity = values.capacity();
    if (capacity == 3) {
        R = std::vector<double>(pins, values[0]);
        L = std::vector<double>(pins, values[1]);
        C = std::vector<double>(pins, values[2]);
    }
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
}
    


