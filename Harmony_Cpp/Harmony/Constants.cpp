#include "Constants.h"

// Define the constants declared in the header
const RCP<const Basic> PI = real_double(3.141592653589793); // SymEngine π
const double M_PI = std::acos(-1.0);                        // Standard C++ π
// Constants
const double mu_0 = 4 * M_PI * 1e-7; // Standard mu_0
const double epsilon = 8.85e-12; // Standard epsilon_0

RCP<const Basic> j = I;  // Imaginary unit
RCP<const Basic> omega = symbol("w");

// Static helper function to create a zero matrix
DenseMatrix createZeroMatrix(int size1, int size2) {
    DenseMatrix zeroMatrix(size1, size2);
    for (int i = 0; i < size1; ++i) {
        for (int j = 0; j < size2; ++j) {
            zeroMatrix.set(i, j, zero); // Use SymEngine's symbolic `zero`
        }
    }
    return zeroMatrix;
}
