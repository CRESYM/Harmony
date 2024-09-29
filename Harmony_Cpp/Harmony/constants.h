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

#include <cmath>  // Include cmath for std::acos
#include <string>
#include <iostream>
#include <vector> // For handling matrices in multi-phase systems
#include <complex>

using namespace std;
using namespace SymEngine; // Use the SymEngine namespace


// Define a constant for π (pi) using SymEngine
extern const RCP<const Basic> PI;

// Define a constant for π (pi) using standard C++
extern const double M_PI ;

extern DenseMatrix createZeroMatrix(int size1, int size2);

#endif // CONSTANTS_H
