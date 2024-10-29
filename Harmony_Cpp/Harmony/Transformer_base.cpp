#include "Transformer_base.h"

Transformer_base::Transformer_base(const std::string& symbol, int pins, const std::vector<double>& values)
	: Element(symbol, pins, pins) {
}

// Destructor
Transformer_base::~Transformer_base() {
    std::cout << "Transformer object for " << getElementSymbol() << " destroyed." << std::endl;
}
