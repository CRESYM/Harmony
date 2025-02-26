#ifndef TRANSFORMER_DELTA_Y_H
#define TRANSFORMER_DELTA_Y_H

#include "Transformer_classic.h"

class TransformerDeltaY : public Transformer_classic {
public:
    // Constructor to initialize the Transformer with a given symbol, number of pins, values,
    // mutual inductances, and coupling coefficients.
    TransformerDeltaY(const std::string& symbol, int pins, const std::vector<double>& values);

    ~TransformerDeltaY() override;


private:
    // inherited from parent class
};

#endif // TRANSFORMER_DELTA_Y_H
