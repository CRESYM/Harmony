#ifndef _SYMBOLIC_FUNCTIONS_H_
#define _SYMBOLIC_FUNCTIONS_H_

#include "../../Constants.h"


// Define a constant for π (pi) using SymEngine
extern const RCP<const Basic> PI;

extern const double mu_0; // vacuum permitivity
extern const double epsilon_0; // vacuum permeability
extern const double gamma_num;

// Define imaginary unit and symbol for angular frequency
extern RCP<const Basic> j;  // Imaginary unit
extern RCP<const Basic> omega;
extern RCP<const Basic> s; // s = j * omega

extern DenseMatrix createZeroMatrix(int size1, int size2);
extern void fillWithZero(DenseMatrix& mat);

extern complex<double> substitute_symbol(const RCP<const Basic>& expr, const std::string& symbol_name, double value);
extern complex<double> substitute_symbol(const RCP<const Basic>& expr, RCP<const Basic> symbol_name, double value);
extern complex<double> substitute_symbol(const RCP<const Basic>& expr, const std::string& symbol_name, complex<double> value);
extern complex<double> substitute_symbol(const RCP<const Basic>& expr, RCP<const Basic> symbol_name, complex<double> value);

extern MatrixXcd substitute_symbol(DenseMatrix, const string&, double);
extern MatrixXcd substitute_symbol(DenseMatrix, const string&, complex<double>);
extern MatrixXcd substitute_symbol(DenseMatrix, RCP<const Basic>, double);
extern MatrixXcd substitute_symbol(DenseMatrix, RCP<const Basic>, complex<double>);

extern RCP<const Basic> substitute_symbols(const RCP<const Basic>& expr, const std::vector<RCP<const Basic>>& symbols, double value);

extern 	double eval_basic(const RCP<const Basic>& expr);

extern MatrixXd kron_reduction(MatrixXd, vector<int>);
extern DenseMatrix kron_reduction(DenseMatrix, vector<int>);

// Helper symbolic operations
extern RCP<const Basic> inv(const RCP<const Basic>& val);
extern RCP<const Basic> addSym(const RCP<const Basic>& a, const RCP<const Basic>& b);
extern RCP<const Basic> subSym(const RCP<const Basic>& a, const RCP<const Basic>& b);


#endif