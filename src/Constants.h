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
#include <Eigen/Sparse>
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
#include <filesystem>
#include <chrono>
#include <set>

#include <matplot/matplot.h>
#include "gurobi_c++.h"

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

template<typename Table>
Eigen::MatrixXd map2dense(const Table& tbl,
    const std::vector<std::string>& colNames)
{
    const int nRow = static_cast<int>(tbl.size());
    const int nCol = static_cast<int>(colNames.size());
    Eigen::MatrixXd M(nRow, nCol);

    for (const auto& [rowKey, colMap] : tbl)
    {
        int r = std::stoi(rowKey);
        for (int c = 0; c < nCol; ++c)
        {
            auto it = colMap.find(colNames[c]);
            M(r, c) = (it != colMap.end()) ? it->second : 0.0;
        }
    }
    return M;
}


#endif // CONSTANTS_H
