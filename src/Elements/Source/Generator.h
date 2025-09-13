#ifndef GENERATOR_H
#define GENERATOR_H

#include "Source_base.h"

class Generator : public Source_base {
public:
    // Constructor
    Generator(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values);
 
    // Destructor
    ~Generator() {}


private:
    double R_f = 1.0;  // Default value
    double L_f = 0.01; // Default value
    double X_d = 1.0;  // Default value
    double T_f = 0.1;  // Default value

};

#endif

