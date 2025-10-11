#include "Standard_functions.h"

// Constants
const double mu_0 = 4 * M_PI * 1e-7; // Standard mu_0
const double epsilon_0 = 8.854e-12; // Standard epsilon_0
const double gamma_num = 0.5772156649; // Euler-Mascheroni constant

int sgn(int v) {
	return (v > 0) - (v < 0);
}

MatrixXcd vectorToMatrix(const vector<vector<complex<double>>>& vec) {
    if (vec.empty() || vec[0].empty()) {
        return MatrixXcd(); // Return empty matrix if input is empty
    }

    size_t rows = vec.size();
    size_t cols = vec[0].size();
    MatrixXcd mat(rows, cols);

    for (size_t i = 0; i < rows; ++i) {
        if (vec[i].size() != cols) {
            throw std::runtime_error("All inner vectors must have the same length");
        }
        for (size_t j = 0; j < cols; ++j) {
            mat(i, j) = vec[i][j];
        }
    }

    return mat;
}