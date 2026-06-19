/**
 * @file SubNetwork.cpp
 * @brief Implementation of Hierarchical sub-network view for modular AC/DC grids and converter interfaces.
 */
#include "SubNetwork.h"

SubNetwork::SubNetwork() : Network(), subnetworkName("Unnamed_SubNetwork") {}

SubNetwork::SubNetwork(const std::string& name)
    : Network(), subnetworkName(name) {
}

SubNetwork::~SubNetwork() {
    // Non-owning view of parent Network's buses/elements — do not delete them
    buses.clear();
    elements.clear();
    connections.clear();
    outputBuses.clear();
    ac_grids.clear();
    dc_grids.clear();
    ac_grid_names.clear();
    dc_grid_names.clear();
    converters.clear();
}

void SubNetwork::printInfo() const {
    std::cout << "SubNetwork Name: " << subnetworkName << std::endl;
    std::cout << "Contained Buses: " << getBuses().size() << std::endl;
    
    std::cout << "Contained Elements: " << getElements().size() << std::endl;
	std::cout << "Output Buses: " << outputBuses.size() << std::endl;
}
