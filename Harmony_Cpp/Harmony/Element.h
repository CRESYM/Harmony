#ifndef ELEMENT_H
#define ELEMENT_H

#include <string>
#include <iostream>
#include <vector> // For handling matrices in multi-phase systems

#include <symengine/symbol.h>
#include <symengine/add.h>
#include <symengine/mul.h>
#include <symengine/pow.h>
#include <symengine/real_double.h>
#include <symengine/eval.h>
#include <symengine/functions.h>
#include <symengine/complex.h>
#include <symengine/complex_double.h>
#include <symengine/expression.h>
#include <symengine/symengine_config.h>

class Element {
public:
    // Constructor for single-phase systems
    Element(const std::string& symbol, int inputPins, int outputPins)
        : element_symbol(symbol), input_pins(inputPins), output_pins(outputPins), is_three_phase(false) {}

    // Constructor for three-phase systems (3x3 impedance/admittance matrix)
    Element(const std::string& symbol, int inputPins, int outputPins, const std::vector<std::vector<double>>& matrix)
        : element_symbol(symbol), input_pins(inputPins), output_pins(outputPins), impedance_matrix(matrix), is_three_phase(true) {}

    // Destructor
    virtual ~Element();

    // Getters
    int getInputPins() const { return input_pins; }
    int getOutputPins() const { return output_pins; }
    std::string getElementSymbol() const { return element_symbol; }

    // Print element information
    void printElementInfo() const {
        std::cout << "Element Symbol: " << element_symbol
            << ", Input Pins: " << input_pins
            << ", Output Pins: " << output_pins << std::endl;

        if (is_three_phase) {
            std::cout << "Three-phase Impedance/Admittance Matrix: \n";
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    std::cout << impedance_matrix[i][j] << " ";
                }
                std::cout << std::endl;
            }
        }
    }

    // Virtual function to compute Y-parameters (to be implemented by derived classes)
    virtual void compute_y_parameters(double frequency) = 0;

    virtual void printElementValues();

protected:
    std::string element_symbol;
    int input_pins;
    int output_pins;

    // For three-phase systems
    std::vector<std::vector<double>> impedance_matrix; // 3x3 matrix for three-phase impedance/admittance
    bool is_three_phase;  // Flag to determine if this is a three-phase system

    //void* element_value;  // Placeholder for single-phase element values (could be removed)
};

#endif // ELEMENT_H
