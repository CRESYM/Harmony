#include "Bus.h"
#include "./Elements/Element.h"

// Constructor for Bus
Bus::Bus(const std::string& name, int number) : busName(name), numberPins(number) {}

// Destructor
Bus::~Bus() {
    connectedElements.clear();
}

// Overloaded equality operator to compare bus names
bool Bus::operator==(const char* name) {
    return (busName == name);
}

// Function to attach an element to the bus
void Bus::attachElement(Element* elem) {
    connectedElements.push_back(elem);
}

// Function to print the elements connected to the bus
void Bus::printConnectedElements() {
    std::cout << "[Debug] printConnectedElements() called for bus " << busName << std::endl;
    std::cout << "Bus " << busName << " is connected to the following elements:\n";
    for (Element* elem : connectedElements) {
        std::cout << "  - " << elem->getElementSymbol() << std::endl; // Use getter method here
    }
}

// Default parameter values
const std::vector<std::string> Bus::DefaultACBusParams = { "1", "100", "345", "1.1", "0.9" };
const std::vector<std::string> Bus::DefaultDCBusParams = { "330", "1.1", "0.9" };

// AC Bus Info initialization
std::vector<std::vector<std::string>> Bus::defaultACBusInfo = []() {
    std::vector<std::vector<std::string>> buses;
    for (int i = 1; i <= 5; ++i) {
        std::ostringstream name;
        name << "ACBUS" << std::setw(2) << std::setfill('0') << i;
        std::vector<std::string> busInfo = { name.str() };
        busInfo.insert(busInfo.end(), DefaultACBusParams.begin(), DefaultACBusParams.end());
        buses.push_back(busInfo);
    }
    return buses;
    }();

// DC Bus Info initialization
std::vector<std::vector<std::string>> Bus::defaultDCBusInfo = []() {
    std::vector<std::vector<std::string>> buses;
    for (int i = 1; i <= 3; ++i) {
        std::ostringstream name;
        name << "DCBUS" << std::setw(2) << std::setfill('0') << i;
        std::vector<std::string> busInfo = { name.str() };
        busInfo.insert(busInfo.end(), DefaultDCBusParams.begin(), DefaultDCBusParams.end());
        buses.push_back(busInfo);
    }
    return buses;
    }();

// Accessor functions
const std::vector<std::vector<std::string>>& Bus::getDefaultACBusInfo() {
    return defaultACBusInfo;
}

const std::vector<std::vector<std::string>>& Bus::getDefaultDCBusInfo() {
    return defaultDCBusInfo;
}
