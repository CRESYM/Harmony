#include "network.h"      
#include "Element.h"      
#include "Bus.h"          

#include <algorithm>      
#include <stdexcept>     
#include <iostream>       

// Getter implementations
int Network::getNumberStateVariables() const {
    return state_variables;
}

int Network::getStateVariablePosition() const {
    return number_nodes + number_voltage_sources + number_switches - 1;
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