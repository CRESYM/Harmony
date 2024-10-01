#include "transformer_delta_delta.h"
#include <cmath>

// Constructor
TransformerDeltaDelta::TransformerDeltaDelta(const std::string& symbol, int pins, const std::vector<double>& values)
    : Element(symbol, pins, pins) {

    if (values.size() == 6) {
        R = { values[0], values[2] };  // Primary and secondary resistances
        X = { values[1], values[3] };  // Primary and secondary reactances
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

    // Y parameters matrix for the three-phase delta-delta transformer
    // We use a 6x6 Y matrix to represent the admittance between the 3 primary and 3 secondary windings.

    RCP<const Basic> R_p = real_double(R[0]);
    RCP<const Basic> X_p = real_double(X[0]);
    RCP<const Basic> R_s = real_double(R[1]);
    RCP<const Basic> X_s = real_double(X[1]);
    RCP<const Basic> a_val = real_double(a); // Turns ratio symbol
    RCP<const Basic> phaseFactor = exp(mul(real_double(-1), j, real_double(phaseLag))); // Phase shift

    // Compute primary and secondary admittances
    RCP<const Basic> Y_p = div(real_double(1), add(R_p, mul(j, X_p)));
    RCP<const Basic> Y_s = div(real_double(1), add(R_s, mul(j, X_s)));

    // Define the 3x3 submatrices for the delta connection. 
    // These account for the mutual interaction between phases A, B, and C.
    
    Matrix Y_delta_p(3, 3);
    Matrix Y_delta_s(3, 3);

    // Populate primary and secondary Y matrices
    // Delta connection has diagonal and off-diagonal elements due to mutual coupling

    // Y matrix for primary (Yp) and secondary (Ys) windings
    // Mutual elements based on delta configuration (using -1 for delta phase shifts)
    
    // Populate Y for primary side
    Y_delta_p.set(0, 0, Y_p);  // Y11
    Y_delta_p.set(0, 1, mul(real_double(-1), Y_p));  // Y12
    Y_delta_p.set(0, 2, real_double(0));  // No direct interaction between phase A and C
    
    Y_delta_p.set(1, 0, mul(real_double(-1), Y_p));  // Y21
    Y_delta_p.set(1, 1, Y_p);  // Y22
    Y_delta_p.set(1, 2, mul(real_double(-1), Y_p));  // Y23
    
    Y_delta_p.set(2, 0, real_double(0));  // No direct interaction between phase C and A
    Y_delta_p.set(2, 1, mul(real_double(-1), Y_p));  // Y32
    Y_delta_p.set(2, 2, Y_p);  // Y33

    // Populate Y for secondary side
    Y_delta_s.set(0, 0, Y_s);  // Y11
    Y_delta_s.set(0, 1, mul(real_double(-1), Y_s));  // Y12
    Y_delta_s.set(0, 2, real_double(0));  // No direct interaction between phase A and C
    
    Y_delta_s.set(1, 0, mul(real_double(-1), Y_s));  // Y21
    Y_delta_s.set(1, 1, Y_s);  // Y22
    Y_delta_s.set(1, 2, mul(real_double(-1), Y_s));  // Y23
    
    Y_delta_s.set(2, 0, real_double(0));  // No direct interaction between phase C and A
    Y_delta_s.set(2, 1, mul(real_double(-1), Y_s));  // Y32
    Y_delta_s.set(2, 2, Y_s);  // Y33

    // Assign submatrices to Y_matrix (6x6 for 3 primary and 3 secondary)
    // Top-left 3x3: Yp (primary winding)
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            Y_matrix.set(i, j, Y_delta_p.get(i, j));  // Yp elements
            Y_matrix.set(i + 3, j + 3, Y_delta_s.get(i, j));  // Ys elements
            Y_matrix.set(i, j + 3, mul(real_double(-a), Y_delta_p.get(i, j)));  // Y12 interaction
            Y_matrix.set(i + 3, j, mul(real_double(-1), Y_delta_p.get(i, j)));  // Y21 interaction
        }
    }
}

// Destructor
TransformerDeltaDelta::~TransformerDeltaDelta() {
    std::cout << "TransformerDeltaDelta object for " << getElementSymbol() << " destroyed." << std::endl;
}
