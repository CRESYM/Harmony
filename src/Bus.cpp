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

