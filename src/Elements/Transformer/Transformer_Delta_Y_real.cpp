/**
 * @file Transformer_Delta_Y_real.cpp
 * @brief Implementation of Delta-wye (Δ-Y) real transformer topology.
 */
#include "Transformer_Delta_Y_real.h"

TransformerDeltaY_real::TransformerDeltaY_real(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values)
    : Transformer_real(symbol, location, pins, values) {
    applyWindingConnection(true, false);
}

TransformerDeltaY_real::~TransformerDeltaY_real() = default;
