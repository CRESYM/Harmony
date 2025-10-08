#ifndef NETWORK_H
#define NETWORK_H

#include "Constants.h"

class Bus;
class Element;

// Type alias for a pair of element designator and pin name, used to map buses to connected elements in the network
using Net = std::unordered_map<Bus*, std::vector<Element*>>;

class Network {
protected:
    //StateSpaceModel state_space_model; 
    std::unordered_map<std::string, Bus*> buses; // Map of bus names to buses
    std::unordered_map<std::string, Element*> elements;  // Map of designators to elements
    std::unordered_map<Bus*, std::vector<Element*>> connections; // Connections between buses and elements

    int pins; // Total number of pins/phases in the network, used for equivalent admittance/impedance calculation
    
    
public:

    // Constructor to initialize the network with zero pins
    Network();

    // Destructor to handle resource cleanup
    ~Network();

    // Function to connect two elements in the network
    //void connect(const std::string& source, const std::string& destination);

    // Overloaded function to connect multiple elements to a single destination
    //void connect(const std::vector<std::string>& sources, const std::string& destination);

    // Function to add a bus to the network using a bus object
    void addBus(Bus* bus);

    // Function to add a bus to the network by providing a bus name and bus object
    void addBus(const std::string& busName, Bus* bus);

    // Function to add an element (with an Element object) and get a unique designator
    void addElement(Element* elem);

    // Function to add an element to the network using a specific designator
    void addElement(const std::string& designator, Element* elem);

    // Function to connect an element to a specific bus at a particular terminal
    void connectElementToBus(Element* elem, int terminal, Bus* bus);

    // Function to delete an element from the network based on its designator
    void deleteElement(const std::string& designator);

    // Function to delete a bus from the network based on its name
    void deleteBus(const std::string& busName);

    // Print all the current connections between buses and elements in the network
    void printConnections();
	void printBuses();
    void printElements();

    // Getters for private members variables for buses, elements, and connections
    const std::unordered_map<std::string, Bus*>& getBuses() const { return buses; }
    const std::unordered_map<std::string, Element*>& getElements() const { return elements; }
    const std::unordered_map<Bus*, std::vector<Element*>>& getConnections() const { return connections; }

};

#endif // NETWORK_H
