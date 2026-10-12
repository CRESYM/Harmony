/**
 * @file Transformer_Delta_Delta_real.cpp
 * @brief Implementation of Delta-delta (Δ-Δ) real transformer topology.
 */
#include "Transformer_Delta_Delta_real.h"

TransformerDeltaDelta_real::TransformerDeltaDelta_real(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values)
    : Transformer_real(symbol, location, pins, values) {
    applyWindingConnection(true, true);
}

TransformerDeltaDelta_real::~TransformerDeltaDelta_real() = default;
