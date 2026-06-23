/**
 * @file Switch.cpp
 * @brief Implementation of Multi-phase switch element for open/closed branch control.
 */
#include "Switch.h"
using namespace SymEngine;

Switch::Switch(const std::string& symbol, const std::string& location, int pins, const std::vector<bool>& state)
    : Element(symbol, location, pins, pins), phaseState(state)
{
    if (state.size() == 1 && pins > 1) { // single-phase
        for (int i = 1; i < pins; ++i) {
            phaseState.push_back(state[0]);  // Fill with the same capacitance value
        }
    }
}

void Switch::writeMNAmatrix(SymEngine::DenseMatrix& matrix, std::unordered_map<Bus*, int>& bus_indices, int location,
    std::map<Element*, std::vector<RCP<const Basic>>>& symbol_map)
{
    std::vector<Bus*> buses = getBuses();
    Bus* node1 = buses.size() > 0 ? buses[0] : nullptr;
    Bus* node2 = buses.size() > 1 ? buses[1] : nullptr;

    for (int p = 0; p < input_pins; p++) {
        int row = location + p;   // branch current row

        if (phaseState[p]) { // Closed switch
            if (node1 && (bus_indices.count(node1) != 0)) {
                int i = bus_indices[node1]+p;
                matrix.set(row, i, one);
                matrix.set(i, row, one);
            }
            if (node2 && (bus_indices.count(node2) != 0)) {
                int j = bus_indices[node2]+p;
                matrix.set(row, j, minus_one);
                matrix.set(j, row, minus_one);
            }
        } else { // Open switch
			// Do nothing, the switch is open, no connection
		}
    }
}


void Switch::printElementValues() {
    std::cout << "Switch " << element_symbol << " state: ";
    for (size_t i = 0; i < phaseState.size(); ++i) {
        std::cout << "[" << i << "]=" << (phaseState[i] ? "closed" : "open") << " ";
    }
    std::cout << std::endl;
}

