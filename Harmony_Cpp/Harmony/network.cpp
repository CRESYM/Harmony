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

    // erase duplicates in start and end buses
    sort(start_buses.begin(), start_buses.end());
    start_buses.erase(unique(start_buses.begin(), start_buses.end()), start_buses.end());

    sort(end_buses.begin(), end_buses.end());
    end_buses.erase(unique(end_buses.begin(), end_buses.end()), end_buses.end());

    // add other buses
    std::vector<Bus*> other_buses;
    for (const auto& bus : buses) {
        if(std::find(start_buses.begin(), start_buses.end(), bus.second) == start_buses.end()
            && std::find(end_buses.begin(), end_buses.end(), bus.second) == end_buses.end())
            other_buses.push_back(bus.second);
    }

    // check bus positions in matrix
    int pos = 0;
    int pos_current = 0;
    std::vector<int> positions;
    for (auto& bus : start_buses) {
        positions.push_back(pos);
        pos += bus->getPinNumber();
        pos_current += bus->getPinNumber();
    }
    pos += pos_current; pos_current = 0;
    for (auto& bus : end_buses) {
        positions.push_back(pos);
        pos += bus->getPinNumber();
        pos_current += bus->getPinNumber();
    }
    pos += pos_current;
    for (auto& bus : other_buses) {
        positions.push_back(pos);
        pos += bus->getPinNumber();
    }

    // check the equivalent matrix size
    int size = pins;
    for (auto& bus : start_buses) {
        size += bus->getPinNumber(); // adding equations for input currents
    }
    for (auto& bus : end_buses) {
        size += bus->getPinNumber(); // adding equations for input currents
    }
    DenseMatrix Y = DenseMatrix(size, size+1);
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size + 1; j++)
            Y.set(i, j, zero);

    // Make MNA with excuded elements
    // Go through buses (connections) and add element Y parameters 
    // if the element should not be skipped.
    // Order for writing equations input voltage and current,
    // then output voltage and current, and then other voltages.
    for (auto& bus : start_buses) {
        int pins = bus->getPinNumber();
        for (auto& element : connections[bus]) {
            DenseMatrix element_Y_matrix = element->compute_y_parameters();

            for (int i = position; i < position + pins; i++)
                for (int j = position; j < position + pins; j++)
                    continue;
        }
    }

    // reduced_row_echelon_form

}
    


    