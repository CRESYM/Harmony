#include "Standard_functions.h"

// Constants
const double mu_0 = 4 * M_PI * 1e-7;      ///< Magnetic constant (permeability of free space).
const double epsilon_0 = 8.854e-12;   ///< Electric constant (permittivity of free space).
const double gamma_num = 0.5772156649; ///< Euler-Mascheroni constant.

/**
 * @brief Computes the sign of an integer.
 * @param v The integer value.
 * @return 1 if v > 0, -1 if v < 0, and 0 if v == 0.
 */
int sgn(int v) {
	return (v > 0) - (v < 0);
}

/**
 * @brief Converts a 2D vector of complex numbers to an Eigen matrix.
 * @param vec The input 2D vector (matrix).
 * @return An Eigen::MatrixXcd representation of the input vector.
 * @throws std::runtime_error if the inner vectors have inconsistent lengths.
 */
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

/**
 * @brief Converts a 2D vector of complex numbers to an Eigen matrix.
 * @param vec The input 2D vector (matrix).
 * @return An Eigen::MatrixXcd representation of the input vector.
 * @throws std::runtime_error if the inner vectors have inconsistent lengths.
 */
MatrixXd vectorToMatrix(const vector<vector<double>>& vec) {
    if (vec.empty() || vec[0].empty()) {
        return MatrixXd(); // Return empty matrix if input is empty
    }

    size_t rows = vec.size();
    size_t cols = vec[0].size();
    MatrixXd mat(rows, cols);

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

/**
 * @brief Converts an Eigen matrix of complex numbers to a 2D vector.
 * @param mat The input Eigen::MatrixXcd.
 * @return A 2D vector representation of the input matrix.
 */
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

vector<vector<double>> matrixToVector(const MatrixXd& mat) {
    if (mat.rows() == 0 || mat.cols() == 0) {
        return {}; // Return an empty vector for an empty matrix
    }
    size_t rows = mat.rows();
    size_t cols = mat.cols();
    vector<vector<double>> vec(rows, vector<double>(cols));
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            vec[i][j] = mat(i, j);
        }
    }
    return vec;
}

MatrixXcd stack_u_4x_3xN(const MatrixXcd& u1, const MatrixXcd& u2, const MatrixXcd& u3, const MatrixXcd& u4)
{
    if (u1.rows() != 3 || u2.rows() != 3 || u3.rows() != 3 || u4.rows() != 3) {
        throw std::invalid_argument("All MMC input groups must have 3 rows.");
    }

    const Eigen::Index nCols = u1.cols();

    if (u2.cols() != nCols || u3.cols() != nCols || u4.cols() != nCols) {
        throw std::invalid_argument("All MMC input groups must have the same number of columns.");
    }

    MatrixXcd u(12, nCols);
    u.block(0, 0, 3, nCols) = u1;
    u.block(3, 0, 3, nCols) = u2;
    u.block(6, 0, 3, nCols) = u3;
    u.block(9, 0, 3, nCols) = u4;

    return u;
}

/**
 * @brief Multiplies two matrices represented as 2D vectors.
 * @param A The left-hand side matrix.
 * @param B The right-hand side matrix.
 * @return The resulting matrix C = A * B.
 * @throws std::runtime_error if matrix dimensions are incompatible for multiplication.
 */
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

/**
 * @brief Multiplies a matrix by a scalar value.
 * @param A The matrix to be multiplied.
 * @param scalar The complex scalar value.
 * @return The resulting matrix C = A * scalar.
 */
vector<vector<complex<double>>> mul_scalar(const vector<vector<complex<double>>>& A, const complex<double>& scalar) {
    vector<vector<complex<double>>> C(A.size(), vector<complex<double>>(A[0].size(), 0.0));
    for (size_t i = 0; i < A.size(); ++i) {
        for (size_t j = 0; j < A[0].size(); ++j) {
            C[i][j] = A[i][j] * scalar;
        }
    }
    return C;
};

/**
 * @brief Adds two matrices of the same dimensions.
 * @param A The first matrix.
 * @param B The second matrix.
 * @return The resulting matrix C = A + B.
 */
vector<vector<complex<double>>> mat_add(const vector<vector<complex<double>>>& A, const vector<vector<complex<double>>>& B) {
    vector<vector<complex<double>>> C(A.size(), vector<complex<double>>(A[0].size(), 0.0));
    for (size_t i = 0; i < A.size(); ++i) {
        for (size_t j = 0; j < A[0].size(); ++j) {
            C[i][j] = A[i][j] + B[i][j];
        }
    }
    return C;
};

/**
 * @brief Subtracts one matrix from another.
 * @param A The matrix from which to subtract.
 * @param B The matrix to subtract.
 * @return The resulting matrix C = A - B.
 */
vector<vector<complex<double>>> mat_sub(const vector<vector<complex<double>>>& A, const vector<vector<complex<double>>>& B) {
    vector<vector<complex<double>>> C(A.size(), vector<complex<double>>(A[0].size(), 0.0));
    for (size_t i = 0; i < A.size(); ++i) {
        for (size_t j = 0; j < A[0].size(); ++j) {
            C[i][j] = A[i][j] - B[i][j];
        }
    }
    return C;
};

/**
 * @brief Transposes a matrix.
 * @param A The matrix to transpose.
 * @return The transposed matrix.
 */
vector<vector<complex<double>>> mat_transpose(const vector<vector<complex<double>>>& A) {
    vector<vector<complex<double>>> C(A[0].size(), vector<complex<double>>(A.size(), 0.0));
    for (size_t i = 0; i < A.size(); ++i) {
        for (size_t j = 0; j < A[0].size(); ++j) {
            C[j][i] = A[i][j];
        }
    }
    return C;
};

/**
 * @brief Extracts a block (submatrix) from a given matrix.
 * @param Y The source matrix.
 * @param r_off The starting row index of the block.
 * @param c_off The starting column index of the block.
 * @param r_num The number of rows in the block.
 * @param c_num The number of columns in the block.
 * @return A new matrix containing the specified block.
 */
extern vector<vector<complex<double>>> get_block(const vector<vector<complex<double>>>& Y, int r_off, int c_off, int r_num, int c_num) {
    vector<vector<complex<double>>> block(r_num, vector<complex<double>>(c_num));
    for (int i = 0; i < r_num; ++i) {
        for (int j = 0; j < c_num; ++j) {
            block[i][j] = Y[r_off + i][c_off + j];
        }
    }
    return block;
};