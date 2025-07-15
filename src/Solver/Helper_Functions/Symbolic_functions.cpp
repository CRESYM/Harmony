#include "Symbolic_functions.h"

// Define the constants declared in the header
const RCP<const Basic> PI = real_double(3.141592653589793); // SymEngine π

// Constants
const double mu_0 = 4 * M_PI * 1e-7; // Standard mu_0
const double epsilon_0 = 8.85e-12; // Standard epsilon_0
const double gamma_num = 0.5772156649;

RCP<const Basic> j = I;  // Imaginary unit
RCP<const Basic> omega = symbol("w");
RCP<const Basic> s = symbol("s"); // mul(j, omega);


// Static helper function to create a zero matrix
DenseMatrix createZeroMatrix(int size1, int size2) {
	DenseMatrix zeroMatrix(size1, size2);
	//std::cout << "[Debug] check0\n";
	for (int i = 0; i < size1; ++i) {
		for (int j = 0; j < size2; ++j) {
			zeroMatrix.set(i, j, zero); // Use SymEngine's symbolic `zero`
		}
	}
	return zeroMatrix;
}
// Fill a matrix with zeros
void fillWithZero(DenseMatrix& mat) {
	for (unsigned i = 0; i < mat.nrows(); ++i)
		for (unsigned j = 0; j < mat.ncols(); ++j)
			mat.set(i, j, zero);
}

// Functions for conversion between symbolic and complex or real double scalar/eigen matrix
complex<double> substitute_symbol(const RCP<const Basic>& expr, const std::string& symbol_name, double value) {
	RCP<const Symbol> symbol = SymEngine::symbol(symbol_name);
	RCP<const Basic> value_expr = real_double(value);
	map_basic_basic subs_map;
	RCP<const Basic> r = expr->subs(subs_map);
	return eval_complex_double(*r);
}

complex<double> substitute_symbol(const RCP<const Basic>& expr, RCP<const Basic> symbol, double value) {
	RCP<const Basic> value_expr = real_double(value);
	map_basic_basic subs_map;
	subs_map[symbol] = value_expr;
	RCP<const Basic> r = expr->subs(subs_map);
	return eval_complex_double(*r);
}

complex<double> substitute_symbol(const RCP<const Basic>& expr, const std::string& symbol_name, complex<double> value) {
	RCP<const Symbol> symbol = SymEngine::symbol(symbol_name);
	RCP<const Basic> value_expr = complex_double(value);
	map_basic_basic subs_map;
	RCP<const Basic> r = expr->subs(subs_map);
	return eval_complex_double(*r);
}

complex<double> substitute_symbol(const RCP<const Basic>& expr, RCP<const Basic> symbol, complex<double> value) {
	RCP<const Basic> value_expr = complex_double(value);
	map_basic_basic subs_map;
	subs_map[symbol] = value_expr;
	RCP<const Basic> r = expr->subs(subs_map);
	return eval_complex_double(*r);
}


MatrixXcd substitute_symbol(DenseMatrix M, const string& symbol_name, double value) {
	RCP<const Symbol> symbol = SymEngine::symbol(symbol_name);
	map_basic_basic m;
	m[symbol] = real_double(value);
	MatrixXcd N(M.nrows(), M.ncols());
	for (int i = 0; i < M.nrows(); ++i) {
		for (int j = 0; j < M.ncols(); ++j) {
			RCP<const Basic> r = subs(M.get(i, j), m);
			N(i, j) = eval_complex_double(*r);
		}
	}
	return N;
}

MatrixXcd substitute_symbol(DenseMatrix M, const string& symbol_name, complex<double> value) {
	RCP<const Symbol> symbol = SymEngine::symbol(symbol_name);
	map_basic_basic m;
	m[symbol] = complex_double(value);
	MatrixXcd N(M.nrows(), M.ncols());
	for (int i = 0; i < M.nrows(); ++i) {
		for (int j = 0; j < M.ncols(); ++j) {
			RCP<const Basic> r = subs(M.get(i, j), m);
			N(i, j) = eval_complex_double(*r);
		}
	}
	return N;
}

MatrixXcd substitute_symbol(DenseMatrix M, RCP<const Basic> symbol, double value) {
	map_basic_basic m;
	m[symbol] = real_double(value);
	MatrixXcd N(M.nrows(), M.ncols());
	for (int i = 0; i < M.nrows(); ++i) {
		for (int j = 0; j < M.ncols(); ++j) {
			RCP<const Basic> r = subs(M.get(i, j), m);
			N(i, j) = eval_complex_double(*r);
		}
	}
	return N;
}

MatrixXcd substitute_symbol(DenseMatrix M, RCP<const Basic> symbol, complex<double> value) {
	map_basic_basic m;
	m[symbol] = complex_double(value);
	MatrixXcd N(M.nrows(), M.ncols());
	for (int i = 0; i < M.nrows(); ++i) {
		for (int j = 0; j < M.ncols(); ++j) {
			RCP<const Basic> r = subs(M.get(i, j), m);
			N(i, j) = eval_complex_double(*r);
		}
	}
	return N;
}

RCP<const Basic> substitute_symbols(const RCP<const Basic>& expr, const std::vector<RCP<const Basic>>& symbols, double value) {
	map_basic_basic subs_map;
	for (const auto& symbol : symbols) {
		RCP<const Basic> value_expr = real_double(value);
		subs_map[symbol] = value_expr;
	}
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

MatrixXd kron_reduction(MatrixXd matrix, vector<int> no_eliminate) {
	vector<int> eliminate;
	for (int i = 0; i < matrix.rows(); i++) {
		if (std::find(no_eliminate.begin(), no_eliminate.end(), i) == no_eliminate.end())
			eliminate.push_back(i);
	}
	MatrixXd M = matrix(no_eliminate, no_eliminate) - matrix(no_eliminate, eliminate) * matrix(eliminate, eliminate).inverse() * matrix(eliminate, no_eliminate);

	return M;
}

DenseMatrix kron_reduction(DenseMatrix matrix, vector<int> no_eliminate) {
	vector<int> eliminate;
	for (int i = 0; i < matrix.nrows(); i++) {
		if (std::find(no_eliminate.begin(), no_eliminate.end(), i) == no_eliminate.end())
			eliminate.push_back(i);
	}
	DenseMatrix M1 = createZeroMatrix(no_eliminate.size(), no_eliminate.size());
	DenseMatrix M2 = createZeroMatrix(no_eliminate.size(), eliminate.size());
	DenseMatrix M3 = createZeroMatrix(eliminate.size(), eliminate.size());
	DenseMatrix M4 = createZeroMatrix(eliminate.size(), no_eliminate.size());

	int i_no_eliminate_curr = 0, i_eliminate_curr = 0;
	int j_no_eliminate_curr = 0, j_eliminate_curr = 0;
	for (int i = 0; i < matrix.nrows(); i++) {
		if (std::find(no_eliminate.begin(), no_eliminate.end(), i) != no_eliminate.end()) { // if it is the row that should not be eliminated
			j_no_eliminate_curr = 0; j_eliminate_curr = 0;
			for (int j = 0; j < matrix.nrows(); j++) {
				if (std::find(no_eliminate.begin(), no_eliminate.end(), j) != no_eliminate.end()) {  // if it is the column that should not be eliminated
					M1.set(i_no_eliminate_curr, j_no_eliminate_curr, matrix.get(i, j));
					j_no_eliminate_curr++;
				}
				else { // this column should be eliminated
					M2.set(i_no_eliminate_curr, j_eliminate_curr, matrix.get(i, j));
					j_eliminate_curr++;
				}
			}
			i_no_eliminate_curr += 1;
		}
		else {
			j_no_eliminate_curr = 0; j_eliminate_curr = 0;
			for (int j = 0; j < matrix.nrows(); j++) {
				if (std::find(no_eliminate.begin(), no_eliminate.end(), j) != no_eliminate.end()) {  // if it is the column that should not be eliminated
					M4.set(i_eliminate_curr, j_no_eliminate_curr, matrix.get(i, j));
					j_no_eliminate_curr++;
				}
				else { // this column should be eliminated
					M3.set(i_eliminate_curr, j_eliminate_curr, matrix.get(i, j));
					j_eliminate_curr++;
				}
			}
			i_eliminate_curr++;
		}
	}

	DenseMatrix C1 = createZeroMatrix(no_eliminate.size(), eliminate.size());
	DenseMatrix C2 = createZeroMatrix(no_eliminate.size(), no_eliminate.size());
	inverse_LU(M3, M3);
	mul_dense_dense(M2, M3, C1);
	mul_dense_dense(C1, M4, C2);
	add_dense_dense(M1, C2, C2);

	return C2;
}

bool convertToBoolean(const std::string& value) {
	// Convert string to lowercase for case-insensitive comparison
	std::string lowercaseValue = value;
	std::transform(lowercaseValue.begin(), lowercaseValue.end(), lowercaseValue.begin(),
		[](unsigned char c) { return std::tolower(c); });

	// Check if the string represents true or false
	if (lowercaseValue == "true" || lowercaseValue == "1") {
		return true;
	}
	else if (lowercaseValue == "false" || lowercaseValue == "0") {
		return false;
	}
	else {
		// Handle invalid input
		throw std::invalid_argument("Invalid boolean value: " + value);
	}
}

// Helper symbolic operations
extern RCP<const Basic> inv(const RCP<const Basic>& val) {
	return div(one, val);
}

extern RCP<const Basic> addSym(const RCP<const Basic>& a, const RCP<const Basic>& b) {
	if (a.is_null()) return b;
	if (b.is_null()) return a;
	return add(a, b);
}

extern RCP<const Basic> subSym(const RCP<const Basic>& a, const RCP<const Basic>& b) {
	if (a.is_null()) return mul(integer(-1), b);
	return sub(a, b);
}