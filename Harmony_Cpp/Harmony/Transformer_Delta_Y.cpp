#include "Transformer_Delta_Y.h"

// Constructor
TransformerDeltaY::TransformerDeltaY(const std::string& symbol, int pins, const std::vector<double>& values)
    : Transformer_classic(symbol, pins, values) {
}

// Destructor
TransformerDeltaY::~TransformerDeltaY() {
    std::cout << "Transformer Delta Y object for " << getElementSymbol() << " destroyed." << std::endl;
}

