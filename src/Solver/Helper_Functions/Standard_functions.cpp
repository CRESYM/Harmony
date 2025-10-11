#include "Standard_functions.h"

// Constants
const double mu_0 = 4 * M_PI * 1e-7; // Standard mu_0
const double epsilon_0 = 8.854e-12; // Standard epsilon_0
const double gamma_num = 0.5772156649; // Euler-Mascheroni constant

int sgn(int v) {
	return (v > 0) - (v < 0);
}

// Function to convert a 2D vector of complex numbers to an Eigen matrix
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

// Function to convert an Eigen matrix to a 2D vector of complex numbers
vector<vector<complex<double>>> matrixToVector(const MatrixXcd& mat) {
    if (mat.rows() == 0 || mat.cols() == 0) {
        return {}; // Return an empty vector for an empty matrix
    }

    size_t rows = mat.rows();
    size_t cols = mat.cols();
    vector<vector<complex<double>>> vec(rows, vector<complex<double>>(cols));

    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            vec[i][j] = mat(i, j);
        }
    }

    return vec;
}

// Matrix multiplication helper
vector<vector<complex<double>>> mat_mul(const vector<vector<complex<double>>>& A, const vector<vector<complex<double>>>& B) {
    // Handle empty matrices
    if (A.empty() || A[0].empty() || B.empty() || B[0].empty()) {
        return {};
    }

    size_t a_rows = A.size();
    size_t a_cols = A[0].size();
    size_t b_rows = B.size();
    size_t b_cols = B[0].size();

    // Validate dimensions for multiplication
    if (a_cols != b_rows) {
        throw std::runtime_error("Matrix dimensions are not compatible for multiplication.");
    }

    // Initialize result matrix C with the correct dimensions
    vector<vector<complex<double>>> C(a_rows, vector<complex<double>>(b_cols, 0.0));

    for (size_t i = 0; i < a_rows; ++i) {
        // Ensure all rows in A have the same number of columns
        if (A[i].size() != a_cols) {
            throw std::runtime_error("Matrix A has rows of different lengths.");
        }
        for (size_t j = 0; j < b_cols; ++j) {
            for (size_t k = 0; k < a_cols; ++k) { // a_cols is same as b_rows
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    return C;
};

// Scalar multiplication helper
vector<vector<complex<double>>> mul_scalar(const vector<vector<complex<double>>>& A, const complex<double>& scalar) {
    vector<vector<complex<double>>> C(A.size(), vector<complex<double>>(A[0].size(), 0.0));
    for (size_t i = 0; i < A.size(); ++i) {
        for (size_t j = 0; j < A[0].size(); ++j) {
            C[i][j] = A[i][j] * scalar;
        }
    }
    return C;
};

// Matrix addition helper
vector<vector<complex<double>>> mat_add(const vector<vector<complex<double>>>& A, const vector<vector<complex<double>>>& B) {
    vector<vector<complex<double>>> C(A.size(), vector<complex<double>>(A[0].size(), 0.0));
    for (size_t i = 0; i < A.size(); ++i) {
        for (size_t j = 0; j < A[0].size(); ++j) {
            C[i][j] = A[i][j] + B[i][j];
        }
    }
    return C;
};

// Matrix subtraction helper
vector<vector<complex<double>>> mat_sub(const vector<vector<complex<double>>>& A, const vector<vector<complex<double>>>& B) {
    vector<vector<complex<double>>> C(A.size(), vector<complex<double>>(A[0].size(), 0.0));
    for (size_t i = 0; i < A.size(); ++i) {
        for (size_t j = 0; j < A[0].size(); ++j) {
            C[i][j] = A[i][j] - B[i][j];
        }
    }
    return C;
};

// Matrix transpose helper
vector<vector<complex<double>>> mat_transpose(const vector<vector<complex<double>>>& A) {
    vector<vector<complex<double>>> C(A[0].size(), vector<complex<double>>(A.size(), 0.0));
    for (size_t i = 0; i < A.size(); ++i) {
        for (size_t j = 0; j < A[0].size(); ++j) {
            C[j][i] = A[i][j];
        }
    }
    return C;
};

// Get block from a matrix
extern vector<vector<complex<double>>> get_block(const vector<vector<complex<double>>>& Y, int r_off, int c_off, int r_num, int c_num) {
    vector<vector<complex<double>>> block(r_num, vector<complex<double>>(c_num));
    for (int i = 0; i < r_num; ++i) {
        for (int j = 0; j < c_num; ++j) {
            block[i][j] = Y[r_off + i][c_off + j];
        }
    }
    return block;
};