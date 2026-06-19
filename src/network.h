#ifndef NETWORK_H
#define NETWORK_H

#include "Constants.h"

class Bus;
class Element;
class SubNetwork;

// Type alias for a pair of element designator and pin name, used to map buses to connected elements in the network
using Net = std::unordered_map<Bus*, std::vector<Element*>>;

class Network {
protected:
    //StateSpaceModel state_space_model; 
    std::unordered_map<std::string, Bus*> buses; // Map of bus names to buses
    std::unordered_map<std::string, Element*> elements;  // Map of designators to elements
    std::unordered_map<Bus*, std::vector<Element*>> connections; // Connections between buses and elements

    int pins; // Total number of pins/phases in the network, used for equivalent admittance/impedance calculation

    // Names for identification
    std::vector<std::string> ac_grid_names;
    std::vector<std::string> dc_grid_names;

    // Core hierarchical system representation
    std::unordered_map<std::string, SubNetwork*> ac_grids;  // AC grids as subnetworks
    std::unordered_map<std::string, SubNetwork*> dc_grids;  // DC grids as subnetworks
    std::unordered_map<std::string, Element*> converters; // Converter subnetworks
    
public:

    // Constructor to initialize the network with zero pins
    Network();

    // Destructor to handle resource cleanup
    virtual ~Network();

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
    void print_summary() const;

    // Getters for private members variables for buses, elements, and connections
    const std::unordered_map<std::string, Bus*>& getBuses() const { return buses; }
    const std::unordered_map<std::string, Element*>& getElements() const { return elements; }
    std::unordered_map<std::string, Element*>& getElements() { return elements; }
    const std::unordered_map<Bus*, std::vector<Element*>>& getConnections() const { return connections; }
    std::unordered_map<Bus*, std::vector<Element*>>& getConnections() { return connections; }

    // Add AC and DC grids to the system
    void add_areas();
    void empty_areas() {
        for (auto& [name, sub] : ac_grids)
            delete sub;
        ac_grids.clear();
        ac_grid_names.clear();

        for (auto& [name, sub] : dc_grids)
            delete sub;
        dc_grids.clear();
        dc_grid_names.clear();

        converters.clear();
	}
    bool is_area_empty() {return ac_grids.empty() && dc_grids.empty(); }

    vector<string> get_ac_grid_names() { return ac_grid_names; };
    vector<string> get_dc_grid_names() { return dc_grid_names; };
    std::unordered_map<std::string, SubNetwork*>& get_ac_grids() { return ac_grids; }
    std::unordered_map<std::string, SubNetwork*>& get_dc_grids() { return dc_grids; }
    std::unordered_map<std::string, Element*>& get_converters() { return converters; }

};

#endif // NETWORK_H
