#include "symbol.h" 
#include "basic.h"
#include "SymbolicType.h"
#include "symengine_n.h"
#include "Kron.h"
#include "Cable.h"
#include "eval_parameter.h"

#include <complex>
#include <string>
#include <vector>
//#include "eigen-3.4.0/Eigen/Dense"
#include <unordered_map>
#include <iostream> //Include for debugging
#include <cstddef> // For NULL


// Define an alias for the map type
using CMapBasicBasic = std::unordered_map<Basic, Basic>;

// Declare the function signature

extern "C" {
	unsigned int basic_subs(Basic *result, const Basic *expr, const Basic *var, const Basic *val);
}

// Define the function implementation
unsigned int basic_subs_cpp(Basic *result, const Basic *expr, const Basic *var, const Basic *val) {
	return basic_subs(result, expr, var, val);
}

Basic subs(const SymbolicType& ex, const CMapBasicBasic& d) {
	Basic s;

	Basic expr(ex.getExpression()); // Assuming ex.getExpression() returns a string representing the expression
	unsigned int err_code = basic_subs_cpp(&s, &expr, &s, nullptr);

	// Check for errors and throw an exception if needed
	//throw_if_error(err_code, ex);

	// Return the substituted expression
	return s;
}

SymbolicType symbols(const std::string& name) {
	return SymbolicType(name);
}

// Function to calculate the inverse of a matrix
std::vector<std::vector<Complex>> inv(const std::vector<std::vector<Complex>>& matrix) {
	int n = matrix.size();

	// Create an augmented matrix [A | I], where A is the input matrix and I is the identity matrix
	std::vector<std::vector<Complex>> augmentedMatrix(n, std::vector<Complex>(2 * n));
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n; ++j) {
			augmentedMatrix[i][j] = matrix[i][j];
			// Set the diagonal elements of the identity matrix to 1
			augmentedMatrix[i][j + n] = (i == j) ? Complex(1.0, 0.0) : Complex(0.0, 0.0);
		}
	}

	// Perform Gaussian elimination to obtain row-echelon form
	for (int i = 0; i < n; ++i) {
		// Find pivot row
		int pivotRow = i;
		for (int j = i + 1; j < n; ++j) {
			if (augmentedMatrix[j][i].abs() > augmentedMatrix[pivotRow][i].abs()) {
				pivotRow = j;
			}
		}
		// Swap rows if necessary
		if (pivotRow != i) {
			std::swap(augmentedMatrix[i], augmentedMatrix[pivotRow]);
		}
		// Perform row operations to create zeros below the pivot
		for (int j = i + 1; j < n; ++j) {
			Complex ratio = augmentedMatrix[j][i] / augmentedMatrix[i][i];
			for (int k = i; k < 2 * n; ++k) {
				augmentedMatrix[j][k] -= ratio * augmentedMatrix[i][k];
			}
		}
	}

	// Perform back substitution to obtain reduced row-echelon form
	for (int i = n - 1; i > 0; --i) {
		for (int j = i - 1; j >= 0; --j) {
			Complex ratio = augmentedMatrix[j][i] / augmentedMatrix[i][i];
			for (int k = i; k < 2 * n; ++k) {
				augmentedMatrix[j][k] -= ratio * augmentedMatrix[i][k];
			}
		}
	}

	// Divide each row by its leading coefficient to obtain the inverse matrix
	std::vector<std::vector<Complex>> inverseMatrix(n, std::vector<Complex>(n));
	for (int i = 0; i < n; ++i) {
		Complex leadingCoeff = augmentedMatrix[i][i];
		for (int j = 0; j < n; ++j) {
			inverseMatrix[i][j] = augmentedMatrix[i][j + n] / leadingCoeff;
		}
	}

	return inverseMatrix;
}

// Function to multiply a scalar with a matrix
std::vector<std::vector<Complex>> multiplyScalar(const Complex& scalar, const std::vector<std::vector<Complex>>& matrix) {
	std::vector<std::vector<Complex>> result;
	for (const auto& row : matrix) {
		std::vector<Complex> newRow;
		for (const auto& elem : row) {
			Complex product = scalar * elem;
			newRow.push_back(product);
		}
		result.push_back(newRow);
	}
	return result;
}

std::pair<std::vector<std::vector<Complex>>, std::vector<std::vector<Complex>>> eval_parameters(const Cable& c, const Complex& s) {
	// Step 1: Evaluate P
	std::vector<std::vector<Complex>> P;
	// Substitute s into c.P and apply N
	for (const auto& row : c.P) {
		std::vector<Complex> newRow;
		for (const auto& expr : row) {
			// Convert double expr to SymbolicType
			SymbolicType symbolicExpr(std::to_string(expr)); // Convert double to string
			// Create a map with s as the key and its value as the expression
			CMapBasicBasic substitutionMap;
			substitutionMap.emplace(symbols("s"), symbolicExpr);
			// Substitute s into the expression and apply N
			// Implement subs and N functions based on your codebase
			Complex result = N(subs(symbols("s"), substitutionMap));
			newRow.push_back(result);
		}
		P.push_back(newRow);
	}

	// Convert P to Array{Float64} and then to Array{Complex}
	for (auto& row : P) {
		for (auto& elem : row) {
			double realPart = elem.getReal();
			double imagPart = elem.getImag();
			elem = Complex(realPart, imagPart);
		}
	}

	// Step 2: Evaluate Z (similar to Step 1)
	std::vector<std::vector<Complex>> Z;
	// Substitute s into c.Z and apply N
	for (const auto& row : c.Z) {
		std::vector<Complex> newRow;
		for (const auto& expr : row) {
			// Convert double expr to SymbolicType
			SymbolicType symbolicExpr(std::to_string(expr)); // Convert double to string
			// Create a map with s as the key and its value as the expression
			CMapBasicBasic substitutionMap;
			substitutionMap.emplace(symbols("s"), symbolicExpr);
			// Substitute s into the expression and apply N
			// Implement subs and N functions based on your codebase
			Complex result = N(subs(symbols("s"), substitutionMap));
			newRow.push_back(result);
		}
		Z.push_back(newRow);
	}

	// Convert Z to Array{Float64} and then to Array{Complex}
	for (auto& row : Z) {
		for (auto& elem : row) {
			double realPart = elem.getReal();
			double imagPart = elem.getImag();
			elem = Complex(realPart, imagPart);
		}
	}

	// Step 3: Kron elimination if necessary
	auto& conductors = c.getCableConductors();
	auto& positions = c.getPositions();

	if (c.getEliminate()) {
		int nₗ = conductors.size(); // Assuming c.conductors is a vector
		int n = positions.size();    // Assuming c.positions is a vector
		std::vector<int> cond_noElim;
		for (int i = 1; i <= n; ++i) {
			cond_noElim.push_back((i - 1) * nₗ + 1);
		}

		// Implement Kron elimination for Z and P matrices
		Kron::eliminate(Z, cond_noElim);
		Kron::eliminate(P, cond_noElim);
	}

	// Step 4: Calculate Y
	std::vector<std::vector<Complex>> Y;
	// Calculate Y = s * inv(P)
	// Implement inv function based on your codebase
	// Implement multiplication of a scalar (s) with a matrix (inv(P))
	Y = multiplyScalar(s, inv(P));

	// Debugging: Print matrices P, Z, and Y (optional)
	//printMatrix(P);
	//printMatrix(Z);
	//printMatrix(Y);

	// Return Z and Y as a pair
	return std::make_pair(Z, Y);
}
