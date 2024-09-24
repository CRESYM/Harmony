#ifndef ELEMENT_H
#define ELEMENT_H

#include <string>
#include <iostream>



class Element {
public:
    std::string element_symbol;
    int input_pins;
    int output_pins;
    void* element_value;  // Placeholder for the element definition

    // Parameterized constructor
    Element(const std::string& symbol, int inputPins, int outputPins)
        : element_symbol(symbol), input_pins(inputPins), output_pins(outputPins), element_value(nullptr) {}

    // Destructor
    virtual ~Element();

    // Getters
    int getInputPins() const { return input_pins; }
    int getOutputPins() const { return output_pins; }

    // Print element information
    void printElementInfo() const {
        std::cout << "Element Symbol: " << element_symbol
            << ", Input Pins: " << input_pins
            << ", Output Pins: " << output_pins << std::endl;
    }

    // Virtual function to compute Y-parameters
    virtual void compute_y_parameters(double R_f, double L_f, double X_d, double T_f, double frequency);
};

#endif // ELEMENT_H