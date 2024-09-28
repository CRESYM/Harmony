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
#include <SymEngine/Matrix.h> // Ensure you include the necessary SymEngine headers

using namespace SymEngine; // Use the SymEngine namespace

class Element {
public:
     // Unified constructor for universal number of phases
    Element(const std::string& symbol, int inputPins, int outputPins)
        : element_symbol(symbol), input_pins(inputPins), output_pins(outputPins) {
        Y_matrix = createZeroMatrix(inputPins, outputPins);
    }

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
    }

    // Virtual function to compute Y-parameters (to be implemented by derived classes)
    virtual void compute_y_parameters(double frequency) = 0;

    virtual void printElementValues()=0;

    DenseMatrix read_y_matrix() { return Y_matrix; }

protected:
    std::string element_symbol;
    int input_pins;
    int output_pins;

    DenseMatrix Y_matrix; // Matrix representation of admittance

    // Static helper function to create a zero matrix
    DenseMatrix createZeroMatrix(int size1, int size2) {
        DenseMatrix zeroMatrix(size1, size2);
        for (int i = 0; i < size1; ++i) {
            for (int j = 0; j < size2; ++j) {
                zeroMatrix.set(i, j, zero); // Use SymEngine's symbolic `zero`
            }
        }
        return zeroMatrix;
    }
};

#endif // ELEMENT_H
