#ifndef TRANSFORMER_Y_Y_H
#define TRANSFORMER_Y_Y_H

#include "Transformer_classic.h"

class TransformerYY : public Transformer_classic {
public:
    // Constructor to initialize the Transformer with a given symbol, number of pins, values,
    // mutual inductances.
    TransformerYY(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values);

    ~TransformerYY();

private:
    // inherited from parent class
  };


#endif // TRANSFORMER_Y_Y_H
