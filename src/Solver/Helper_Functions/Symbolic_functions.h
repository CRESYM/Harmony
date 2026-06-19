#ifndef _SYMBOLIC_FUNCTIONS_H_
#define _SYMBOLIC_FUNCTIONS_H_

/**
 * @file Symbolic_functions.h
 * @brief SymEngine-based symbolic algebra utilities for network analysis.
 *
 * Provides symbolic constants, matrix creation/conversion, parameter
 * substitution, Kron reduction, and basic symbolic arithmetic used when
 * assembling MNA matrices and transfer functions.
 */

#include "../../Constants.h"

/** @brief Symbolic representation of π. */
extern const RCP<const Basic> PI;

/** @brief Vacuum permeability (H/m). */
extern const double mu_0;
/** @brief Vacuum permittivity (F/m). */
extern const double epsilon_0;
/** @brief Euler–Mascheroni constant. */
extern const double gamma_num;

/** @brief Imaginary unit j. */
extern RCP<const Basic> j;
/** @brief Angular frequency symbol ω. */
extern RCP<const Basic> omega;
/** @brief Complex frequency symbol s = jω. */
extern RCP<const Basic> s;

/**
 * @brief Creates a zero-filled SymEngine dense matrix.
 * @param size1 Number of rows.
 * @param size2 Number of columns.
 * @return Zero matrix of the requested dimensions.
 */
extern DenseMatrix createZeroMatrix(int size1, int size2);

/**
 * @brief Sets every entry of a SymEngine dense matrix to zero in place.
 * @param mat Matrix to zero.
 */
extern void fillWithZero(DenseMatrix& mat);

/**
 * @brief Converts an Eigen complex matrix to a SymEngine DenseMatrix.
 * @param eigenMat Input Eigen MatrixXcd.
 * @return Equivalent SymEngine dense matrix.
 */
extern DenseMatrix eigenToSymEngineDenseMatrix(const MatrixXcd& eigenMat);

/**
 * @brief Substitutes a named symbol with a real value in an expression.
 * @param expr Symbolic expression.
 * @param symbol_name Name of the symbol to replace.
 * @param value Real substitution value.
 * @return Complex numeric result after substitution.
 */
extern complex<double> substitute_symbol(const RCP<const Basic>& expr, const std::string& symbol_name, double value);

/**
 * @brief Substitutes a symbolic variable with a real value in an expression.
 * @param expr Symbolic expression.
 * @param symbol_name Symbol to replace.
 * @param value Real substitution value.
 * @return Complex numeric result after substitution.
 */
extern complex<double> substitute_symbol(const RCP<const Basic>& expr, RCP<const Basic> symbol_name, double value);

/**
 * @brief Substitutes a named symbol with a complex value in an expression.
 * @param expr Symbolic expression.
 * @param symbol_name Name of the symbol to replace.
 * @param value Complex substitution value.
 * @return Complex numeric result after substitution.
 */
extern complex<double> substitute_symbol(const RCP<const Basic>& expr, const std::string& symbol_name, complex<double> value);

/**
 * @brief Substitutes a symbolic variable with a complex value in an expression.
 * @param expr Symbolic expression.
 * @param symbol_name Symbol to replace.
 * @param value Complex substitution value.
 * @return Complex numeric result after substitution.
 */
extern complex<double> substitute_symbol(const RCP<const Basic>& expr, RCP<const Basic> symbol_name, complex<double> value);

/**
 * @brief Substitutes a named symbol with a real value in every matrix entry.
 * @param mat Symbolic dense matrix.
 * @param symbol_name Name of the symbol to replace.
 * @param value Real substitution value.
 * @return Numeric complex matrix after substitution.
 */
extern MatrixXcd substitute_symbol(DenseMatrix, const string&, double);

/**
 * @brief Substitutes a named symbol with a complex value in every matrix entry.
 * @param mat Symbolic dense matrix.
 * @param symbol_name Name of the symbol to replace.
 * @param value Complex substitution value.
 * @return Numeric complex matrix after substitution.
 */
extern MatrixXcd substitute_symbol(DenseMatrix, const string&, complex<double>);

/**
 * @brief Substitutes a symbolic variable with a real value in every matrix entry.
 * @param mat Symbolic dense matrix.
 * @param symbol_name Symbol to replace.
 * @param value Real substitution value.
 * @return Numeric complex matrix after substitution.
 */
extern MatrixXcd substitute_symbol(DenseMatrix, RCP<const Basic>, double);

/**
 * @brief Substitutes a symbolic variable with a complex value in every matrix entry.
 * @param mat Symbolic dense matrix.
 * @param symbol_name Symbol to replace.
 * @param value Complex substitution value.
 * @return Numeric complex matrix after substitution.
 */
extern MatrixXcd substitute_symbol(DenseMatrix, RCP<const Basic>, complex<double>);

/**
 * @brief Substitutes multiple symbols with the same real value in an expression.
 * @param expr Symbolic expression.
 * @param symbols List of symbols to replace.
 * @param value Common real substitution value.
 * @return Simplified symbolic expression after substitution.
 */
extern RCP<const Basic> substitute_symbols(const RCP<const Basic>& expr, const std::vector<RCP<const Basic>>& symbols, double value);

/**
 * @brief Evaluates a symbolic expression to a real scalar.
 * @param expr Symbolic expression (must be real-valued after substitution).
 * @return Numeric double result.
 */
extern 	double eval_basic(const RCP<const Basic>& expr);

/**
 * @brief Kron-reduces a real matrix by eliminating specified nodes.
 * @param matrix Input admittance/impedance matrix.
 * @param nodes Indices of nodes to eliminate.
 * @return Reduced matrix.
 */
extern MatrixXd kron_reduction(MatrixXd, vector<int>);

/**
 * @brief Kron-reduces a symbolic dense matrix by eliminating specified nodes.
 * @param matrix Input symbolic matrix.
 * @param nodes Indices of nodes to eliminate.
 * @return Symbolically reduced matrix.
 */
extern DenseMatrix kron_reduction(DenseMatrix, vector<int>);

/**
 * @brief Takes the element-wise absolute value of a complex sparse matrix.
 * @param matrix Input complex sparse matrix.
 * @return Real sparse matrix of magnitudes.
 */
extern Eigen::SparseMatrix<double> absoluteSparseMatrix(const Eigen::SparseMatrix<std::complex<double>>& matrix);

/**
 * @brief Symbolic multiplicative inverse (1/x).
 * @param val Symbolic value.
 * @return Symbolic inverse expression.
 */
extern RCP<const Basic> inv(const RCP<const Basic>& val);

/**
 * @brief Symbolic addition a + b.
 * @param a First operand.
 * @param b Second operand.
 * @return Simplified sum.
 */
extern RCP<const Basic> addSym(const RCP<const Basic>& a, const RCP<const Basic>& b);

/**
 * @brief Symbolic subtraction a - b.
 * @param a Minuend.
 * @param b Subtrahend.
 * @return Simplified difference.
 */
extern RCP<const Basic> subSym(const RCP<const Basic>& a, const RCP<const Basic>& b);


#endif
