#ifndef TRANSFORMER_DELTA_DELTA_H
#define TRANSFORMER_DELTA_DELTA_H

#include "Transformer_classic.h"

class TransformerDeltaDelta : public Transformer_classic {
public:
    // Constructor to initialize the Transformer with a given symbol, number of pins, values (with phase lag),
    // mutual inductances, coupling coefficients, and mutual reactance for delta winding.
    TransformerDeltaDelta(const std::string& symbol, int pins, const std::vector<double>& values,
        const std::vector<double>& couplingCoefficients,
        double M_delta);

    ~TransformerDeltaDelta() override;




private:
    double phaseLag;  // Phase lag to incorporate in the Y parameters

    // New members for mutual inductances and coupling coefficients
    std::vector<double> couplingCoeff;  // Coupling coefficients for transformer windings
    double M_delta;  // Mutual reactance for the delta winding
};

#endif // TRANSFORMER_DELTA_DELTA_H
