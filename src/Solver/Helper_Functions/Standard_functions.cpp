/**
 * @file Standard_functions.cpp
 * @brief Implementation of Standard numeric constants and matrix utility functions.
 */
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

namespace {

struct ParkKernels {
    vector<vector<complex<double>>> a;
    vector<vector<complex<double>>> a_tran;
    vector<vector<complex<double>>> a_conj;
    vector<vector<complex<double>>> a_conj_tran;
};

ParkKernels makeParkKernels()
{
    // Fourier kernel of Park (4.54): φ = −2π/3 so a = [1, α², α] with α = e^{j 2π/3}.
    const complex<double> ang = std::exp(complex<double>(0, -2.0 * M_PI / 3.0));
    const complex<double> imag_unit(0, 1);
    ParkKernels k;
    k.a.assign(3, vector<complex<double>>(3));
    k.a[0] = { 1.0, ang, ang * ang };
    k.a[1] = { imag_unit, imag_unit * ang, imag_unit * ang * ang };
    k.a[2] = { 0.0, 0.0, 0.0 };
    k.a_tran = mat_transpose(k.a);
    k.a_conj.assign(3, vector<complex<double>>(3));
    k.a_conj[0] = { 1.0, conj(ang), conj(ang * ang) };
    k.a_conj[1] = { -imag_unit, -imag_unit * conj(ang), -imag_unit * conj(ang * ang) };
    k.a_conj[2] = { 0.0, 0.0, 0.0 };
    k.a_conj_tran = mat_transpose(k.a_conj);
    return k;
}

bool parkAbcSquare(const vector<vector<complex<double>>>& Y, int& nabc)
{
    if (Y.empty() || Y[0].empty())
        return false;
    nabc = static_cast<int>(Y.size());
    if (nabc % 3 != 0 || static_cast<int>(Y[0].size()) != nabc)
        return false;
    return true;
}

enum class ParkMapKind { A0, Cminus, Cplus };

vector<vector<complex<double>>> apply_park_map(
    const vector<vector<complex<double>>>& Y_a,
    const vector<vector<complex<double>>>& Y_b,
    ParkMapKind kind)
{
    int nabc = 0;
    if (!parkAbcSquare(Y_a, nabc))
        return Y_a;
    if (kind == ParkMapKind::A0) {
        int nabc_b = 0;
        if (!parkAbcSquare(Y_b, nabc_b) || nabc_b != nabc)
            return Y_a;
    }
    const int nports = nabc / 3;
    const ParkKernels k = makeParkKernels();
    vector<vector<complex<double>>> Y_dq(
        static_cast<size_t>(2 * nports),
        vector<complex<double>>(static_cast<size_t>(2 * nports), { 0.0, 0.0 }));

    for (int pi = 0; pi < nports; ++pi) {
        for (int pj = 0; pj < nports; ++pj) {
            auto Ya = get_block(Y_a, 3 * pi, 3 * pj, 3, 3);
            vector<vector<complex<double>>> blk;
            if (kind == ParkMapKind::A0) {
                auto Yb = get_block(Y_b, 3 * pi, 3 * pj, 3, 3);
                blk = mul_scalar(
                    mat_add(mat_mul(mat_mul(k.a, Ya), k.a_conj_tran),
                            mat_mul(mat_mul(k.a_conj, Yb), k.a_tran)),
                    1.0 / 6.0);
            }
            else if (kind == ParkMapKind::Cminus) {
                blk = mul_scalar(mat_mul(mat_mul(k.a, Ya), k.a_tran), 1.0 / 6.0);
            }
            else {
                blk = mul_scalar(mat_mul(mat_mul(k.a_conj, Ya), k.a_conj_tran), 1.0 / 6.0);
            }
            for (int i = 0; i < 2; ++i)
                for (int j = 0; j < 2; ++j)
                    Y_dq[static_cast<size_t>(2 * pi + i)][static_cast<size_t>(2 * pj + j)] = blk[i][j];
        }
    }
    return Y_dq;
}

MatrixXcd invertParkBlock(const MatrixXcd& A)
{
    if (A.size() == 0)
        return A;
    MatrixXcd Ause = A;
    if (!Ause.allFinite())
        Ause = MatrixXcd::Identity(A.rows(), A.cols()) * 1e-12;
    Eigen::FullPivLU<MatrixXcd> lu(Ause);
    if (!lu.isInvertible()) {
        Ause += MatrixXcd::Identity(Ause.rows(), Ause.cols()) * 1e-12;
        lu.compute(Ause);
    }
    return lu.solve(MatrixXcd::Identity(Ause.rows(), Ause.cols()));
}

} // namespace

vector<vector<complex<double>>> apply_park_A0(
    const vector<vector<complex<double>>>& Y_minus,
    const vector<vector<complex<double>>>& Y_plus)
{
    return apply_park_map(Y_minus, Y_plus, ParkMapKind::A0);
}

vector<vector<complex<double>>> apply_park_C_minus(
    const vector<vector<complex<double>>>& Y_minus)
{
    return apply_park_map(Y_minus, Y_minus, ParkMapKind::Cminus);
}

vector<vector<complex<double>>> apply_park_C_plus(
    const vector<vector<complex<double>>>& Y_plus)
{
    return apply_park_map(Y_plus, Y_plus, ParkMapKind::Cplus);
}

vector<vector<complex<double>>> apply_park_Yeff(
    const vector<vector<complex<double>>>& Y_m3,
    const vector<vector<complex<double>>>& Y_m1,
    const vector<vector<complex<double>>>& Y_p1,
    const vector<vector<complex<double>>>& Y_p3)
{
    const auto A0_w = apply_park_A0(Y_m1, Y_p1);
    const auto A0_m2 = apply_park_A0(Y_m3, Y_m1);
    const auto A0_p2 = apply_park_A0(Y_p1, Y_p3);
    const auto Cm_w = apply_park_C_minus(Y_m1);
    const auto Cp_w = apply_park_C_plus(Y_p1);
    const auto Cp_m2 = apply_park_C_plus(Y_m1);
    const auto Cm_p2 = apply_park_C_minus(Y_p1);

    const MatrixXcd A0 = vectorToMatrix(A0_w);
    const MatrixXcd corr_m = vectorToMatrix(Cm_w) * invertParkBlock(vectorToMatrix(A0_m2))
        * vectorToMatrix(Cp_m2);
    const MatrixXcd corr_p = vectorToMatrix(Cp_w) * invertParkBlock(vectorToMatrix(A0_p2))
        * vectorToMatrix(Cm_p2);
    const MatrixXcd Yeff = A0 - corr_m - corr_p;
    return matrixToVector(Yeff);
}


