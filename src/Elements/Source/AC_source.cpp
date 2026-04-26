#include "AC_source.h"

AC_source::AC_source(const std::string& symbol, const std::string& location, int pins, double Vi, DenseMatrix Z)
	: Source_base(symbol, location, pins)
{
    if (Z.ncols() != 0)  // if there are entries
    {
        if (pins > 0) { // Check for valid number of pins
            if (Z.ncols() == 1) {
                for (int i = 0; i < pins; i++)
                    Y_matrix.set(i, i, div(integer(1), Z.get(0, 0)));
            }
            else if (Z.ncols() == pins) {
                for (int i = 0; i < pins; i++)
                    Y_matrix.set(i, i, div(integer(1), Z.get(0, i)));
            }
            else
                throw invalid_argument("Invalid number of series impedance vector entries: " + Z.ncols());
        }
    }
    else
        throw invalid_argument("Invalid number of pins, must be greater than 0!");

    // Fill in the complete Y parameters
    for (int i = 0; i < pins; i++)
        for (int j = 0; j < pins; j++) {
            Y_matrix.set(pins + i, j, sub(zero, Y_matrix.get(i, j)));
            Y_matrix.set(pins + i, pins + j, Y_matrix.get(i, j));
            Y_matrix.set(i, pins + j, sub(zero, Y_matrix.get(i, j)));
        }

	V.push_back(Vi); V.push_back(Vi); V.push_back(Vi); // Assuming the same voltage for all pins, can be modified for different voltages per pin
	auto Z_value = substitute_symbol(Z.get(0, 0), omega, 2 * M_PI * 50.0).real(); // Assuming 50 Hz for AC source
	Zsrc.push_back(Z_value); // Assuming 50 Hz for AC source
}

AC_source::AC_source(const std::string& symbol, const std::string& location, int pins, double Vi, const std::vector<double>& Z)
    : Source_base(symbol, location, pins)
{
    if (Z.size() != 0)  // if there are entries
    {
        if (pins > 0) { // Check for valid number of pins
            if (Z.size() == 1) {
                for (int i = 0; i < pins; i++)
                    Y_matrix.set(i, i, div(integer(1), real_double(Z[0])));
            }
            else if (Z.size() == pins) {
                for (int i = 0; i < pins; i++)
                    Y_matrix.set(i, i, div(integer(1), real_double(Z[i])));
            }
            else
                throw invalid_argument("Invalid number of series impedance vector entries: " + std::to_string(Z.size()));
        }
    }
    else
        throw invalid_argument("Invalid number of pins, must be greater than 0!");

    // Fill in the complete Y parameters
    for (int i = 0; i < pins; i++) {
        for (int j = 0; j < pins; j++) {
            Y_matrix.set(pins + i, j, sub(zero, Y_matrix.get(i, j)));
            Y_matrix.set(pins + i, pins + j, Y_matrix.get(i, j));
            Y_matrix.set(i, pins + j, sub(zero, Y_matrix.get(i, j)));
        }
		Zsrc.push_back(Z[i]);
    }
    V.push_back(Vi); V.push_back(Vi); V.push_back(Vi); // Assuming the same voltage for all pins, can be modified for different voltages per pin
}

AC_source::AC_source(const std::string& symbol, const std::string& location, int pins, double Vi, const double Z)
    : Source_base(symbol, location, pins)
{
    if (pins > 0) { // Check for valid number of pins
        for (int i = 0; i < pins; i++)
            Y_matrix.set(i, i, div(integer(1), real_double(Z)));
    }
    else
        throw invalid_argument("Invalid number of pins, must be greater than 0!");

    // Fill in the complete Y parameters
    for (int i = 0; i < pins; i++)
        for (int j = 0; j < pins; j++) {
            Y_matrix.set(pins + i, j, sub(zero, Y_matrix.get(i, j)));
            Y_matrix.set(pins + i, pins + j, Y_matrix.get(i, j));
            Y_matrix.set(i, pins + j, sub(zero, Y_matrix.get(i, j)));
        }
    V.push_back(Vi); V.push_back(Vi); V.push_back(Vi); // Assuming the same voltage for all pins, can be modified for different voltages per pin
    Zsrc.push_back(Z); // Assuming 50 Hz for AC source
}

// Destructor
AC_source::~AC_source() {
    // No need for manual memory management for DenseMatrix or other standard library components
    // std::cout << "AC source object for " << getElementSymbol() << " destroyed." << std::endl;
}


void AC_source::writeMNAmatrix(SymEngine::DenseMatrix& matrix, std::unordered_map<Bus*, int>& bus_indices, int location,
    std::map<Element*, std::vector<RCP<const Basic>>>& symbol_map)
{
    Bus* node1 = nullptr;
    Bus* node2 = nullptr;
    for (auto& [bus, index] : connections) {
        if (index == 1) {
            node1 = bus;  // First bus connected to the element
        }
        else if (index == 2) {
            node2 = bus;  // Second bus connected to the element
        }
    }

    std::vector<RCP<const Basic>> symbols;

    for (int p = 0; p < input_pins; ++p) {
        int row = location + p;   // branch current row

        RCP<const Basic> v_sym = symbol("V_" + getElementSymbol() + std::to_string(p));
		symbols.push_back(v_sym);

        if (node1 && (bus_indices.count(node1) != 0)) {
            int r = bus_indices[node1]+p;
			matrix.set(row, matrix.ncols() - 1, addSym(matrix.get(row, matrix.ncols() - 1), v_sym)); // Set voltage symbol in the last column
            matrix.set(row, r, one);
            matrix.set(r, row, one);
        }
        if (node2 && (bus_indices.count(node2) != 0)) {
            int r = bus_indices[node2]+p;
			matrix.set(row, matrix.ncols() - 1, addSym(matrix.get(row, matrix.ncols() - 1), mul(minus_one, v_sym))); // Set voltage symbol in the last column
            matrix.set(row, r, mul(integer(-1), one));
            matrix.set(r, row, mul(integer(-1), one));
        }
    }
	symbol_map[this] = symbols;  // Store the symbols for this element
}

void AC_source::printElementValues() {
	printElementInfo();
}

// LIKELY NEED TO MODIFY THIS FUNCTION TO ACCOUNT FOR PHASE SHIFT IN AC SOURCE
std::vector<MatrixXcd> AC_source::simulateInputStep(
    const std::vector<MatrixXcd>& /*states*/, int nKeep) const
{
    MatrixXcd Vi = MatrixXcd::Zero(3, nKeep);
	Vi(0, 1) = std::complex<double>(V[0], 0.0);
    return { Vi };
}