#ifndef TRANSFORMER_DELTA_DELTA_H
#define TRANSFORMER_DELTA_DELTA_H

#include "Transformer_classic.h"

class TransformerDeltaDelta : public Transformer_classic {
public:
    // Constructor to initialize the Transformer with a given symbol, number of pins, values (with phase lag),
    // mutual inductances, coupling coefficients, and mutual reactance for delta winding.
    TransformerDeltaDelta(const std::string& symbol, int pins, const std::vector<double>& values);

    ~TransformerDeltaDelta() override;

private:
    // inherited from the parent class
};

#endif // TRANSFORMER_DELTA_DELTA_H
