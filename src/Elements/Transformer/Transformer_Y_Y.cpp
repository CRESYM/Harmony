/**
 * @file Transformer_Y_Y.cpp
 * @brief Implementation of Wye-wye (Y-Y) classic transformer topology.
 */
#include "Transformer_Y_Y.h"

// Constructor
TransformerYY::TransformerYY(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values)
    : Transformer_classic(symbol, location, pins, values) {

}

// Destructor
TransformerYY::~TransformerYY() = default;
