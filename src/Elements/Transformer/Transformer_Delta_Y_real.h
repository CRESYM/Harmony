#ifndef TRANSFORMER_DELTA_Y_REAL_H
#define TRANSFORMER_DELTA_Y_REAL_H

#include "Transformer_real.h"

class TransformerDeltaY_real : public Transformer_real {
public:
    // Constructor to initialize the Transformer with a given symbol, number of pins, values,
    // mutual inductances, and coupling coefficients.
    TransformerDeltaY_real(const std::string& symbol, int pins, const std::vector<double>& values);

    ~TransformerDeltaY_real() override;


private:
    // inherited from parent class
};

#endif // TRANSFORMER_DELTA_Y_H
#pragma once
