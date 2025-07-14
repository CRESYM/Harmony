#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <symengine/basic.h>
#include <symengine/symbol.h>
#include <symengine/add.h>
#include <symengine/mul.h>
#include <symengine/pow.h>
#include <symengine/real_double.h>
#include <symengine/eval_double.h>
#include <symengine/eval.h>
#include <symengine/functions.h>
#include <symengine/complex.h>
#include <symengine/complex_double.h>
#include <symengine/expression.h>
#include <symengine/symengine_config.h>
#include <symengine/matrix.h> // Ensure you include the necessary SymEngine headers
#include <symengine/subs.h>
#include <symengine/simplify.h>
#include <symengine/matrices/identity_matrix.h> // identity matrix
#include <symengine/matrices/matrix_mul.h> // matrix multiplications
#include <symengine/matrices/matrix_add.h>
#include <symengine/matrix_expressions.h>
#include <symengine/matrices/immutable_dense_matrix.h>
#include <symengine/polys/basic_conversions.h>
// Eigen library for linear algebra
#include <Eigen/Dense>
#include <unsupported/Eigen/MatrixFunctions>

#define _USE_MATH_DEFINES
#include <math.h> 
#include <string>
#include <iostream>
#include <fstream>
#include <vector> // For handling matrices in multi-phase systems
#include <complex>
#include <stdexcept>
#include <tuple>
#include <variant>
#include <algorithm>
#include <sstream>
#include <any>
#include <map>
#include <cctype> // Include the header for std::tolower
#include <memory>  
#include <unordered_map>
#include <cmath>

using SymEngine::RCP;
using SymEngine::Basic;
using SymEngine::DenseMatrix;
using SymEngine::integer;
using SymEngine::mul;
using SymEngine::symbol;
using SymEngine::one;
using namespace std;
using namespace std::complex_literals;
using namespace SymEngine; // Use the SymEngine namespace
using namespace Eigen;


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

#endif // CONSTANTS_H
