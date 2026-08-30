/**
 * @file Transformer_Delta_Y.cpp
 * @brief Implementation of Delta-wye (Δ-Y) classic transformer topology.
 */
#include "Transformer_Delta_Y.h"

TransformerDeltaY::TransformerDeltaY(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values)
    : Transformer_classic(symbol, location, pins, values) {
    applyWindingConnection(true, false);
}

TransformerDeltaY::~TransformerDeltaY() = default;
