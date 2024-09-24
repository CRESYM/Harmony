#include "network.h"
//#include "Element.h" 
//#include "Bus.h"
#include <algorithm>
#include <stdexcept>

// Function to add a bus to the network
void Network::addBus(const std::string& busName, Bus* bus) {
    buses[busName] = bus;
}

// Function to add an element to the network
void Network::addElement(const std::string& designator, Element* elem) {
    elements[designator] = elem;
}

// Function to connect an element to a bus
void Network::connectElementToBus(Element* elem, Bus* bus) {
    connections[bus].push_back(elem);
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
        std::cout << "Bus " << bus->busName << " has the following elements attached:\n";
        for (Element* elem : connection.second) {
            std::cout << "  - " << elem->getElementSymbol() << std::endl; // Use the getter method
            //std::cout << "  - " << elem->element_symbol << std::endl;
        }
    }
}

/*Net& Network::netFor(std::string start_buses, std::string end_buses, Element* elements_to_skip) {
    for (auto& start_bus : start_buses) {
        if (std::find(end_buses.begin(), end_buses.end(), start_bus) == end_buses.end())
            continue;
        Bus* bus = buses[start_bus];

        //for node_name in start_pins
        //    if occursin("gnd", string(node_name))
        /*        if !in(node_name, dict[:output_list])
                    push!(dict[:output_list], node_name)
                    continue
                    end
                else
                    # add nodes to the node list
                    !in(node_name, dict[:node_list]) && push!(dict[:node_list], node_name)
                    end

                    # find all elements inside the port connected to the node
                    elements_pins = filter(p ->  !in(p[1], elim_elements) && !in(p[1], dict[:element_list]), node)

                    for (element, pin) in elements_pins
                        push!(dict[:element_list], element) # add element's symbol to the list
                        other_nodes = get_nodes(net.elements[element], pin) # get the pins from the other side of element
                        make_lists(net, dict, elim_elements, other_nodes, end_pins)
                        end
                        end

    }
    
}*/



//void Network::compute_equivalent_admittance(std::string start_bus, std::string end_bus) {
//};
