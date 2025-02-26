#include "Transformer_Y_Y.h"

// Constructor
TransformerYY::TransformerYY(const std::string& symbol, int pins, const std::vector<double>& values)
    : Transformer_classic(symbol, pins, values) {

}

// Destructor
TransformerYY::~TransformerYY() {
    std::cout << "Transformer Y-Y object for " << getElementSymbol() << " destroyed." << std::endl;
}
