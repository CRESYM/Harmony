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

    // Y parameters matrix for the three-phase delta-delta transformer
    // We use a 6x6 Y matrix to represent the admittance between the 3 primary and 3 secondary windings.

    RCP<const Basic> R_p = real_double(R[0]);
    RCP<const Basic> X_p = real_double(X[0]);
    RCP<const Basic> R_s = real_double(R[1]);
    RCP<const Basic> X_s = real_double(X[1]);
    RCP<const Basic> a_val = real_double(a); // Turns ratio symbol

    RCP<const Basic> phaseFactor = exp(mul(neg(j), real_double(phaseLag))); // Phase shift

    // Compute primary and secondary admittances
    RCP<const Basic> Y_p = div(real_double(1), add(R_p, mul(j, X_p)));
    RCP<const Basic> Y_s = div(real_double(1), add(R_s, mul(j, X_s)));

    // Populate the Y matrices for primary and secondary sides (delta configuration)
    DenseMatrix Y_delta_p(3, 3);
    DenseMatrix Y_delta_s(3, 3);

    Y_delta_p.set(0, 0, Y_p);
    Y_delta_p.set(0, 1, mul(real_double(-1), Y_p));
    Y_delta_p.set(0, 2, real_double(0));

    Y_delta_p.set(1, 0, mul(real_double(-1), Y_p));
    Y_delta_p.set(1, 1, Y_p);
    Y_delta_p.set(1, 2, mul(real_double(-1), Y_p));

    Y_delta_p.set(2, 0, real_double(0));
    Y_delta_p.set(2, 1, mul(real_double(-1), Y_p));
    Y_delta_p.set(2, 2, Y_p);

    Y_delta_s.set(0, 0, Y_s);
    Y_delta_s.set(0, 1, mul(real_double(-1), Y_s));
    Y_delta_s.set(0, 2, real_double(0));

    Y_delta_s.set(1, 0, mul(real_double(-1), Y_s));
    Y_delta_s.set(1, 1, Y_s);
    Y_delta_s.set(1, 2, mul(real_double(-1), Y_s));

    Y_delta_s.set(2, 0, real_double(0));
    Y_delta_s.set(2, 1, mul(real_double(-1), Y_s));
    Y_delta_s.set(2, 2, Y_s);

    // Assign submatrices to the Y_matrix (6x6)
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            Y_matrix.set(i, j, Y_delta_p.get(i, j));  // Primary winding
            Y_matrix.set(i + 3, j + 3, Y_delta_s.get(i, j));  // Secondary winding

            // Primary to secondary interaction with phase shift
            Y_matrix.set(i, j + 3, mul(mul(real_double(-a), phaseFactor), Y_delta_p.get(i, j)));

            // Secondary to primary interaction with phase shift
            Y_matrix.set(i + 3, j, mul(mul(real_double(-1), phaseFactor), Y_delta_p.get(i, j)));
        }
    }
}

// Destructor
TransformerDeltaDelta::~TransformerDeltaDelta() {
    std::cout << "TransformerDeltaDelta object for " << getElementSymbol() << " destroyed." << std::endl;
}
