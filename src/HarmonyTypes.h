#ifndef HARMONY_TYPES_H
#define HARMONY_TYPES_H

/**
 * @file HarmonyTypes.h
 * @brief Lightweight shared types: Eigen, STL, and common aliases (no SymEngine/GUI/solvers).
 */

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <unsupported/Eigen/MatrixFunctions>
#include <unsupported/Eigen/NonLinearOptimization>
#include <unsupported/Eigen/NumericalDiff>

#define _USE_MATH_DEFINES
#include <math.h>

#include <algorithm>
#include <any>
#include <atomic>
#include <chrono>
#include <cmath>
#include <complex>
#include <cctype>
#include <cstdlib>
#include <deque>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>

using namespace std;
using namespace std::complex_literals;
using namespace Eigen;

template<typename Table>
Eigen::MatrixXd map2dense(const Table& tbl,
    const std::vector<std::string>& colNames)
{
    const int nRow = static_cast<int>(tbl.size());
    const int nCol = static_cast<int>(colNames.size());
    Eigen::MatrixXd M(nRow, nCol);

    for (const auto& [rowKey, colMap] : tbl) {
        int r = std::stoi(rowKey);
        for (int c = 0; c < nCol; ++c) {
            auto it = colMap.find(colNames[c]);
            M(r, c) = (it != colMap.end()) ? it->second : 0.0;
        }
    }
    return M;
}

#endif // HARMONY_TYPES_H
