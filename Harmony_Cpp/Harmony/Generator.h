#ifndef GENERATOR
#define GENERATOR

#include "Element.h"
#include <array>

class Generator : public Element {
public:

  //Single-phase
    Generator(const std::string& symbol, int inputPins, int outputPins);

    /*Generator(const std::string& symbol, int inputPins, int outputPins)
        : Element(symbol, inputPins, outputPins),
        R_f{ 1.0, 1.0, 1.0 },
        L_f{ 0.01, 0.01, 0.01 },
        X_d{ 1.0, 1.0, 1.0 },
        T_f{ 0.1, 0.1, 0.1 } {}*/

    ~Generator() {}

    //void compute_y_parameters(double frequency) override; // Correct virtual function override

private:
    double R_f = 1.0;  // Default value
    double L_f = 0.01; // Default value
    double X_d = 1.0;  // Default value
    double T_f = 0.1;  // Default value
    //std::array<double, 3> R_f = { 1.0, 1.0, 1.0 };   // Resistance values for three phases
    //std::array<double, 3> L_f = { 0.01, 0.01, 0.01 }; // Inductance values for three phases
    //std::array<double, 3> X_d = { 1.0, 1.0, 1.0 };    // Reactance values for three phases
    //std::array<double, 3> T_f = { 0.1, 0.1, 0.1 };    // Time constant values for three phases
};

#endif

