#ifndef ELEMENT_H
#define ELEMENT_H

#include "Constants.h"

// Declarations 
class Bus;

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
    std::vector<Bus*> getBuses();
    Bus* getOtherBus(Bus*);

    // Attach bus pointer to the proper terminal
    void attachBus(Bus*, int);

    // Print element information
    void printElementInfo() const {
        std::cout << "Element Symbol: " << element_symbol
            << ", Input Pins: " << input_pins
            << ", Output Pins: " << output_pins << std::endl;

        // Print the Y-parameters
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                double Y_val_abs = eval_double(*abs(Y_matrix.get(i, j)));
                std::cout << "| Y" << (i + 1) << (j + 1) << "|: " << Y_val_abs << " S" << std::endl;
            }
        }
    }

    // Virtual function to compute Y-parameters (to be implemented by derived classes)
    virtual void compute_y_parameters(double frequency) = 0;
    virtual DenseMatrix compute_y_parameters() { return Y_matrix; };

    virtual void printElementValues()=0;

    DenseMatrix read_y_matrix() { return Y_matrix; }

protected:
    std::string element_symbol;
    int input_pins;
    int output_pins;
    std::map<int, Bus*> connections;

    DenseMatrix Y_matrix; // Matrix representation of admittance
};

#endif // ELEMENT_H
