/**
 * @file DC_source.cpp
 * @brief Implementation of Ideal DC voltage source with optional series impedance.
 */
#include "DC_source.h"

DC_source::DC_source(const std::string& symbol, const std::string& location, int pins, double Vi, double Z)
	: Source_base(symbol, location, pins){
	// For a DC source, the Y-parameter matrix is not frequency-dependent and can have resistive conductance entries based on the internal resistance
    if (pins > 0) { // Check for valid number of pins
        for (int i = 0; i < pins; i++) {
            Y_matrix.set(i, i, div(integer(1), real_double(Z)));
			V.push_back(Vi); // Assuming the same voltage for all pins, can be modified for different voltages per pin
			Zsrc.push_back(Z); // Assuming the same internal resistance for all pins, can be modified for different resistances per pin
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


DC_source::DC_source(const std::string& symbol, const std::string& location, int pins, const vector<double>& Vi, double Z)
    : Source_base(symbol, location, pins) {
    // For a DC source, the Y-parameter matrix is not frequency-dependent and can be set to zero
    if (Vi.size() != 0)  // if there are entries
    {
        if (pins > 0) { // Check for valid number of pins
            if (Vi.size() == pins) {
                for (int i = 0; i < pins; i++) {
                    Y_matrix.set(i, i, div(integer(1), real_double(Z)));
                    Zsrc.push_back(Z);
                    V.push_back(Vi[i]); // Assuming the same voltage for all pins, can be modified for different voltages per pin
                }
            }
            else
                throw invalid_argument("Invalid number of voltage vector entries: " + std::to_string(Vi.size()));
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

DC_source::DC_source(const std::string& symbol, const std::string& location, int pins, double Vi, const vector<double>& Z)
    : Source_base(symbol, location, pins) {
    // For a DC source, the Y-parameter matrix is not frequency-dependent and can be set to zero
    if (Z.size() != 0)  // if there are entries
    {
        if (pins > 0) { // Check for valid number of pins
            if (Z.size() == 1) {
                for (int i = 0; i < pins; i++) {
                    Y_matrix.set(i, i, div(integer(1), real_double(Z[0])));
                    Zsrc.push_back(Z[0]);
                    V.push_back(Vi); // Assuming the same voltage for all pins, can be modified for different voltages per pin
                }
            }
            else if (Z.size() == pins) {
                for (int i = 0; i < pins; i++) {
                    Y_matrix.set(i, i, div(integer(1), real_double(Z[i])));
                    Zsrc.push_back(Z[i]);
                    V.push_back(Vi); // Assuming the same voltage for all pins, can be modified for different voltages per pin
                }
            }
            else
                throw invalid_argument("Invalid number of series impedance vector entries: " + std::to_string(Z.size()));
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

DC_source::DC_source(const std::string& symbol, const std::string& location, int pins, const vector<double>& Vi, const vector<double>& Z)
    : Source_base(symbol, location, pins) {
    // For a DC source, the Y-parameter matrix is not frequency-dependent and can be set to zero
    if (Vi.size() != 0 && Z.size() != 0)  // if there are entries
    {
        if (pins > 0) { // Check for valid number of pins
            if (Vi.size() == pins && Z.size() == pins) {
                for (int i = 0; i < pins; i++) {
                    Y_matrix.set(i, i, div(integer(1), real_double(Z[i])));
                    Zsrc.push_back(Z[i]);
                    V.push_back(Vi[i]); // Assuming the same voltage for all pins, can be modified for different voltages per pin
                }
            }
            else
                throw invalid_argument("Invalid number of voltage or series impedance vector entries: " + std::to_string(Vi.size()) + " or " + std::to_string(Z.size()));
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

void DC_source::writeMNAmatrix(SymEngine::DenseMatrix& matrix,
    std::unordered_map<Bus*, int>& bus_indices, int location,
    std::map<Element*, std::vector<RCP<const Basic>>>& symbol_map)
{
    Bus* node1 = nullptr;
    Bus* node2 = nullptr;
    for (auto& [bus, index] : connections) {
        if (index == 1) node1 = bus;
        else if (index == 2) node2 = bus;
    }

    std::vector<RCP<const Basic>> symbols;

    for (int p = 0; p < input_pins; ++p) {
        int row = location + p;
        RCP<const Basic> v_sym = symbol("V_" + getElementSymbol() + std::to_string(p));
        symbols.push_back(v_sym);

        if (node1 && (bus_indices.count(node1) != 0)) {
            int r = bus_indices[node1] + p;
            matrix.set(row, matrix.ncols() - 1,
                addSym(matrix.get(row, matrix.ncols() - 1), v_sym));
            matrix.set(row, r, one);
            matrix.set(r, row, one);
        }
        if (node2 && (bus_indices.count(node2) != 0)) {
            int r = bus_indices[node2] + p;
            matrix.set(row, matrix.ncols() - 1,
                addSym(matrix.get(row, matrix.ncols() - 1), mul(minus_one, v_sym)));
            matrix.set(row, r, mul(integer(-1), one));
            matrix.set(r, row, mul(integer(-1), one));
        }
    }
    symbol_map[this] = symbols;
}

std::vector<MatrixXcd> DC_source::simulateInputStep(
    const std::vector<MatrixXcd>& /*states*/, int nKeep) const
{
    MatrixXcd Vi = MatrixXcd::Zero(input_pins * 3, nKeep);
    for (int p = 0; p < input_pins; ++p) {
        Vi(p * 3 + 2, 0) = complex<double>(V[p], 0.0);
    }        

    return { Vi };
}