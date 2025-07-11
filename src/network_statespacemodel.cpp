#include "network.h"      
#include "Element.h"      
#include "Bus.h"    
#include "Resistor.h"
#include "Capacitor.h"
#include "Inductor.h"
#include "AC_Source.h"
#include "Switch.h"

#include <algorithm>      
#include <stdexcept>     
#include <iostream>        

// Getter implementations
int Network::getNumberStateVariables() const {
    return state_variables;
}

int Network::getStateVariablePosition() const {
    return number_nodes + number_current_sources + number_switches - 1;
}

const std::vector<RCP<const Symbol>>& Network::getStateVariableSymbols() const {
    return state_variables_symbols;
}

int Network::getNumberIndependentSource() const {
    return number_independent_sources;
}

const std::vector<RCP<const Symbol>>& Network::getSourceSymbols() const {
    return sources_symbols;
}

int Network::getNumberEquations() const {
    return total_number_equations;
}

int  Network::getNumberOutputs() const {
    return number_outputs;
}

const std::vector<int>& Network::getOutputIndexes() const {
    return output_indexes;
}

//std::unordered_map<Bus*, int> Network::getBusIndexMap() const {
//    std::unordered_map<Bus*, int> busIndexMap;
//    int index = 0;
//    for (const auto& pair : buses) {
//        Bus* busPtr = pair.second;
//        busIndexMap[busPtr] = index++;
//    }
//    return busIndexMap;
//}

const std::unordered_map<Bus*, int>& Network::getBusIndexMap() const {
    return bus_indices;
}

void Network::assignMatrixIndices() {
    bus_indices.clear();
    current_source_indices.clear();
    state_var_indices.clear();

    int bus_index = 0;
    // Assign indices to buses, excluding the reference bus (assuming "bus0" is ground)
    for (const auto& [name, busPtr] : buses) {
        if (busPtr->getBusName() == "bus0") continue;
        bus_indices[busPtr] = bus_index++;
    }
    std::cout << "[assignMatrixIndices] Bus indices:\n";
    for (auto& [busPtr, idx] : bus_indices) {
        std::cout << "  Bus " << busPtr->getBusName() << " -> " << idx << "\n";
    }

    int current_source_index = bus_index;  // after buses
    // Independent voltage sources introduce an extra unknown: their current
    for (const auto& [name, elementPtr] : elements) {
        if (auto ac_source = dynamic_cast<AC_source*>(elementPtr)) {
            current_source_indices[elementPtr] = current_source_index++;
        }
    }
    std::cout << "[assignMatrixIndices] Current source indices:\n";
    for (auto& [elPtr, idx] : current_source_indices) {
        std::cout << "  Element pointer: " << elPtr << " -> " << idx << "\n";
    }

    int state_var_idx_counter = current_source_index; // Use a distinct counter for clarity here
    for (const auto& [name, elementPtr] : elements) {
        if (dynamic_cast<Capacitor*>(elementPtr) ||
            dynamic_cast<Inductor*>(elementPtr))
        {
            state_var_indices[elementPtr] = state_var_idx_counter++;
        }
    }
    std::cout << "[assignMatrixIndices] State variable indices:\n";
    for (auto& [elPtr, idx] : state_var_indices) {
        std::cout << "  Element pointer: " << elPtr << " -> " << idx << "\n";
    }

    // --- ADD THIS LINE ---
    this->state_variables = state_var_indices.size(); // Update the member variable
    // --- END ADDITION ---
}


const std::unordered_map<Element*, int>& Network::getCurrentSourceIndexMap() const {
    return current_source_indices;
}

const std::unordered_map<Element*, int>& Network::getStateVarIndexMap() const {
    return state_var_indices;
}

void Network::finalizeCounts() {
    number_resistors = 0;
    number_inductors = 0;
    number_capacitors = 0;
    number_current_sources = 0;
    number_switches = 0;
    number_independent_sources = 0;
    number_elements = static_cast<int>(elements.size());
    // number_nodes = static_cast<int>(buses.size());
    number_nodes = static_cast<int>(buses.size()) - 1;  //consider bus0 as reference
    if (number_nodes < 0) number_nodes = 0;

    for (const auto& [name, element] : elements) {
        if (dynamic_cast<Resistor*>(element)) {
            number_resistors++;
        }
        else if (dynamic_cast<Inductor*>(element)) {
            number_inductors++;
        }
        else if (dynamic_cast<Capacitor*>(element)) {
            number_capacitors++;
        }
        else if (dynamic_cast<AC_source*>(element)) {
            number_current_sources++;

        }
        else if (dynamic_cast<Switch*>(element)) {
            number_switches++;
        }
        // Add other cases as needed
    }

    // state_variables = number_inductors + number_capacitors;
    state_variables = number_current_sources + number_inductors + number_capacitors;
    //total_number_equations = number_nodes + number_current_sources + number_switches - 1;
    total_number_equations = number_nodes + number_current_sources + number_switches;

    number_outputs = static_cast<int>(output_indexes.size());

    std::cout << "[Network] Nodes: " << number_nodes << std::endl;
    std::cout << "  AC sources: " << number_current_sources << std::endl;
    std::cout << "  Resistors: " << number_resistors << std::endl;
    std::cout << "  Inductors: " << number_inductors << std::endl;
    std::cout << "  Capacitors: " << number_capacitors << std::endl;
    std::cout << "  Switches: " << number_switches << std::endl;
    std::cout << "  Independent sources: " << number_independent_sources << std::endl;
    std::cout << "  Total elements: " << number_elements << std::endl;
    std::cout << "  State variables: " << state_variables << std::endl;
    std::cout << "  Total equations: " << total_number_equations << std::endl;
    std::cout << "  Outputs: " << number_outputs << std::endl;
}
