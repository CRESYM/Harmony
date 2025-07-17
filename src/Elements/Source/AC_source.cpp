#include "AC_source.h"

AC_source::AC_source(const std::string& symbol, int pins, DenseMatrix Z)
	: Element(symbol, pins, pins)
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
}

AC_source::AC_source(const std::string& symbol, int pins, const std::vector<double>& Z)
    : Element(symbol, pins, pins)
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
                throw invalid_argument("Invalid number of series impedance vector entries: " + Z.size());
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
}

AC_source::AC_source(const std::string& symbol, int pins, const double Z)
    : Element(symbol, pins, pins)
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
}

// Destructor
AC_source::~AC_source() {
    // No need for manual memory management for DenseMatrix or other standard library components
    // std::cout << "AC source object for " << getElementSymbol() << " destroyed." << std::endl;
}


// Power flow computation for AC networks
void AC_source::computePowerFlowAC(std::map<std::string, std::map<std::string, double>>& branchData,
    std::map<std::string, double>& globalParams) const {
    int key = branchData.size();  // Unique branch identifier
    branchData[std::to_string(key)]["generator"] = 1;

    // Compute generator impedance at operational frequency
    std::complex<double> s = globalParams["omega"] * std::complex<double>(0, 1);

    // Convert SymEngine expression to double
    double Y_00_real = SymEngine::eval_double(*Y_matrix.get(0, 0));
    std::complex<double> Y_00(Y_00_real, 0.0);

    if (Y_00 == std::complex<double>(0, 0)) {
        throw std::runtime_error("Y_matrix(0,0) is zero, division by zero error.");
    }

    std::complex<double> Z_eq = std::complex<double>(1.0) / Y_00 / globalParams["Z_base"];

    branchData[std::to_string(key)]["br_r"] = std::real(Z_eq);
    branchData[std::to_string(key)]["br_x"] = std::imag(Z_eq);
    branchData[std::to_string(key)]["g_fr"] = 0;
    branchData[std::to_string(key)]["b_fr"] = 0;
    branchData[std::to_string(key)]["g_to"] = 0;
    branchData[std::to_string(key)]["b_to"] = 0;
}

// Power flow computation for DC networks
void AC_source::computePowerFlowDC(std::map<std::string, std::map<std::string, double>>& branchDCData,
    std::map<std::string, double>& globalParams) const {
    int key = branchDCData.size();  // Unique DC branch identifier
    branchDCData[std::to_string(key)]["l"] = 0.0;
    branchDCData[std::to_string(key)]["c"] = 0.0;

    // Compute Y parameters at low frequency (DC)
    std::complex<double> s = std::complex<double>(0, 1e-6);

    // Convert SymEngine expression to double
    double Y_00_real = SymEngine::eval_double(*Y_matrix.get(0, 0));
    std::complex<double> Y_00(Y_00_real, 0.0);

    if (Y_00 == std::complex<double>(0, 0)) {
        throw std::runtime_error("Y_matrix(0,0) is zero, division by zero error.");
    }

    std::complex<double> Z_eq = std::complex<double>(1.0) / Y_00 / globalParams["Z_base"];

    branchDCData[std::to_string(key)]["r"] = std::real(Z_eq);
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

