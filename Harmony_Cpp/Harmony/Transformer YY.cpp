#include "Transformer YY.h"
#include <cmath>

// Constructor
TransformerWyeWye::TransformerWyeWye(const std::string& symbol, int pins, const std::vector<double>& values)
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

    // Y parameters for the grounded Y-Y three-phase transformer
    // We use a 6x6 Y matrix to represent the admittance between the 3 primary windings and 3 secondary windings.

    RCP<const Basic> R_primary = real_double(R[0]);
    RCP<const Basic> X_primary = real_double(X[0]);
    RCP<const Basic> R_secondary = real_double(R[1]);
    RCP<const Basic> X_secondary = real_double(X[1]);
    RCP<const Basic> a_val = real_double(a);  // Turns ratio symbol

    RCP<const Basic> phaseFactor = exp(mul(neg(j), real_double(phaseLag)));  // Phase shift

    // Compute primary (Y side) and secondary (Y side) admittances
    RCP<const Basic> Y_primary = div(real_double(1), add(R_primary, mul(j, X_primary)));
    RCP<const Basic> Y_secondary = div(real_double(1), add(R_secondary, mul(j, X_secondary)));

    // Define the 3x3 submatrices for the primary and secondary Y-side connections

    DenseMatrix Y_primary_matrix(3, 3);
    DenseMatrix Y_secondary_matrix(3, 3);

    // Populate primary side Y matrix (for primary winding)
    Y_primary_matrix.set(0, 0, Y_primary);  // Y11
    Y_primary_matrix.set(0, 1, real_double(0));  // No direct interaction between phases
    Y_primary_matrix.set(0, 2, real_double(0));  // No direct interaction between phases

    Y_primary_matrix.set(1, 0, real_double(0));  // No direct interaction between phases
    Y_primary_matrix.set(1, 1, Y_primary);  // Y22
    Y_primary_matrix.set(1, 2, real_double(0));  // No direct interaction between phases

    Y_primary_matrix.set(2, 0, real_double(0));  // No direct interaction between phases
    Y_primary_matrix.set(2, 1, real_double(0));  // No direct interaction between phases
    Y_primary_matrix.set(2, 2, Y_primary);  // Y33

    // Populate secondary side Y matrix (for secondary winding)
    Y_secondary_matrix.set(0, 0, Y_secondary);  // Y11
    Y_secondary_matrix.set(0, 1, real_double(0));  // No direct interaction between phases
    Y_secondary_matrix.set(0, 2, real_double(0));  // No direct interaction between phases

    Y_secondary_matrix.set(1, 0, real_double(0));  // No direct interaction between phases
    Y_secondary_matrix.set(1, 1, Y_secondary);  // Y22
    Y_secondary_matrix.set(1, 2, real_double(0));  // No direct interaction between phases

    Y_secondary_matrix.set(2, 0, real_double(0));  // No direct interaction between phases
    Y_secondary_matrix.set(2, 1, real_double(0));  // No direct interaction between phases
    Y_secondary_matrix.set(2, 2, Y_secondary);  // Y33

    // Assign submatrices to Y_matrix (6x6 for 3 primary and 3 secondary windings)
    // Top-left 3x3: Y_primary (primary winding)
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            Y_matrix.set(i, j, Y_primary_matrix.get(i, j));  // Y_primary elements
            Y_matrix.set(i + 3, j + 3, Y_secondary_matrix.get(i, j));  // Y_secondary elements
            Y_matrix.set(i, j + 3, mul(real_double(-a), Y_primary_matrix.get(i, j)));  // Y12 interaction (primary to secondary)
            Y_matrix.set(i + 3, j, mul(real_double(-1), Y_primary_matrix.get(i, j)));  // Y21 interaction (secondary to primary)
        }
    }
}

// Destructor
TransformerWyeWye::~TransformerWyeWye() {
    std::cout << "TransformerWyeWye object for " << getElementSymbol() << " destroyed." << std::endl;
}
