#include "Transformer_Delta_Delta.h"

// Constructor
TransformerDeltaDelta::TransformerDeltaDelta(const std::string& symbol, int pins, const std::vector<double>& values)
    : Transformer_classic(symbol, pins, values) {   
}

// Destructor
TransformerDeltaDelta::~TransformerDeltaDelta() {
    std::cout << "Transformer Delta-Delta object for " << getElementSymbol() << " destroyed." << std::endl;
}
