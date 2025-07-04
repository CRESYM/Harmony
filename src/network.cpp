#include "network.h"
#include "Element.h" 
#include "Bus.h"
#include <algorithm>
#include <stdexcept>


Network::~Network() {
    // Delete all Bus objects in the network
    for (std::unordered_map<std::string, Bus*>::iterator i = buses.begin(); i != buses.end(); i++)
        delete i->second;
    buses.clear();
    // Delete all Element objects in the network
    for (std::unordered_map<std::string, Element*>::iterator i = elements.begin(); i != elements.end(); i++)
        delete i->second;
    elements.clear();
    // Clear all connections
    connections.clear();
}

// Function to add a bus to the network
void Network::addBus(Bus* bus) {
    buses[bus->getBusName()] = bus;
    // Increment the total pin count if the bus is not ground
    if (bus->getBusName() != "gnd")
        pins += bus->getPinNumber();
}

// Function to add a bus to the network
void Network::addBus(const std::string& busName, Bus* bus) {
    buses[busName] = bus;
    if (busName != "gnd")
        pins += bus->getPinNumber();
}

// Function to add an element to the network
void Network::addElement(Element* elem) {
    elements[elem->getElementSymbol()] = elem;
}

// Function to add an element to the network
void Network::addElement(const std::string& designator, Element* elem) {
    elements[designator] = elem;
}

// Function to connect an element to a bus
void Network::connectElementToBus(Element* elem, int terminal, Bus* bus) {
    int pins;
    if (terminal == 1) {
        pins = elem->getInputPins();
    }
    else if (terminal == 2) {
        pins = elem->getOutputPins();
    }
    else {
        // Throw an exception if the terminal number is invalid
        std::cerr << "Invalid terminal number: " << terminal << std::endl;
        throw invalid_argument("Invalid terminal number.");
        exit(1);
    }

    //std::cout << "[Debug] Trying to connect " << elem->getElementSymbol()
    //    << " (pins=" << pins << ", terminal=" << terminal << ") to bus "
    //    << bus->getBusName() << " (bus pins=" << bus->getPinNumber() << ")\n";

    // Check if the number of pins matches between the element and the bus
    if (pins == bus->getPinNumber()) {
        // Add the element to the connections map for this bus
        connections[bus].push_back(elem);
        // Attach the bus to the element at the specified terminal
        elem->attachBus(bus, terminal);
        // Attach the element to the bus
        bus->attachElement(elem);  // Attach the element to the bus
        //std::cout << "[Debug] Successfully connected " << elem->getElementSymbol()
        //    << " to bus " << bus->getBusName() << std::endl;
    }
    else {
        std::cerr << "Connection failed: element pins = " << pins
            << ", bus pins = " << bus->getPinNumber() << std::endl;
        // Throw an exception if the number of pins does not match
        throw invalid_argument("Invalid connection, number of bus and element pins different.");
        exit(1);
    }
}

// Function to delete an element from the network
void Network::deleteElement(const std::string& designator) {
    if (elements.find(designator) != elements.end()) {
        elements.erase(designator);
    }
    else {
        throw std::invalid_argument("Element not found");
    }
}

// Function to delete a bus from the network
void Network::deleteBus(const std::string& busName) {
    if (buses.find(busName) != buses.end()) {
        // Decrement the total pin count
        pins -= buses[busName]->getPinNumber();
        buses.erase(busName);
    }
    else {
        throw std::invalid_argument("Bus not found");
    }
}

// Function to print the connections between elements and buses
void Network::printConnections() {
    for (const auto& connection : connections) {
        Bus* bus = connection.first;
        std::cout << "Bus " << bus->getBusName() << " has the following elements attached:\n";
        for (Element* elem : connection.second) {
            std::cout << "  - " << elem->getElementSymbol() << std::endl; // Use the getter method
        }
        std::cout << "Total number of pins is " << pins << std::endl;
    }
}

