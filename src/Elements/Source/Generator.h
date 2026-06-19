#ifndef GENERATOR_H
#define GENERATOR_H

/**
 * @file Generator.h
 * @brief Synchronous generator source model.
 */

#include "Source_base.h"

/**
 * @class Generator
 * @brief Synchronous machine generator with default electrical parameters.
 * @ingroup source
 */
class Generator : public Source_base {
public:
    /**
     * @brief Construct a generator with voltage and parameter vector.
     * @param symbol Element identifier.
     * @param location Network area or location string.
     * @param pins Number of pins (phases).
     * @param V Voltage magnitude.
     * @param values Generator electrical parameters.
     */
    Generator(const std::string& symbol, const std::string& location, int pins, double V, const std::vector<double>& values);
 
    // Destructor
    ~Generator() {}


private:
    double R_f = 1.0;  // Default value
    double L_f = 0.01; // Default value
    double X_d = 1.0;  // Default value
    double T_f = 0.1;  // Default value
	double X_m = 1.0;  // Default value

};

#endif

