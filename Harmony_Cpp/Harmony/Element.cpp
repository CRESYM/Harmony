// element.cpp

#include "element.h"
#include "Constants.h"

#include <symengine/expression.h>
#include <symengine/real_double.h>
#include <symengine/eval.h>
#include <symengine/complex_double.h>
#include <symengine/functions.h>
#include <iostream>

using namespace SymEngine;

// Destructor definition
Element::~Element() {
    // If you need to clean up resources, do it here.
    // Currently, there's nothing to clean up, but it’s a good practice to define it.
}

/*// Default constructor
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
}*/
void Element::compute_y_parameters(double R_f, double L_f, double X_d, double T_f, double frequency) {
    std::cout << "Computing Y-parameters for element: " << element_symbol << std::endl;

    RCP<const Basic> omega = mul(real_double(2), mul(PI, real_double(frequency)));
    RCP<const Basic> s = mul(I, omega);

    RCP<const Basic> R_f_val = real_double(R_f);
    RCP<const Basic> L_f_val = real_double(L_f);
    RCP<const Basic> Z_f = add(R_f_val, mul(s, L_f_val));
    RCP<const Basic> Y_f = div(real_double(1), Z_f);

    RCP<const Basic> X_d_val = real_double(X_d);
    RCP<const Basic> Z_d = mul(I, X_d_val);
    RCP<const Basic> T_f_val = real_double(T_f);
    RCP<const Basic> H_f = div(real_double(1), add(mul(T_f_val, s), real_double(1)));

    RCP<const Basic> Y11 = Y_f;
    RCP<const Basic> Y12 = neg(div(H_f, Z_d));
    RCP<const Basic> Y21 = real_double(0);
    RCP<const Basic> Y22 = neg(div(real_double(1), Z_d));

    std::cout << "|Element Y11|: " << eval_double(*abs(Y11)) << " S" << std::endl;
    std::cout << "|Element Y12|: " << eval_double(*abs(Y12)) << " S" << std::endl;
    std::cout << "|Element Y21|: " << eval_double(*abs(Y21)) << " S" << std::endl;
    std::cout << "|Element Y22|: " << eval_double(*abs(Y22)) << " S" << std::endl;
}