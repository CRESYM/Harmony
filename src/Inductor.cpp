#include "Inductor.h"
#include <symengine/symbol.h>
#include <symengine/mul.h>
#include <symengine/integer.h>
#include <iostream>

using namespace SymEngine;

// Frequency-domain constructor (symbolic Y = 1 / (sL))
Inductor::Inductor(const std::string& symbol, int pins, const std::vector<double>& inductance)
	: Element(symbol, pins, pins), L(inductance)
{
    if (pins == 1) {
        RCP<const Basic> sL = mul(s, real_double(L[0]));
        RCP<const Basic> Yval = div(one, sL);
        Y_matrix.set(0, 0, Yval);
        Y_matrix.set(0, 1, mul(integer(-1), Yval));
        Y_matrix.set(1, 0, mul(integer(-1), Yval));
        Y_matrix.set(1, 1, Yval);
    }
    else {
        for (int i = 0; i < pins; ++i) {
            RCP<const Basic> sL = mul(s, real_double(L[i]));
            RCP<const Basic> Yval = div(one, sL);
            int a = 2 * i;
            int b = 2 * i + 1;
            Y_matrix.set(a, a, Yval);
            Y_matrix.set(a, b, mul(integer(-1), Yval));
            Y_matrix.set(b, a, mul(integer(-1), Yval));
            Y_matrix.set(b, b, Yval);
        }
    } 
}

// MNA matrix writer
void Inductor::writeMNAmatrix(SymEngine::DenseMatrix& matrix, std::unordered_map<Bus*, int>& bus_indices, int location,
    std::map<Element*, std::vector<RCP<const Basic>>>& symbol_map)
{
    std::vector<Bus*> buses = getBuses();
    Bus* node1 = buses.size() > 0 ? buses[0] : nullptr;
    Bus* node2 = buses.size() > 1 ? buses[1] : nullptr;

    std::vector<RCP<const Basic>> symbols;

    for (int p = 0; p < input_pins; ++p) {
        int row = location + p;   // branch current row

        RCP<const Basic> il_sym = symbol("Vl_" + getElementSymbol() + std::to_string(p));
        symbols.push_back(il_sym);

		matrix.set(row, row, one);  // Set diagonal to 1 for current equation
        if (node1 && (bus_indices.count(node1) != 0)) {
            int r = bus_indices[node1] + p;
            RCP<const Basic> Lsym = real_double(L[p]);
            matrix.set(row, r, div(mul(integer(-1), one), Lsym));
            matrix.set(row, matrix.ncols() - 1, mul(integer(-1), il_sym));
        }
        if (node2 && (bus_indices.count(node2) != 0)) {
            int r = bus_indices[node2] + p;
            RCP<const Basic> Lsym = real_double(L[p]);
            matrix.set(row, r, div(one, Lsym));
            matrix.set(row, matrix.ncols() - 1, il_sym);
        }
    }
    symbol_map[this] = symbols;  // Store the symbols for this element
}

// Access initial value
double Inductor::getInitialCurrent() const {
    return initial_value;
}

// Print values
void Inductor::printElementValues() {
    std::cout << "Inductor symbol: " << getElementSymbol() << std::endl;
    std::cout << "Initial current: " << initial_value << " A" << std::endl;
    std::cout << "Y matrix (symbolic):\n" << Y_matrix.__str__() << std::endl;
}
