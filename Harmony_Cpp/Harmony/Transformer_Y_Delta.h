#ifndef TRANSFORMER_Y_DELTA_H
#define TRANSFORMER_Y_DELTA_H

#include "Transformer_classic.h"

class TransformerYDelta : public Transformer_classic {
public:
    // Constructor to initialize the Transformer with a given symbol, number of pins, values (including phase lag), mutual reactances, and coupling coefficients
    TransformerYDelta(const std::string& symbol, int pins, const std::vector<double>& values);

    ~TransformerYDelta() override;

private:
    // inherited from parent class
};

#endif // TRANSFORMER_Y_DELTA_H


