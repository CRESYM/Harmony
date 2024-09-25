#ifndef GENERATOR
#define GENERATOR

#include "Element.h"

class Generator : public Element {
public:

  //  Generator(const std::string& symbol, int inputPins, int outputPins)
    //    : Element(symbol, inputPins, outputPins) {}

    Generator(const std::string& symbol, int inputPins, int outputPins)
        : Element(symbol, inputPins, outputPins) {}

    ~Generator() {}

    void compute_y_parameters(double frequency) override; // Declaration
private:
    double R_f = 1.0;  // Default value
    double L_f = 0.01; // Default value
    double X_d = 1.0;  // Default value
    double T_f = 0.1;  // Default value
};

#endif

