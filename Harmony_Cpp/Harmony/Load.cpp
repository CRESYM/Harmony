// Load.cpp

#include "Element.h"
#include "Load.h"
#include "Constants.h"
#include <iostream>

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
        if (R[i] == 0 || L[i] == 0 || C[i] == 0) {
            std::cerr << "Load parameters not initialized correctly for phase " << i + 1 << "!" << std::endl;
            return;
        }
        else {
            std::cerr << "Load parameters initialized correctly for phase " << i + 1 << "!" << std::endl;
        }
    }
}
    
void Load::printElementValues() {
        printElementInfo();
        for (int i = 0; i < input_pins; i++) {
            std::cout << "Resistance connected to pin " << i + 1 << " is: " << R[i] << std::endl;
            std::cout << "Inductance connected to pin " << i + 1 << " is: " << L[i] << std::endl;
            std::cout << "Capacitance connected to pin " << i + 1 << " is: " << C[i] << std::endl;
        }
}

// Load-specific Y-parameters for three-phase
void Load::compute_y_parameters(double frequency) {
    std::cout << "Computing Y-parameters for a three-phase load...\n";

    RCP<const Basic> omega = mul(real_double(2), mul(PI, real_double(frequency)));
    RCP<const Basic> j = I;

    // Matrix for three-phase Y-parameters
    std::vector<std::vector<RCP<const Basic>>> Y_matrix(3, std::vector<RCP<const Basic>>(3));

    // Loop over phases to compute Y-parameters
    for (int i = 0; i < input_pins; ++i) {
        RCP<const Basic> R_val = real_double(R[i]);
        RCP<const Basic> L_val = real_double(L[i]);
        RCP<const Basic> C_val = real_double(C[i]);

        // Print debug info
        std::cout << "Phase " << i + 1 << " -> R: " << eval_double(*R_val)
            << ", L: " << eval_double(*L_val)
            << ", C: " << eval_double(*C_val) << std::endl;

        // jωL
        RCP<const Basic> j_omega_L = mul(j, mul(omega, L_val));
        // Calculate the capacitive reactance: j/(ωC)
        RCP<const Basic> j_omega_C = mul(j, div(real_double(-1), mul(omega, C_val)));

        // Impedance
        RCP<const Basic> Z_RLC = add(add(R_val, j_omega_L), j_omega_C);

        // Ensure Z_RLC is not zero
        if (Z_RLC.get() == nullptr || eval_double(*Z_RLC) == 0.0) {
            std::cerr << "Z_RLC is zero for phase " << i + 1 << std::endl;
            continue; // Skip to next phase
        }

        // Admittance
        Y_matrix[i][i] = div(real_double(1), Z_RLC);
    }

    // Print the Y-parameters for three-phase load
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            double Y_val_abs = eval_double(*abs(Y_matrix[i][j]));
            std::cout << "|Three-phase Load Y" << (i + 1) << (j + 1) << "|: " << Y_val_abs << " S" << std::endl;
        }
    }
}

