// element.cpp

# include "Element.h"
#include "Constants.h"
#include <iostream>
#include <vector>


using namespace SymEngine;

// Destructor
Element::~Element() {}

// Attach terminal to the bus
void Element::attachBus(Bus* bus, int terminal) {
    connections[terminal] = bus;
}

// Getters
std::vector<Bus*> Element::getBuses() {
    std::vector<Bus*> buses;
    for (std::map<int, Bus*>::iterator it = connections.begin(); it != connections.end(); ++it) {
        buses.push_back(it->second);
    }
    return buses;
}

std::vector<Bus*> Element::getOtherBuses(Bus* bus) {
    std::vector<Bus*> buses;
    for (std::map<int, Bus*>::iterator it = connections.begin(); it != connections.end(); ++it) {
        if (bus != it->second)
            buses.push_back(it->second);
    }

    return buses;
}

// Implementation of compute_y_parameters for single-phase (default implementation)
void Element::compute_y_parameters(double frequency) {
    std::cout << "This should be overridden in derived classes." << std::endl;
}

void Element::compute_y_parameters() {
    std::cout << "This should be overridden in derived classes." << std::endl;
}

void Element::printElementValues() {}