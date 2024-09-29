#include "network.h"
#include "Element.h" 
#include "Bus.h"
#include <algorithm>
#include <stdexcept>

Network::~Network() {
    for (std::unordered_map<std::string, Bus*>::iterator i = buses.begin(); i != buses.end(); i++)
        delete i->second;
    buses.clear();
    for (std::unordered_map<std::string, Element*>::iterator i = elements.begin(); i != elements.end(); i++)
        delete i->second;
    elements.clear();
    connections.clear();
}

// Function to add a bus to the network
void Network::addBus(Bus* bus) {
    buses[bus->getBusName()] = bus;
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
    connections[bus].push_back(elem);
    elem->attachBus(bus, terminal);
    bus->attachElement(elem);  // Attach the element to the bus
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

void Network::compute_equivalent_impedance(std::vector<Bus*> start_buses, std::vector<Bus*> end_buses, std::vector<Element*> skip_elements) {
    if (start_buses.empty())
    {
        throw std::invalid_argument("There is no start buses.");
        exit(1);
    }

    // erase duplicates in start and end buses, remove gnd from the list of buses
    sort(start_buses.begin(), start_buses.end()); start_buses.erase(unique(start_buses.begin(), start_buses.end()), start_buses.end());
    sort(end_buses.begin(), end_buses.end()); end_buses.erase(unique(end_buses.begin(), end_buses.end()), end_buses.end());
    // Remove the element using erase function and iterators
    for (int i = 0; i < start_buses.capacity(); i++) {
        if (start_buses[i]->getBusName() == "gnd") { // leave the function
            throw std::invalid_argument("Ground cannot be a start bus.");
            exit(1);
        }        
    }

    // check bus positions in matrix, add only buses which are not gnd
    int pos = 0; 
    std::vector<int> positions_currents;
    std::unordered_map<Bus*, int> all_buses;
    for (auto& bus : start_buses) {
        all_buses[bus] = pos; pos += bus->getPinNumber();
        positions_currents.push_back(pos);
        pos += bus->getPinNumber();
    }
    for (auto& bus : end_buses) {
        if (bus->getBusName() != "gnd") {
            all_buses[bus] = pos; pos += bus->getPinNumber();
            positions_currents.push_back(pos);
            pos += bus->getPinNumber();
        }
    }
    for (const auto& bus : buses) {
        if (std::find(start_buses.begin(), start_buses.end(), bus.second) == start_buses.end()
            && std::find(end_buses.begin(), end_buses.end(), bus.second) == end_buses.end()) {
            if (bus.second->getBusName() != "gnd") {
                all_buses[bus.second] = pos;
                pos += bus.second->getPinNumber();
            }
        }
    }


    // make the equivalent matrix as zero matrix
    DenseMatrix Y = createZeroMatrix(pos, pos + 1);

    // Make MNA with excuded elements
    // Go through buses (connections) and add element Y parameters 
    // if the element should not be skipped.
    // Order for writing equations input voltage and current,
    // then output voltage and current, and then other voltages.
    for (auto& bus : all_buses) {
        for (auto& element : connections[bus.first]) {
            DenseMatrix element_Y_matrix = element->compute_y_parameters();

            // add element_Y_matrix
            int pins = bus.first->getPinNumber();
            int position = bus.second;
            for (int i = 0; i < pins; i++) {
                for (int j = 0; j < pins; j++) {
                    Y.set(position + i, position + j, add(Y.get(position + i, position + j),
                        element_Y_matrix.get(i, j)));
                }
            }

            // get the other bus and add -element_Y_matrix
            Bus* other_bus = element->getOtherBus(bus.first);
            if (other_bus->getBusName() != "gnd") {
                int pins = other_bus->getPinNumber();
                int position = all_buses[other_bus];
                for (int i = 0; i < pins; i++) {
                    for (int j = 0; j < pins; j++) {
                        Y.set(position + i, position + j, sub(Y.get(position + i, position + j),
                            element_Y_matrix.get(i, j)));
                    }
                }
            }

            // check if there is current associated to bus
        }
    }

    // reduced_row_echelon_form

}
    


    