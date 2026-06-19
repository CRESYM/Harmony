#ifndef TRANSFORMER_REAL_H
#define TRANSFORMER_REAL_H

/**
 * @file Transformer_real.h
 * @brief Real transformer model with turns ratio and phase shift.
 */

#include "Transformer_base.h"

/**
 * @class Transformer_real
 * @brief Real transformer with turns ratio and phase lag.
 * @ingroup transformer
 */
class Transformer_real : public Transformer_base {
public:
    /**
     * @brief Construct a real transformer model.
     * @param symbol Element identifier.
     * @param location Network area or location string.
     * @param pins Number of pins (phases) per winding.
     * @param values Turns ratio, phase shift, and winding parameter vector.
     */
    Transformer_real(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values);

    ~Transformer_real() override; 

    double getTurnsRatio() const { return a; }

    double getPhaseLag() const { return phi; }  // Method to get phase lag

private:
    double a;  // Turns ratio
    double phi; // Phase shift
};

#endif // TRANSFORMER_REAL_H
