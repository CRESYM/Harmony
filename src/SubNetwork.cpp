#include "SubNetwork.h"

SubNetwork::SubNetwork() : Network(), subnetworkName("Unnamed_SubNetwork") {}

SubNetwork::SubNetwork(const std::string& name)
    : Network(), subnetworkName(name) {
}

SubNetwork::~SubNetwork() = default;

void SubNetwork::printInfo() const {
    std::cout << "SubNetwork Name: " << subnetworkName << std::endl;
    std::cout << "Contained Buses: " << getBuses().size() << std::endl;
    
    std::cout << "Contained Elements: " << getElements().size() << std::endl;
	std::cout << "Output Buses: " << outputBuses.size() << std::endl;
}