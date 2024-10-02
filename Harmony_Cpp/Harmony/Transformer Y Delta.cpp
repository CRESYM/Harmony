#include "transformer Y Delta.h"
#include <cmath>

// Constructor
TransformerYDelta::TransformerYDelta(const std::string& symbol, int pins, const std::vector<double>& values)
    : Element(symbol, pins, pins) {

    if (values.size() == 6) {
        R = { values[0], values[2] };  // Primary (Y side) and secondary (Delta side) resistances
        X = { values[1], values[3] };  // Primary (Y side) and secondary (Delta side) reactances
        a = values[4];  // Turns ratio
        phaseLag = values[5];  // Phase lag
    }
    else {
        throw std::invalid_argument("Invalid number of values, must be 6 (R_primary, X_primary, R_secondary, X_secondary, a, phaseLag)!");
    }

    // Check if the values are properly initialized
    for (int i = 0; i < 2; ++i) {
        if (R[i] == 0 || X[i] == 0 || a == 0) {
            std::cerr << "Transformer parameters not initialized correctly for winding " << i + 1 << "!" << std::endl;
            return;
        }
    }

    // Y parameters matrix for the three-phase Y-Delta transformer
    // We use a 6x6 Y matrix to represent the admittance between the 3 primary (Y) and 3 secondary (Delta) windings.

    RCP<const Basic> R_p = real_double(R[0]);  // Primary (Y side) resistance
    RCP<const Basic> X_p = real_double(X[0]);  // Primary (Y side) reactance
    RCP<const Basic> R_s = real_double(R[1]);  // Secondary (Delta side) resistance
    RCP<const Basic> X_s = real_double(X[1]);  // Secondary (Delta side) reactance
    RCP<const Basic> a_val = real_double(a);  // Turns ratio

    // Compute primary and secondary admittances
    RCP<const Basic> Y_p = div(real_double(1), add(R_p, mul(j, X_p)));  // Admittance for Y side
    RCP<const Basic> Y_s = div(real_double(1), add(R_s, mul(j, X_s)));  // Admittance for Delta side

    // Define the 3x3 submatrices for the primary Y side and the secondary Delta side.
    DenseMatrix Y_y(3, 3);  // Primary Y side
    DenseMatrix Y_delta(3, 3);  // Secondary Delta side

    // Populate Y matrix for primary (Y side)
    Y_y.set(0, 0, Y_p);  // Y11
    Y_y.set(0, 1, mul(real_double(-1), Y_p));  // Y12
    Y_y.set(0, 2, real_double(0));  // No direct interaction between phase A and C

    Y_y.set(1, 0, mul(real_double(-1), Y_p));  // Y21
    Y_y.set(1, 1, Y_p);  // Y22
    Y_y.set(1, 2, mul(real_double(-1), Y_p));  // Y23

    Y_y.set(2, 0, real_double(0));  // No direct interaction between phase C and A
    Y_y.set(2, 1, mul(real_double(-1), Y_p));  // Y32
    Y_y.set(2, 2, Y_p);  // Y33

    // Populate Y matrix for secondary (Delta side)
    Y_delta.set(0, 0, Y_s);  // Y11
    Y_delta.set(0, 1, mul(real_double(-1), Y_s));  // Y12
    Y_delta.set(0, 2, real_double(0));  // No direct interaction between phase A and C

    Y_delta.set(1, 0, mul(real_double(-1), Y_s));  // Y21
    Y_delta.set(1, 1, Y_s);  // Y22
    Y_delta.set(1, 2, mul(real_double(-1), Y_s));  // Y23

    Y_delta.set(2, 0, real_double(0));  // No direct interaction between phase C and A
    Y_delta.set(2, 1, mul(real_double(-1), Y_s));  // Y32
    Y_delta.set(2, 2, Y_s);  // Y33

    // Assign submatrices to Y_matrix (6x6 for 3 primary and 3 secondary windings)
    // Top-left 3x3: Y (Y side)
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            Y_matrix.set(i, j, Y_y.get(i, j));  // Primary Y elements
            Y_matrix.set(i + 3, j + 3, Y_delta.get(i, j));  // Secondary Delta elements
            Y_matrix.set(i, j + 3, mul(real_double(-a), Y_y.get(i, j)));  // Y12 interaction
            Y_matrix.set(i + 3, j, mul(real_double(-1), Y_y.get(i, j)));  // Y21 interaction
        }
    }
}

// Destructor
TransformerYDelta::~TransformerYDelta() {
    std::cout << "TransformerYDelta object for " << getElementSymbol() << " destroyed." << std::endl;
}
