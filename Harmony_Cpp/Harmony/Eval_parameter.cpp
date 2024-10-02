#include "Cable.h"
#include "eval_parameter.h"



//using SymEngine::linalg::inv; // Import the inv function from the linalg namespace

/*std::vector<std::vector<SymEngine::RCP<const SymEngine::Basic>>> convert_to_symengine_matrix(const std::vector<std::vector<std::complex<double>>>& input_matrix) {
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

// Function to substitute a symbol with a numerical value
RCP<const Basic> substitute_symbol(const RCP<const Basic>& expr, const std::string& symbol_name, double value) {
	RCP<const Symbol> symbol = SymEngine::symbol(symbol_name);
	RCP<const Basic> value_expr = real_double(value);
	map_basic_basic subs_map;
	subs_map[symbol] = value_expr;
	return expr->subs(subs_map);
}

std::pair<std::vector<std::vector<std::complex<double>>>, std::vector<std::vector<std::complex<double>>>> Eval_parameter :: eval_parameters(const Cable& c, const std::complex<double>& s_param) {
	try {
		// Step 1: Evaluate P
		std::vector<std::vector<SymEngine::RCP<const SymEngine::Basic>>> c_P = c.getCP();

		// Initialize the complex array P
		std::vector<std::vector<std::complex<double>>> P;
		// Ensure the symbol 's' is substituted with 's_param'
		for (const auto& row : c_P) {
			std::vector<std::complex<double>> P_row;
			for (const auto& elem : row) {
				try {
					auto substituted_elem = substitute_symbol(elem, "s", std::real(s_param));
					double real_part = SymEngine::eval_double(*substituted_elem);
					std::complex<double> complex_value(real_part);
					P_row.push_back(complex_value);
				}
				catch (const SymEngineException &e) {
					std::cerr << "Error substituting or evaluating expression: " << e.what() << std::endl;
					throw;
				}
			}
			P.push_back(P_row);
		}

		// Evaluate Z matrix
		std::vector<std::vector<SymEngine::RCP<const SymEngine::Basic>>> c_Z = c.getCZ();

		std::vector<std::vector<std::complex<double>>> Z;

		for (const auto& row : c_Z) {
			std::vector<std::complex<double>> Z_row;

			for (const auto& elem : row) {
				try {
					auto substituted_elem = substitute_symbol(elem, "s", std::real(s_param));
					double real_part = SymEngine::eval_double(*substituted_elem);
					std::complex<double> complex_value(real_part);
					Z_row.push_back(complex_value);
				}
				catch (const SymEngineException &e) {
					std::cerr << "Error substituting or evaluating expression: " << e.what() << std::endl;
					throw;
				}
			}
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


		if (c.getEliminate()) {
			int nl = conductors.size(); // Assuming c.conductors is a vector
			int n = positions.size();    // Assuming c.positions is a vector
			std::vector<int> cond_noElim;
			for (int i = 1; i <= n; ++i) {
				cond_noElim.push_back((i - 1) * nl + 1);
			}
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
	catch (const std::exception& e) {
		std::cerr << "Exception caught in eval_parameters: " << e.what() << std::endl;
		throw;
	}
}*/

