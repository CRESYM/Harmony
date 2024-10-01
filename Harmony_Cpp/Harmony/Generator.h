#ifndef GENERATOR
#define GENERATOR

#include "Element.h"
#include <array>

class Generator : public Element {
public:
    // Constructor
    Generator(const std::string& symbol, int pins, const std::vector<double>& values);

    // Destructor
    ~Generator() {}

private:
    double R_f = 1.0;  // Default value
    double L_f = 0.01; // Default value
    double X_d = 1.0;  // Default value
    double T_f = 0.1;  // Default value
};

#endif

