#include "bus.h"
#include "element.h"

#include <iostream>
#include <sstream>  // Include for string streams
#include <symengine/printers.h>  // Correct header for printing

// Constructor for Bus
Bus::Bus(const std::string& name) : busName(name) {}

// Function to attach an element to the bus
void Bus::attachElement(Element* elem) {
    connectedElements.push_back(elem);
}

// Function to print the elements connected to the bus
void Bus::printConnectedElements() {
    std::cout << "Bus " << busName << " is connected to the following elements:\n";
    for (Element* elem : connectedElements) {
        std::cout << "  - " << elem->getElementSymbol() << std::endl; // Use getter method here
        //std::cout << "  - " << elem->element_symbol << std::endl;
    }
}

