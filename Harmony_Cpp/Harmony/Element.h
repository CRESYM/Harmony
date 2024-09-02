#ifndef ELEMENT_H
#define ELEMENT_H

#include <string>
#include <unordered_map>
#include <stdexcept>
#include <functional> // Include the functional header for std::hash

#include <symengine/expression.h>
#include <symengine/symbol.h>
#include <symengine/add.h>
#include <symengine/mul.h>
#include <symengine/pow.h>
#include <symengine/real_double.h>
#include <symengine/eval.h>
#include <symengine/functions.h>
#include <symengine/complex.h>
#include <symengine/complex_double.h>


class Element {
public:
    std::string element_symbol;
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