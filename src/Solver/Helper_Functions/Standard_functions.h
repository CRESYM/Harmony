#ifndef _STANDARD_FUNCTIONS_H_
#define _STANDARD_FUNCTIONS_H_

/**
 * @file Standard_functions.h
 * @brief Standard numeric constants and matrix utility functions.
 *
 * Provides physical constants, sign helpers, vector/matrix conversions,
 * and basic complex-matrix arithmetic used throughout the solver.
 */

#include "core/Constants.h"

/** @brief Vacuum permeability (H/m). */
extern const double mu_0;
/** @brief Vacuum permittivity (F/m). */
extern const double epsilon_0;
/** @brief Euler–Mascheroni constant. */
extern const double gamma_num;

/**
 * @brief Returns the sign of an integer.
 * @param v Input value.
 * @return -1, 0, or +1.
 */
extern int sgn(int v);

/**
 * @brief Converts a nested complex vector to an Eigen MatrixXcd.
 * @param vec 2-D vector of complex values.
 * @return Equivalent dense complex matrix.
 */
extern MatrixXcd vectorToMatrix(const vector<vector<complex<double>>>& vec);

/**
 * @brief Converts a nested real vector to an Eigen MatrixXd.
 * @param vec 2-D vector of double values.
 * @return Equivalent dense real matrix.
 */
extern MatrixXd vectorToMatrix(const vector<vector<double>>& vec);

/**
 * @brief Converts an Eigen MatrixXcd to a nested complex vector.
 * @param mat Input complex matrix.
 * @return 2-D vector representation.
 */
extern vector<vector<complex<double>>> matrixToVector(const MatrixXcd& mat);

/**
 * @brief Converts an Eigen MatrixXd to a nested real vector.
 * @param mat Input real matrix.
 * @return 2-D vector representation.
 */
extern vector<vector<double>> matrixToVector(const MatrixXd& mat);

/**
 * @brief Complex matrix multiplication using nested vectors.
 * @param A Left operand.
 * @param B Right operand.
 * @return Product A * B.
 */
extern vector<vector<complex<double>>> mat_mul(const vector<vector<complex<double>>>& A, const vector<vector<complex<double>>>& B);

/**
 * @brief Scales every entry of a complex matrix by a scalar.
 * @param A Input matrix.
 * @param scalar Complex scale factor.
 * @return Scaled matrix.
 */
extern vector<vector<complex<double>>> mul_scalar(const vector<vector<complex<double>>>& A, const complex<double>& scalar);

/**
 * @brief Element-wise addition of two complex matrices.
 * @param A First operand.
 * @param B Second operand.
 * @return A + B.
 */
extern vector<vector<complex<double>>> mat_add(const vector<vector<complex<double>>>& A, const vector<vector<complex<double>>>& B);

/**
 * @brief Transposes a complex matrix stored as nested vectors.
 * @param A Input matrix.
 * @return Transpose of A.
 */
extern vector<vector<complex<double>>> mat_transpose(const vector<vector<complex<double>>>& A);

/**
 * @brief Extracts a rectangular sub-block from a complex matrix.
 * @param Y Source matrix.
 * @param r_off Row offset (0-based).
 * @param c_off Column offset (0-based).
 * @param r_num Number of rows to extract.
 * @param c_num Number of columns to extract.
 * @return Sub-matrix of size r_num × c_num.
 */
extern vector<vector<complex<double>>> get_block(const vector<vector<complex<double>>>& Y, int r_off, int c_off, int r_num, int c_num);

/**
 * @brief Park A0 map of an n-port abc admittance (paper eq. 9).
 *
 * Y_minus is Yabc(j(ω−ωo)), Y_plus is Yabc(j(ω+ωo)). Each port is 3×3 abc;
 * the result is 2×2 dq per port (size 2n × 2n). Coupling blocks C± are not
 * included (Tier 1). Matrices whose size is not a multiple of 3 are returned
 * unchanged (Y_minus).
 */
extern vector<vector<complex<double>>> apply_park_A0(
    const vector<vector<complex<double>>>& Y_minus,
    const vector<vector<complex<double>>>& Y_plus);

/**
 * @brief Coupling C− of an n-port abc admittance (paper eq. 10).
 *
 * C−(ω) = (1/6) a Yabc(j(ω−ωo)) aᵀ. Size 2n × 2n.
 */
extern vector<vector<complex<double>>> apply_park_C_minus(
    const vector<vector<complex<double>>>& Y_minus);

/**
 * @brief Coupling C+ of an n-port abc admittance (paper eq. 11).
 *
 * C+(ω) = (1/6) a* Yabc(j(ω+ωo)) aᴴ. Size 2n × 2n.
 */
extern vector<vector<complex<double>>> apply_park_C_plus(
    const vector<vector<complex<double>>>& Y_plus);

/**
 * @brief Effective dq admittance Yeff (paper eq. 18) for an n-port abc Y.
 *
 * Y_m3, Y_m1, Y_p1, Y_p3 are Yabc at ω−3ωo, ω−ωo, ω+ωo, ω+3ωo.
 * Yeff = A0(ω) − C−(ω) A0(ω−2ωo)⁻¹ C+(ω−2ωo) − C+(ω) A0(ω+2ωo)⁻¹ C−(ω+2ωo).
 */
extern vector<vector<complex<double>>> apply_park_Yeff(
    const vector<vector<complex<double>>>& Y_m3,
    const vector<vector<complex<double>>>& Y_m1,
    const vector<vector<complex<double>>>& Y_p1,
    const vector<vector<complex<double>>>& Y_p3);

#endif // _STANDARD_FUNCTIONS_H_
