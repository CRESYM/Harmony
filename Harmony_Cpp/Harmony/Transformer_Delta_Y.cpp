#include "Transformer_Delta_Y.h"


// Constructor
TransformerDeltaY::TransformerDeltaY(const std::string& symbol, int pins, const std::vector<double>& values)
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

    // Y parameters for the grounded star-delta three-phase transformer
    // We use a 6x6 Y matrix to represent the admittance between the 3 primary delta windings and 3 secondary star windings.

    RCP<const Basic> R_delta = real_double(R[0]);
    RCP<const Basic> X_delta = real_double(X[0]);
    RCP<const Basic> R_star = real_double(R[1]);
    RCP<const Basic> X_star = real_double(X[1]);
    RCP<const Basic> a_val = real_double(a);  // Turns ratio symbol

    RCP<const Basic> phaseFactor = exp(mul(neg(j), real_double(phaseLag)));  // Phase shift

    // Compute primary (delta) and secondary (star) admittances
    RCP<const Basic> Y_delta = div(real_double(1), add(R_delta, mul(j, X_delta)));
    RCP<const Basic> Y_star = div(real_double(1), add(R_star, mul(j, X_star)));

    // Define the 3x3 submatrices for the delta and star connections

    DenseMatrix Y_delta_matrix(3, 3);
    DenseMatrix Y_star_matrix(3, 3);

    // Populate delta side Y matrix (for primary side)
    Y_delta_matrix.set(0, 0, Y_delta);  // Y11
    Y_delta_matrix.set(0, 1, mul(real_double(-1), Y_delta));  // Y12
    Y_delta_matrix.set(0, 2, real_double(0));  // No direct interaction between phase A and C

    Y_delta_matrix.set(1, 0, mul(real_double(-1), Y_delta));  // Y21
    Y_delta_matrix.set(1, 1, Y_delta);  // Y22
    Y_delta_matrix.set(1, 2, mul(real_double(-1), Y_delta));  // Y23

    Y_delta_matrix.set(2, 0, real_double(0));  // No direct interaction between phase C and A
    Y_delta_matrix.set(2, 1, mul(real_double(-1), Y_delta));  // Y32
    Y_delta_matrix.set(2, 2, Y_delta);  // Y33

    // Populate star side Y matrix (for secondary side)
    Y_star_matrix.set(0, 0, Y_star);  // Y11
    Y_star_matrix.set(0, 1, real_double(0));  // No direct interaction between phase A and B
    Y_star_matrix.set(0, 2, real_double(0));  // No direct interaction between phase A and C

    Y_star_matrix.set(1, 0, real_double(0));  // No direct interaction between phase B and A
    Y_star_matrix.set(1, 1, Y_star);  // Y22
    Y_star_matrix.set(1, 2, real_double(0));  // No direct interaction between phase B and C

    Y_star_matrix.set(2, 0, real_double(0));  // No direct interaction between phase C and A
    Y_star_matrix.set(2, 1, real_double(0));  // No direct interaction between phase C and B
    Y_star_matrix.set(2, 2, Y_star);  // Y33

    // Assign submatrices to Y_matrix (6x6 for 3 primary and 3 secondary)
    // Top-left 3x3: Y_delta (primary winding)
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            Y_matrix.set(i, j, Y_delta_matrix.get(i, j));  // Y_delta elements
            Y_matrix.set(i + 3, j + 3, Y_star_matrix.get(i, j));  // Y_star elements
            Y_matrix.set(i, j + 3, mul(real_double(-a), Y_delta_matrix.get(i, j)));  // Y12 interaction (primary to secondary)
            Y_matrix.set(i + 3, j, mul(real_double(-1), Y_delta_matrix.get(i, j)));  // Y21 interaction (secondary to primary)
        }
    }
}

// Destructor
TransformerDeltaY::~TransformerDeltaY() {
    std::cout << "Transformer Delta Y object for " << getElementSymbol() << " destroyed." << std::endl;
}
