#ifndef _Impedance_h_

#define _Impedance_h_

#include "Element.h"

class Impedance : public Element {
public:
    // Constructor for Impedance
    Impedance(const std::string& symbol, int inputPins, int outputPins)
        : Element(symbol, inputPins, outputPins) {}
    ~Impedance() {}
};

#endif


