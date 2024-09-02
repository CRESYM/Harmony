// element.cpp

#include "element.h"
#include <iostream>
#include <stdexcept>

// Default constructor
Element::Element()
    : element_symbol(""), input_pins(0), output_pins(0), element_value(nullptr) {
    // Initialize default state
}

Element::Element(const std::string& symbol, int input_pins, int output_pins)
    : element_symbol(symbol), input_pins(input_pins), output_pins(output_pins), element_value(nullptr) {
    // Define pins based on input and output pin counts
    for (int i = 1; i <= input_pins; ++i) {
        pins.push_back("1." + std::to_string(i));
    }
    for (int i = 1; i <= output_pins; ++i) {
        pins.push_back("2." + std::to_string(i));
    }
}

// New constructor that takes an unordered_map
Element::Element(const std::unordered_map<std::string, std::string>& args)
    : element_symbol(""), input_pins(0), output_pins(0), element_value(nullptr) {
    try {
        element_symbol = args.at("symbol");  // Retrieve and set the symbol
        input_pins = std::stoi(args.at("input_pins"));  // Convert input pins count from string to int
        output_pins = std::stoi(args.at("output_pins"));  // Convert output pins count from string to int
    }
    catch (const std::out_of_range& e) {
        std::cerr << "Error: Missing required argument: " << e.what() << std::endl;
        throw;
    }
    catch (const std::invalid_argument& e) {
        std::cerr << "Error: Invalid argument provided: " << e.what() << std::endl;
        throw;
    }

    // Define pins based on input and output pin counts
    for (int i = 1; i <= input_pins; ++i) {
        pins.push_back("1." + std::to_string(i));
    }
    for (int i = 1; i <= output_pins; ++i) {
        pins.push_back("2." + std::to_string(i));
    }
}

Element::~Element() {
    // Clean up resources if needed
    // For example, if element_value was dynamically allocated, free it here
}

int Element::getInputPins() const {
    return input_pins;
}

int Element::getOutputPins() const {
    return output_pins;
}

int Element::getTotalPins() const {
    return input_pins + output_pins;
}

void Element::addElementValue(const std::string& key, void* value) {
    if (key == "element_value") {
        element_value = value;
    }
    // Add handling for other keys if necessary
}

std::vector<std::string> Element::getNodes() const {
    return pins;
}

std::vector<std::string> Element::getNodesByPin(const std::string& pin) const {
    std::vector<std::string> filtered_pins;
    for (const auto& p : pins) {
        if (p != pin) {
            filtered_pins.push_back(p);
        }
    }
    return filtered_pins;
}

void Element::printElementInfo() const {
    std::cout << "Element Symbol: " << element_symbol << std::endl;
    std::cout << "Number of Input Pins: " << input_pins << std::endl;
    std::cout << "Number of Output Pins: " << output_pins << std::endl;
    std::cout << "Total Number of Pins: " << getTotalPins() << std::endl;

    std::cout << "All Nodes (Pins):" << std::endl;
    for (const auto& pin : pins) {
        std::cout << "  " << pin << std::endl;
    }
}

