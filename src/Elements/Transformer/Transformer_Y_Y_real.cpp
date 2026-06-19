/**
 * @file Transformer_Y_Y_real.cpp
 * @brief Implementation of Wye-wye (Y-Y) real transformer topology.
 */
#include "Transformer_Y_Y_real.h"

// Constructor
TransformerYY_real::TransformerYY_real(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values)
    : Transformer_real(symbol, location, pins, values) {

	//std::cout << Y_matrix.ncols() << " columns, " << Y_matrix.nrows() << " rows." << std::endl;
}

// Destructor
TransformerYY_real::~TransformerYY_real() {
    std::cout << "Transformer Y-Y object for " << getElementSymbol() << " destroyed." << std::endl;
}
