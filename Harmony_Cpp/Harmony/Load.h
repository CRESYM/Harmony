#ifndef LOAD
#define LOAD

#include "Element.h"


class Load : public Element {
public:
    Load(const std::string& symbol, int inputPins, int outputPins)
        : Element(symbol, inputPins, outputPins) {}

    ~Load() {}

    void compute_y_parameters(double frequency) override; // Declaration
private:
    double R = 10.0;
    double L = 0.01;
    double C = 0.001;
};

#endif