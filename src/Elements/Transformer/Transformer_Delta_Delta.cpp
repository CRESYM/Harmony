#include "Transformer_Delta_Delta.h"

// Constructor
TransformerDeltaDelta::TransformerDeltaDelta(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values)
    : Transformer_classic(symbol, location, pins, values) {   

    if (pins != 3) {
        throw std::invalid_argument("Invalid number of pins. It must be 3!");
        exit(1);
    }

    auto M = DenseMatrix(3, 3, { integer(1), zero, integer(-1), integer(-1), integer(1), zero, zero, integer(-1), integer(1) });
    mul_dense_scalar(M, real_double(1.0 / sqrt(3)), M);

    // Build the matrices for multiplication
    auto N1 = createZeroMatrix(6, 6);
    auto N2 = createZeroMatrix(6, 6);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            N1.set(i, j, M.get(i,j));
            N1.set(3 + i, 3 + j, M.get(i, j));
        }
        N2.set(i, i, real_double(1.0 / sqrt(3)));
        N2.set(3 + i, 3 + i, real_double(1.0 / sqrt(3)));
    }
    // mul_dense_scalar(N2, real_double(1.0 / sqrt(3)), N2);

    mul_dense_dense(N1, Y_matrix, Y_matrix);
    mul_dense_dense(Y_matrix, N2, Y_matrix);
}

// Destructor
TransformerDeltaDelta::~TransformerDeltaDelta() {
    std::cout << "Transformer Delta-Delta object for " << getElementSymbol() << " destroyed." << std::endl;
}
