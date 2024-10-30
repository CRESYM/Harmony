#include "Transformer_Delta_Delta.h"

// Constructor
TransformerDeltaDelta::TransformerDeltaDelta(const std::string& symbol, int pins, const std::vector<double>& values)
    : Element(symbol, pins, pins){  // Initialize M_delta

    if (values.size() == 8) {
        R = { values[0], values[2] };  // Primary and secondary resistances
        X = { values[1], values[3] };  // Primary and secondary reactances
        a = values[4];  // Turns ratio
        phaseLag = values[5];  // Phase lag in radians
        // No M_Y as both primary and secondary are delta
    }
    else {
        throw std::invalid_argument("Invalid number of values, must be 8 (R_primary, X_primary, R_secondary, X_secondary, a, phaseLag, M_delta)!");
    }


    if (couplingCoefficients.size() != 6) {
        throw std::invalid_argument("Invalid number of coupling coefficients, must be 6!");
    }
    couplingCoeff = couplingCoefficients;

    // Y parameters for the Delta-Delta three-phase transformer
    // We use a 6x6 Y matrix to represent the admittance between the 3 primary windings and 3 secondary windings.

    // Define basic components
    RCP<const Basic> R_primary = real_double(R[0]);
    RCP<const Basic> X_primary = real_double(X[0]);
    RCP<const Basic> R_secondary = real_double(R[1]);
    RCP<const Basic> X_secondary = real_double(X[1]);

    RCP<const Basic> a_val = real_double(a);  // Turns ratio
    RCP<const Basic> phaseFactor = exp(mul(neg(j), real_double(phaseLag)));  // Phase shift factor

    // Compute primary (delta side) and secondary (delta side) admittances
    RCP<const Basic> Y_primary = div(real_double(1), add(R_primary, mul(j, X_primary)));
    RCP<const Basic> Y_secondary = div(real_double(1), add(R_secondary, mul(j, X_secondary)));

    // Define the 3x3 submatrices for the primary and secondary delta side connections
    DenseMatrix Y_primary_matrix(3, 3);
    DenseMatrix Y_secondary_matrix(3, 3);

    // Populate primary side Y matrix (for delta winding)
    Y_primary_matrix.set(0, 0, Y_primary);  // Y11
    Y_primary_matrix.set(0, 1, neg(Y_primary));  // Y12
    Y_primary_matrix.set(0, 2, Y_primary);  // Y13

    Y_primary_matrix.set(1, 0, neg(Y_primary));  // Y21
    Y_primary_matrix.set(1, 1, Y_primary);  // Y22
    Y_primary_matrix.set(1, 2, neg(Y_primary));  // Y23

    Y_primary_matrix.set(2, 0, Y_primary);  // Y31
    Y_primary_matrix.set(2, 1, neg(Y_primary));  // Y32
    Y_primary_matrix.set(2, 2, Y_primary);  // Y33

    // Populate secondary side Y matrix (for delta winding)
    Y_secondary_matrix.set(0, 0, Y_secondary);  // Y11
    Y_secondary_matrix.set(0, 1, neg(Y_secondary));  // Y12
    Y_secondary_matrix.set(0, 2, Y_secondary);  // Y13

    Y_secondary_matrix.set(1, 0, neg(Y_secondary));  // Y21
    Y_secondary_matrix.set(1, 1, Y_secondary);  // Y22
    Y_secondary_matrix.set(1, 2, neg(Y_secondary));  // Y23

    Y_secondary_matrix.set(2, 0, Y_secondary);  // Y31
    Y_secondary_matrix.set(2, 1, neg(Y_secondary));  // Y32
    Y_secondary_matrix.set(2, 2, Y_secondary);  // Y33

    // Assign submatrices to Y_matrix (6x6 for 3 primary and 3 secondary windings)
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            Y_matrix.set(i, j, Y_primary_matrix.get(i, j));  // Y_primary elements
            Y_matrix.set(i + 3, j + 3, Y_secondary_matrix.get(i, j));  // Y_secondary elements

            // Interaction between primary and secondary with phase shift and mutual couplings
            RCP<const Basic> couplingA = mul(real_double(couplingCoeff[i * 3 + j]),
                div(real_double(1), add(Y_primary_matrix.get(i, j), Y_secondary_matrix.get(j, j))));

            Y_matrix.set(i, j + 3, add(neg(mul(a_val, phaseFactor)), Y_primary_matrix.get(i, j)));  // Y12 interaction (primary to secondary with phase shift)
            Y_matrix.set(i + 3, j, add(neg(phaseFactor), Y_secondary_matrix.get(i, j)));  // Y21 interaction (secondary to primary with phase shift)

            // Apply mutual couplings (M_delta for delta winding)
            Y_matrix.set(i, j, add(Y_matrix.get(i, j), mul(real_double(M_delta), couplingA)));  // Add mutual coupling for primary to secondary
            Y_matrix.set(i + 3, j + 3, add(Y_matrix.get(i + 3, j + 3), mul(real_double(M_delta), couplingA)));  // Add mutual coupling for secondary to primary
        }
    }
}

// Destructor
TransformerDeltaDelta::~TransformerDeltaDelta() {
    std::cout << "Transformer Delta-Delta object for " << getElementSymbol() << " destroyed." << std::endl;
}
