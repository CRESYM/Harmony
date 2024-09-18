#ifndef NETWORK_H
#define NETWORK_H

#include <unordered_map>
#include <vector>
#include <tuple>
#include <string>
#include <stdexcept>
#include <iostream>

// Type alias for a pair of element designator and pin name
using Net = std::vector<std::tuple<std::string, std::string>>;

class Network {
public:
	std::unordered_map<std::string, std::vector<std::string>> connections;

	// Constructor
	Network() {}

    // Function to add a node (element) to the network
    void addNode(const std::string& nodeName);

    // Function to connect two elements in the network
    void connect(const std::string& source, const std::string& destination);

    // Overloaded function to connect multiple elements to a single destination
    void connect(const std::vector<std::string>& sources, const std::string& destination);

    // Function to add an element (with an Element object) and get a unique designator
    std::string addElement(Element& elem);

    // Function to add an element by a specified designator
    void addElement(const std::string& designator, Element& elem);

    // Function to delete an element from the network
    void deleteElement(const std::string& designator);

    // Function to add a pin to the network
    void addPin(const std::tuple<std::string, std::string>& pin);

    // Overloaded function to add a pin to a specific net
    void addPin(const std::tuple<std::string, std::string>& pin, const std::string& netName);

    // Function to get the net name by a designator
    std::string getNetName(const std::string& name) const;

    // Function to get the net name for a pin
    std::string getNetName(const std::tuple<std::string, std::string>& pin) const;

    // Function to connect all elements' pins with their nets
    void connectPins();

private:
    std::unordered_map<std::string, Element> elements;  // Map of designators to elements
    std::unordered_map<std::string, Net> nets;          // Map of net names to nets (pins)
    std::unordered_map<std::string, std::vector<std::string>> connections;  // Node connections

    // Helper function to find the net for a given pin
    Net& netFor(const std::tuple<std::string, std::string>& pin);

    // Helper function to delete nets associated with a designator
    void deleteNet(const std::string& designator);
};

#endif // NETWORK_H
