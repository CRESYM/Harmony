#ifndef ELEMENT_H
#define ELEMENT_H

#include <string>
#include <unordered_map>
#include <stdexcept>
#include <functional> // Include the functional header for std::hash


class Element {
public:
    std::string symbol;
    std::vector<std::string> pins;
    int input_pins;
    int output_pins;
    void* element_value;  // Placeholder for the element definition

	// Default constructor
	// Default constructor
//public:
    // Default constructor
    Element();

    // Parameterized constructor
    Element(const std::string& symbol, int input_pins, int output_pins);

    // New constructor that takes an unordered_map
    Element(const std::unordered_map<std::string, std::string>& args);

    // Destructor
    ~Element();

    // Getters
    int getInputPins() const;
    int getOutputPins() const;
    int getTotalPins() const;

    // Add element value
    void addElementValue(const std::string& key, void* value);

    // Get nodes (all pins)
    std::vector<std::string> getNodes() const;

    // Get nodes excluding a specific pin
    std::vector<std::string> getNodesByPin(const std::string& pin) const;

    // Print element information
    void printElementInfo() const;
};

#endif // ELEMENT_H