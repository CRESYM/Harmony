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
#include <SymEngine/Matrix.h> // Ensure you include the necessary SymEngine headers
#include <SymEngine/subs.h>
#include <symengine/simplify.h>
#include <symengine/matrices/identity_matrix.h> // identity matrix
#include <symengine/matrices/matrix_mul.h> // matrix multiplications
#include <symengine/matrices/matrix_add.h>
#include <Symengine/matrix_expressions.h>
#include <symengine/matrices/immutable_dense_matrix.h>
#include <symengine/polys/basic_conversions.h>

#include <intrin.h>

#include <cmath>  // Include cmath for std::acos
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

// Eigen library for linear algebra
#include <Eigen/Dense>
#include <unsupported/Eigen/MatrixFunctions>

using namespace std;
using namespace SymEngine; // Use the SymEngine namespace
using namespace Eigen;


// Define a constant for π (pi) using SymEngine
extern const RCP<const Basic> PI;

// Define a constant for π (pi) using standard C++
extern const double M_PI ;

extern const double mu_0; // vacuum permitivity
extern const double epsilon_0; // vacuum permeability
extern const double gamma_num;

// Define imaginary unit and symbol for angular frequency
extern RCP<const Basic> j;  // Imaginary unit
extern RCP<const Basic> omega;
extern RCP<const Basic> s; // s = j * omega

extern DenseMatrix createZeroMatrix(int size1, int size2);

extern RCP<const Basic> substitute_symbol(const RCP<const Basic>& expr, const std::string& symbol_name, double value);

extern 	double eval_basic(const RCP<const Basic>& expr);

extern MatrixXd kron_reduction(MatrixXd, vector<int>);
extern DenseMatrix kron_reduction(DenseMatrix, vector<int>);

#endif // CONSTANTS_H
