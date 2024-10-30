#ifndef TRANSFORMER_REAL_H
#define TRANSFORMER_REAL_H

#include "Transformer_base.h"


class Transformer_real : public Transformer_base {
public:
    // Constructor to initialize the Transformer with a given symbol, number of pins, and values
    Transformer_real(const std::string& symbol, int pins, const std::vector<double>& values);

    ~Transformer_real() override; 

    double getTurnsRatio() const { return a; }

    double getPhaseLag() const { return phi; }  // Method to get phase lag

private:
    double a;  // Turns ratio
    double phi; // Phase shift
};

#endif // TRANSFORMER_REAL_H
