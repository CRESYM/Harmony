#ifndef SYMENGINE_TYPES_H
#define SYMENGINE_TYPES_H

/**
 * @file SymEngineTypes.h
 * @brief SymEngine headers and type aliases for symbolic network assembly.
 */

#include "HarmonyTypes.h"

#include <symengine/add.h>
#include <symengine/basic.h>
#include <symengine/complex.h>
#include <symengine/complex_double.h>
#include <symengine/eval.h>
#include <symengine/eval_double.h>
#include <symengine/expression.h>
#include <symengine/functions.h>
#include <symengine/matrices/identity_matrix.h>
#include <symengine/matrices/immutable_dense_matrix.h>
#include <symengine/matrices/matrix_add.h>
#include <symengine/matrices/matrix_mul.h>
#include <symengine/matrix.h>
#include <symengine/matrix_expressions.h>
#include <symengine/mul.h>
#include <symengine/number.h>
#include <symengine/polys/basic_conversions.h>
#include <symengine/pow.h>
#include <symengine/printers.h>
#include <symengine/real_double.h>
#include <symengine/real_mpfr.h>
#include <symengine/simplify.h>
#include <symengine/subs.h>
#include <symengine/symbol.h>
#include <symengine/symengine_config.h>

using SymEngine::Basic;
using SymEngine::ComplexDouble;
using SymEngine::DenseMatrix;
using SymEngine::I;
using SymEngine::RCP;
using SymEngine::RealMPFR;
using SymEngine::Symbol;
using SymEngine::SymEngineException;
using SymEngine::complex_double;
using SymEngine::integer;
using SymEngine::map_basic_basic;
using SymEngine::minus_one;
using SymEngine::mul;
using SymEngine::one;
using SymEngine::real_double;
using SymEngine::symbol;
using SymEngine::vec_uint;
using SymEngine::zero;

#endif // SYMENGINE_TYPES_H
