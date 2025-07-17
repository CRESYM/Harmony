#ifndef CONSTANTS_H
#define CONSTANTS_H

// SymEngine library for symbolic mathematics
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
#include <symengine/printers.h>  // Correct header for printing

#include<matplot/matplot.h>
using namespace matplot;


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
#include <functional>
#include <iomanip>


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



#endif // CONSTANTS_H
