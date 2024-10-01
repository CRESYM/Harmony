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
        Y_matrix = createZeroMatrix(2*inputPins, 2*outputPins);
    }

    // Destructor
    virtual ~Element();

    // Getters
    int getInputPins() const { return input_pins; }
    int getOutputPins() const { return output_pins; }
    std::string getElementSymbol() const { return element_symbol; }
    std::vector<Bus*> getBuses();
    Bus* getOtherBus(Bus*);
    std::map<Bus*, int> getConnections() { return connections; }

    // Attach bus pointer to the proper terminal
    void attachBus(Bus*, int);

    // Print element information
    void printElementInfo() const {
        std::cout << "Element Symbol: " << element_symbol
            << ", Input Pins: " << input_pins
            << ", Output Pins: " << output_pins << std::endl;
    }

    // Print all element values, addition to printElementInfo
    virtual void printElementValues();

    // Print Y matrix in file
    void writeFile(double start_frequency, int end_frequency, int number_of_points);

    // Virtual function to compute Y-parameters (to be implemented by derived classes)
    virtual std::vector<std::vector<complex<double>>> compute_y_parameters(double frequency);
    virtual DenseMatrix compute_y_parameters() { return Y_matrix; };

    

    DenseMatrix read_y_matrix() { return Y_matrix; }

protected:
    std::string element_symbol;
    int input_pins;
    int output_pins;
    std::map<Bus* , int> connections;

    DenseMatrix Y_matrix; // Matrix representation of admittance
};

#endif // ELEMENT_H
