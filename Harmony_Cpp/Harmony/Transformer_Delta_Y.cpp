#include "Transformer_Delta_Y.h"

// Constructor
TransformerDeltaY::TransformerDeltaY(const std::string& symbol, int pins, const std::vector<double>& values,
    const std::vector<double>& mutualInductances, const std::vector<double>& couplingCoefficients)
    : Element(symbol, pins, pins) {

    if (values.size() == 8) {
        R = { values[0], values[2] };  // Primary (Delta) and Secondary (Y) resistances
        X = { values[1], values[3] };  // Primary (Delta) and Secondary (Y) reactances
        a = values[4];  // Turns ratio
        phaseLag = values[5];  // Phase lag in radians
        M_delta = values[6];  // Mutual reactance in Delta winding
        M_star = values[7];   // Mutual reactance in Star winding
    }
    else {
        throw std::invalid_argument("Invalid number of values, must be 8 (R_primary, X_primary, R_secondary, X_secondary, a, phaseLag, M_delta, M_star)!");
    }

    // Initialize mutual inductances and coupling coefficients
    if (mutualInductances.size() != 6) {
        throw std::invalid_argument("Invalid number of mutual inductances, must be 6!");
    }
    mutualInd = mutualInductances;

    if (couplingCoefficients.size() != 6) {
        throw std::invalid_argument("Invalid number of coupling coefficients, must be 6!");
    }
    couplingCoeff = couplingCoefficients;

    // Y parameters for the grounded star-delta three-phase transformer
    // We use a 6x6 Y matrix to represent the admittance between the 3 primary delta windings and 3 secondary star windings.

    // Define basic components
    RCP<const Basic> R_delta = real_double(R[0]);
    RCP<const Basic> X_delta = real_double(X[0]);
    RCP<const Basic> R_star = real_double(R[1]);
    RCP<const Basic> X_star = real_double(X[1]);

    RCP<const Basic> a_val = real_double(a);  // Turns ratio
    RCP<const Basic> phaseFactor = exp(mul(neg(j), real_double(phaseLag)));  // Phase shift factor (e^(-j*phaseLag))

    // Compute primary (Delta) and secondary (Y) admittances
    RCP<const Basic> Y_delta = div(real_double(1), add(R_delta, mul(j, X_delta)));
    RCP<const Basic> Y_star = div(real_double(1), add(R_star, mul(j, X_star)));

    // Define the 3x3 submatrices for the delta and star connections
    DenseMatrix Y_delta_matrix(3, 3);
    DenseMatrix Y_star_matrix(3, 3);

    // Populate delta side Y matrix (primary side)
    Y_delta_matrix.set(0, 0, Y_delta);  // Y11 (Phase A)
    Y_delta_matrix.set(0, 1, neg(Y_delta));  // Y12 (A to B)
    Y_delta_matrix.set(0, 2, Y_delta);  // Y13 (A to C)

    Y_delta_matrix.set(1, 0, neg(Y_delta));  // Y21 (B to A)
    Y_delta_matrix.set(1, 1, Y_delta);  // Y22 (Phase B)
    Y_delta_matrix.set(1, 2, neg(Y_delta));  // Y23 (B to C)

    Y_delta_matrix.set(2, 0, Y_delta);  // Y31 (C to A)
    Y_delta_matrix.set(2, 1, neg(Y_delta));  // Y32 (C to B)
    Y_delta_matrix.set(2, 2, Y_delta);  // Y33 (Phase C)

    // Populate star side Y matrix (secondary side)
    Y_star_matrix.set(0, 0, Y_star);  // Y11 (Phase A)
    Y_star_matrix.set(0, 1, neg(Y_star));  // Y12 (A to B)
    Y_star_matrix.set(0, 2, Y_star);  // Y13 (A to C)

    Y_star_matrix.set(1, 0, neg(Y_star));  // Y21 (B to A)
    Y_star_matrix.set(1, 1, Y_star);  // Y22 (Phase B)
    Y_star_matrix.set(1, 2, neg(Y_star));  // Y23 (B to C)

    Y_star_matrix.set(2, 0, Y_star);  // Y31 (C to A)
    Y_star_matrix.set(2, 1, neg(Y_star));  // Y32 (C to B)
    Y_star_matrix.set(2, 2, Y_star);  // Y33 (Phase C)

    // Assign submatrices to Y_matrix (6x6 for 3 primary and 3 secondary)
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            Y_matrix.set(i, j, Y_delta_matrix.get(i, j));         // Primary winding (Delta side)
            Y_matrix.set(i + 3, j + 3, Y_star_matrix.get(i, j)); // Secondary winding (Star side)

            // Interaction between primary and secondary with phase shift and mutual couplings
            RCP<const Basic> couplingA = mul(real_double(couplingCoeff[i * 3 + j]),
                div(real_double(1), add(Y_delta_matrix.get(i, j), Y_star_matrix.get(j, j))));

            Y_matrix.set(i, j + 3, add(neg(mul(a_val, phaseFactor)), Y_delta_matrix.get(i, j)));  // Y12 interaction (primary to secondary with phase shift)
            Y_matrix.set(i + 3, j, add(neg(phaseFactor), Y_delta_matrix.get(i, j)));  // Y21 interaction (secondary to primary with phase shift)

            // Apply mutual couplings
            Y_matrix.set(i, j + 3, add(Y_matrix.get(i, j + 3), couplingA)); // Add mutual coupling for primary to secondary
            Y_matrix.set(i + 3, j, add(Y_matrix.get(i + 3, j), couplingA)); // Add mutual coupling for secondary to primary
        }
    }
}

// Destructor
TransformerDeltaY::~TransformerDeltaY() {
    std::cout << "Transformer Delta Y object for " << getElementSymbol() << " destroyed." << std::endl;
}

