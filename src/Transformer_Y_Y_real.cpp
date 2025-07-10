#include "Transformer_Y_Y_real.h"

// Constructor
TransformerYY_real::TransformerYY_real(const std::string& symbol, int pins, const std::vector<double>& values)
    : Transformer_real(symbol, pins, values) {

}

// Destructor
TransformerYY_real::~TransformerYY_real() {
    std::cout << "Transformer Y-Y object for " << getElementSymbol() << " destroyed." << std::endl;
}
