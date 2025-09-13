#ifndef TRANSFORMER_Y_Y_REAL_H
#define TRANSFORMER_Y_Y_REAL_H

#include "Transformer_real.h"

class TransformerYY_real : public Transformer_real {
public:
    // Constructor to initialize the Transformer with a given symbol, number of pins, values,
    // mutual inductances.
    TransformerYY_real(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values);

    ~TransformerYY_real();

private:
    // inherited from parent class
};


#endif // TRANSFORMER_Y_Y_REAL_H
