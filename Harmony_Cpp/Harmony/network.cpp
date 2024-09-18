#include "network.h"
#include "Element.h" // Assuming there is a file for the Element class
#include <algorithm>
#include <stdexcept>

// Constructor for Network class
Network::Network() {
    elements = std::unordered_map<std::string, Element>();
    nets = std::unordered_map<std::string, Net>();
    connections = std::unordered_map<std::string, std::vector<std::string>>();
}

// Add a node (element) to the network
void Network::addNode(const std::string& nodeName) {
    connections[nodeName] = std::vector<std::string>();
}

// Connect two nodes in the network
void Network::connect(const std::string& source, const std::string& destination) {
    connections[source].push_back(destination);
}

// Connect multiple nodes to a destination
void Network::connect(const std::vector<std::string>& sources, const std::string& destination) {
    for (const auto& source : sources) {
        connect(source, destination);
    }
}


// Add a pin to a specified net
void Network::addPin(const std::tuple<std::string, std::string>& pin, const std::string& netName) {
    nets[netName].push_back(pin);
}

// Get the net name for a designator (element or net)
std::string Network::getNetName(const std::string& name) const {
    if (nets.find(name) != nets.end()) {
        return name;
    }
    else {
        throw std::invalid_argument("Unknown net name: " + name);
    }
}
