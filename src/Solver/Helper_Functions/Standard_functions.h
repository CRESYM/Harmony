#ifndef _STANDARD_FUNCTIONS_H_
#define _STANDARD_FUNCTIONS_H_

/**
 * @file Standard_functions.h
 * @brief Standard numeric constants and matrix utility functions.
 *
 * Provides physical constants, sign helpers, vector/matrix conversions,
 * and basic complex-matrix arithmetic used throughout the solver.
 */

#include "../../Constants.h"

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
 * @brief Stacks four input matrices into a single block column.
 * @param u1 First input block.
 * @param u2 Second input block.
 * @param u3 Third input block.
 * @param u4 Fourth input block.
 * @return Vertically stacked complex matrix.
 */
extern MatrixXcd stack_u_4x_3xN(const MatrixXcd& u1, const MatrixXcd& u2, const MatrixXcd& u3, const MatrixXcd& u4);

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
 * @brief Element-wise subtraction of two complex matrices.
 * @param A Minuend.
 * @param B Subtrahend.
 * @return A - B.
 */
extern vector<vector<complex<double>>> mat_sub(const vector<vector<complex<double>>>& A, const vector<vector<complex<double>>>& B);

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


#endif // _STANDARD_FUNCTIONS_H_
