#include "Cable.h"
#include "eval_parameter.h"
//#include "MySymbol.h" 
//#include "MyBasic.h"
//#include "MySymbolicType.h"
#include "symengine_n.h"
#include "Kron.h"

#include <iostream>
#include <symengine_config.h>
#include <basic.h>
#include <symbol.h>
#include <functions.h>
#include <matrix.h>
#include <matrix_expressions.h>
#include <complex.h>
#include <complex_double.h>
#include <eval_mpc.h> // Include for mpc functions
#include <complex_mpc.h>
#include <add.h>
#include <matrices/transpose.h>
#include <eval_double.h>
#include <real_double.h>
#include <rational.h>

using SymEngine::Basic;
using SymEngine::Symbol;
using SymEngine::FunctionSymbol;
using SymEngine::add;
using SymEngine::mul;
using SymEngine::div;
using SymEngine::eye;
using SymEngine::transpose;
using SymEngine::ComplexDouble;
using SymEngine::Complex;
using SymEngine::real_double;
using SymEngine::conjugate;
using SymEngine::I;

using namespace SymEngine;

//using SymEngine::linalg::inv; // Import the inv function from the linalg namespace

std::vector<std::vector<SymEngine::RCP<const SymEngine::Basic>>> convert_to_symengine_matrix(const std::vector<std::vector<std::complex<double>>>& input_matrix) {
	std::vector<std::vector<SymEngine::RCP<const SymEngine::Basic>>> sym_matrix;

	for (const auto& row : input_matrix) {
		std::vector<SymEngine::RCP<const SymEngine::Basic>> sym_row;

		for (const auto& elem : row) {
			// Construct SymEngine::Complex object from real and imaginary parts
			SymEngine::RealDouble real_part(elem.real());
			SymEngine::RealDouble imag_part(elem.imag());

			// Construct SymEngine::Complex object from rational_class parts
			auto complex_num = SymEngine::Complex::from_two_nums(real_part, imag_part);
			// Add the constructed SymEngine::Complex object to the row
			sym_row.push_back(complex_num);
		}

		// Add the row to the matrix
		sym_matrix.push_back(sym_row);
	}

	return sym_matrix;
}

std::pair<std::vector<std::vector<std::complex<double>>>, std::vector<std::vector<std::complex<double>>>> Eval_parameter::eval_parameters(const Cable & c, const std::complex<double>& s_param) {
	// Step 1: Evaluate P
	std::vector<std::vector<SymEngine::RCP<const SymEngine::Basic>>> c_P;

	// Initialize the complex array P
	std::vector<std::vector<std::complex<double>>> P;

	//const Complex& s = c.getSymbolS();
	const SymEngine::Complex* s_ptr = c.getSymbolS();

	if (s_ptr) {
		// Dereference the pointer to access the object
		const SymEngine::Complex& s = *s_ptr;

		// Iterate over each row in c.P
		for (const auto& row : c_P) {
			std::vector<std::complex<double>> P_row;
			// Iterate over each element in the row
			for (const auto& elem : row) {
				// Evaluate the expression and convert it to double

				double real_part = SymEngine::eval_double(*elem);

				std::complex<double> complex_value(real_part);

				P_row.push_back(complex_value);
			}
			// Add the row to P
			P.push_back(P_row);
		}
	}
	else {
		std::cerr << "Error: SymEngine::Complex pointer is null." << std::endl;
		// Handle the error appropriately, possibly by returning an error indicator or throwing an exception
		return {}; // Return an empty pair
	}

	// Evaluate Z matrix
	std::vector<std::vector<SymEngine::RCP<const SymEngine::Basic>>> c_Z;

	std::vector<std::vector<std::complex<double>>> Z;
	for (const auto& row : c_Z) {
		std::vector<std::complex<double>> Z_row;

		for (const auto& elem : row) {

			double real_part = SymEngine::eval_double(*elem);

			std::complex<double> complex_value(real_part);//, imag_part);
			Z_row.push_back(complex_value);
		}
		// Add the row to Z
		Z.push_back(Z_row);
	}


	// Convert P and Z matrices to arrays of complex numbers
	std::vector<std::vector<std::complex<double>>> P_real, Z_real;
	for (const auto& row_P : P) {
		std::vector<std::complex<double>> row_real_P;
		for (const auto& elem_P : row_P) {
			//row_real_P.emplace_back(std::complex<double>(elem_P), 0.0);
			row_real_P.emplace_back(elem_P);

		}
		P_real.emplace_back(row_real_P);
	}
	for (const auto& row_Z : Z) {
		std::vector<std::complex<double>> row_real_Z;
		for (const auto& elem_Z : row_Z) {
			//row_real_Z.emplace_back(std::complex<double>(std::real(elem_Z), 0.0));
			row_real_Z.emplace_back(elem_Z);
		}
		Z_real.emplace_back(row_real_Z);
	}

	// If eliminate flag is true, apply Kron elimination
	// Step 3: Kron elimination if necessary
	auto& conductors = c.getCableConductors();
	auto& positions = c.getPositions();

	// Convert Z and P matrices to SymEngine complex matrices
	auto sym_Z = convert_to_symengine_matrix(Z);
	auto sym_P = convert_to_symengine_matrix(P);

	if (c.getEliminate()) {
		int nl = conductors.size(); // Assuming c.conductors is a vector
		int n = positions.size();    // Assuming c.positions is a vector
		std::vector<int> cond_noElim;
		for (int i = 1; i <= n; ++i) {
			cond_noElim.push_back((i - 1) * nl + 1);
		}

		// Convert Z and P matrices to SymEngine complex matrices
		std::vector<std::vector<SymEngine::RCP<const SymEngine::Basic>>> sym_Z = convert_to_symengine_matrix(Z);
		std::vector<std::vector<SymEngine::RCP<const SymEngine::Basic>>> sym_P = convert_to_symengine_matrix(P);
	}


	//Step4. Compute shunt admittance matrix Y
	std::vector<std::vector<std::complex<double>>> Y_real;
	for (size_t i = 0; i < P_real.size(); ++i) {
		std::vector<std::complex<double>> row_Y_real;
		for (size_t j = 0; j < P_real[i].size(); ++j) {
			row_Y_real.emplace_back(s_param / P_real[i][j]);
		}
		Y_real.emplace_back(row_Y_real);
	}

	if (Z_real.empty() || Y_real.empty()) {
		// Return a default value if the computations were unsuccessful
		return std::make_pair(std::vector<std::vector<std::complex<double>>>(), std::vector<std::vector<std::complex<double>>>());
	}
	else {
		// Return the computed matrices if the computations were successful
		return std::make_pair(Z_real, Y_real);
	}
}


/*std::tuple<MatrixXcd, MatrixXcd> eval_parameters(const Cable& c, const ComplexDouble& s) {
	// Convert c.P and c.Z to Eigen matrices
	MatrixXd P = Eigen::Map<MatrixXd>(c.P.data()->data(), c.P.size(), c.P[0].size());
	MatrixXd Z = Eigen::Map<MatrixXd>(c.Z.data()->data(), c.Z.size(), c.Z[0].size());

	// Perform substitutions for c.P and c.Z with the symbolic variable 's'
	P = P.array().eval();
	Z = Z.array().eval();

	// Apply Kron elimination if 'c.eliminate' is true
	if (c.eliminate) {
		// Implement Kron elimination here
	}

	// Convert the expressions to complex matrices
	MatrixXcd P_complex = P.cast<ComplexDouble>();
	MatrixXcd Z_complex = Z.cast<ComplexDouble>();

	// Calculate shunt admittance matrix Y
	MatrixXcd Y = s * P_complex.inverse();

	return std::make_tuple(Z_complex, Y);
}*/
/*
// Define an alias for the map type
//using CMapBasicBasic = std::unordered_map<Basic, Basic, BasicHash, BasicEqual>;

// Declare the function signature
extern "C" {
	unsigned int basic_subs(Basic* result, const Basic* expr, const Basic* var, const Basic* val);
}

// Define the function implementation
unsigned int basic_subs_cpp(Basic* result, const Basic* expr, const Basic* var, const Basic* val) {
	// Implement your logic here or call another function that implements basic_subs
	// For example:
	return basic_subs(result, expr, var, val);
}

SymEngine::Basic subs(const SymbolicType& ex, const CMapBasicBasic& d) {
	//Basic s;

	//Basic expr(ex.getExpression()); // Assuming ex.getExpression() returns a string representing the expression
	//unsigned int err_code = basic_subs_cpp(&s, &expr, &s, nullptr);

	// Check for errors and throw an exception if needed
	//throw_if_error(err_code, ex);

	// Return the substituted expression
	//return s;

	// Get the expression string from the SymbolicType object
	std::string expression = ex.getExpression();

	// Create a symbol from the expression string
	auto symbol = SymEngine::symbol(expression);

	// Convert CMapBasicBasic to SymEngine's map_basic_basic
	SymEngine::map_basic_basic subsMap;
	for (const auto& entry : d) {
		subsMap[*entry.first] = *entry.second;
	}

	// Perform substitution using the subs method provided by SymEngine
	auto substitutedExpression = symbol->subs(subsMap);

	// Return the substituted expression
	return substitutedExpression;

	// Convert the expression string to a Basic object
	//SymEngine::Basic expr(expression); // Assuming Basic has a constructor that takes a string

	// Create a Basic object to store the result of substitution
	//SymEngine::Basic substitutedExpression;

	// Call the basic_subs_cpp function passing pointers to the expression and the result
	//unsigned int err_code = basic_subs_cpp(&substitutedExpression, &expr, nullptr, nullptr);

	// Return the substituted expression
	//return substitutedExpression;
}

SymbolicType symbols(const std::string& name) {
	return SymbolicType(name);
}

// Function to calculate the inverse of a matrix
std::vector<std::vector<SymEngine::Complex>> inv(const std::vector<std::vector<SymEngine::Complex>>& matrix) {
	int n = matrix.size();

	// Create an augmented matrix [A | I], where A is the input matrix and I is the identity matrix
	std::vector<std::vector<SymEngine::Complex>> augmentedMatrix(n, std::vector<SymEngine::Complex>(2 * n));
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
			SymEngine::Complex ratio = augmentedMatrix[j][i] / augmentedMatrix[i][i];
			for (int k = i; k < 2 * n; ++k) {
				augmentedMatrix[j][k] -= ratio * augmentedMatrix[i][k];
			}
		}
	}

	// Perform back substitution to obtain reduced row-echelon form
	for (int i = n - 1; i > 0; --i) {
		for (int j = i - 1; j >= 0; --j) {
			SymEngine::Complex ratio = augmentedMatrix[j][i] / augmentedMatrix[i][i];
			for (int k = i; k < 2 * n; ++k) {
				augmentedMatrix[j][k] -= ratio * augmentedMatrix[i][k];
			}
		}
	}

	// Divide each row by its leading coefficient to obtain the inverse matrix
	std::vector<std::vector<SymEngine::Complex>> inverseMatrix(n, std::vector<SymEngine::Complex>(n));
	for (int i = 0; i < n; ++i) {
		SymEngine::Complex leadingCoeff = augmentedMatrix[i][i];
		for (int j = 0; j < n; ++j) {
			inverseMatrix[i][j] = augmentedMatrix[i][j + n] / leadingCoeff;
		}
	}

	return inverseMatrix;
}

// Function to multiply a scalar with a matrix
std::vector<std::vector<SymEngine::Complex>> multiplyScalar(const SymEngine::Complex& scalar, const std::vector<std::vector<SymEngine::Complex>>& matrix) {
	std::vector<std::vector<SymEngine::Complex>> result;
	for (const auto& row : matrix) {
		std::vector<SymEngine::Complex> newRow;
		for (const auto& elem : row) {
			SymEngine::Complex product = scalar * elem;
			newRow.push_back(product);
		}
		result.push_back(newRow);
	}
	return result;
}

std::pair<std::vector<std::vector<SymEngine::Complex>>, std::vector<std::vector<SymEngine::Complex>>> eval_parameters(const Cable& c, const SymEngine::Complex& s) {
	// Step 1: Evaluate P
	std::vector<std::vector<SymEngine::Complex>> P;
	// Substitute s into c.P and apply N
	for (const auto& row : c.P) {
		std::vector<SymEngine::Complex> newRow;
		for (const auto& expr : row) {
			// Convert double expr to SymbolicType
			SymbolicType symbolicExpr(std::to_string(expr)); // Convert double to string
			// Create a map with s as the key and its value as the expression
			CMapBasicBasic substitutionMap;
			//std::unordered_map<Basic, Basic, BasicHash, BasicEqual> substitutionMap;

			substitutionMap.emplace(symbols("s"), symbolicExpr);
			// Substitute s into the expression and apply N
			// Implement subs and N functions based on your codebase
			SymEngine::Complex result = N(subs(SymbolicType(std::to_string(expr)), substitutionMap));
			newRow.push_back(result);
			//std::cout << "hello eval1" << std::endl;
		}
		P.push_back(newRow);
	}

	// Convert P to Array{Float64} and then to Array{Complex}
	for (auto& row : P) {
		for (auto& elem : row) {
			double realPart = elem.getReal();
			double imagPart = elem.getImag();
			elem = SymEngine::Complex(realPart, imagPart);
		}
	}

	// Step 2: Evaluate Z (similar to Step 1)
	std::vector<std::vector<SymEngine::Complex>> Z;
	// Substitute s into c.Z and apply N
	/*for (const auto& row : c.Z) {
		std::vector<Complex> newRow;
		for (const auto& expr : row) {
			// Convert double expr to SymbolicType
			SymbolicType symbolicExpr(std::to_string(expr)); // Convert double to string
			// Create a map with s as the key and its value as the expression
			//CMapBasicBasic substitutionMap;
			//substitutionMap.emplace(symbols("s"), symbolicExpr);
			// Substitute s into the expression and apply N
			// Implement subs and N functions based on your codebase
			//Complex result = N(subs(symbols("s"), substitutionMap));
			std::unordered_map<Basic, Basic> substitutionMap;
			//substitutionMap.emplace(symbols("s"), symbolicExpr);
			//Complex result = N(subs(SymbolicType(std::to_string(expr)), substitutionMap));
			//newRow.push_back(result);
			std::cout << "hello eval2" << std::endl;

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
	//Y = multiplyScalar(s, inv(P));

	// Debugging: Print matrices P, Z, and Y (optional)
	//printMatrix(P);
	//printMatrix(Z);
	//printMatrix(Y);

	// Return Z and Y as a pair
	//return std::make_pair(Z, Y);

	return std::make_pair(std::move(Z), std::move(Y));
	//std::cout << "hello eval2" << std::endl;
}*/

