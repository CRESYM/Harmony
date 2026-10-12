/**
 * @file Transformer_Delta_Delta.cpp
 * @brief Implementation of Delta-delta (Δ-Δ) classic transformer topology.
 */
#include "Transformer_Delta_Delta.h"

TransformerDeltaDelta::TransformerDeltaDelta(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values)
    : Transformer_classic(symbol, location, pins, values) {
    applyWindingConnection(true, true);
}

TransformerDeltaDelta::~TransformerDeltaDelta() = default;
