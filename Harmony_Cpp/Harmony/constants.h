#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <symengine/basic.h>
#include <symengine/expression.h>
#include <symengine/real_double.h>
#include <symengine/eval.h>
#include <cmath>  // Include cmath for std::acos

using namespace SymEngine;

// Define a constant for π (pi) using SymEngine
extern const RCP<const Basic> PI;

// Define a constant for π (pi) using standard C++
extern const double M_PI ;

#endif // CONSTANTS_H
