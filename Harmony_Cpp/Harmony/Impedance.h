#ifndef _Impedance_h_

#define _Impedance_h_

#include "Element.h"

class Impedance : public Element {
public:
    double R = 5.0;  // Default resistance value
    double X = 10; // Default reactance value

    // Constructor for Impedance
    Impedance(const std::string& symbol, int inputPins, int outputPins)
        : Element(symbol, inputPins, outputPins) {} 

    ~Impedance() {}

    void compute_y_parameters(double frequency) override; // Declaration
};

#endif


