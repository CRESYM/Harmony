#include "Constants.h"

// Define the constants declared in the header
const RCP<const Basic> PI = real_double(3.141592653589793); // SymEngine π
const double M_PI = std::acos(-1.0);                        // Standard C++ π
// Constants
const double mu_0 = 4 * M_PI * 1e-7; // Standard mu_0
const double epsilon_0 = 8.85e-12; // Standard epsilon_0

RCP<const Basic> j = I;  // Imaginary unit
RCP<const Basic> omega = symbol("w");
RCP<const Basic> s = symbol("s"); // mul(j, omega);

// Static helper function to create a zero matrix
DenseMatrix createZeroMatrix(int size1, int size2) {
    DenseMatrix zeroMatrix(size1, size2);
    for (int i = 0; i < size1; ++i) {
        for (int j = 0; j < size2; ++j) {
            zeroMatrix.set(i, j, zero); // Use SymEngine's symbolic `zero`
        }
    }
    return zeroMatrix;
}

RCP<const Basic> substitute_symbol(const RCP<const Basic>& expr, const std::string& symbol_name, double value) {
	RCP<const Symbol> symbol = SymEngine::symbol(symbol_name);
	RCP<const Basic> value_expr = real_double(value);
	map_basic_basic subs_map;
	subs_map[symbol] = value_expr;
	return expr->subs(subs_map);
}

double eval_basic(const RCP<const Basic>& expr) {
	try {
		return eval_double(*expr);
	}
	catch (const SymEngineException& e) {
		std::cerr << "SymEngineException: " << e.what() << std::endl;
		throw;
	}
	catch (const std::exception& e) {
		std::cerr << "Standard exception: " << e.what() << std::endl;
		throw;
	}
}

//function kron(matrix::Array{ Complex }, no_eliminate::Array{ Int })
//n = size(matrix, 1)
//eliminate = setdiff(1:n, no_eliminate)
//matrix = matrix [[no_eliminate; eliminate], :]
//matrix = matrix[:, [no_eliminate; eliminate]]
//
//n_noElim = length(no_eliminate)
//matrix = matrix[1:n_noElim, 1 : n_noElim] - matrix[1:n_noElim, 1 + n_noElim : end] *
//inv(matrix[1 + n_noElim:end, 1 + n_noElim : end]) * matrix[1 + n_noElim:end, 1 : n_noElim]
//end

