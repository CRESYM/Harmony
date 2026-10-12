/**
 * @file Transformer_Y_Delta_real.cpp
 * @brief Implementation of Wye-delta (Y-Δ) real transformer topology.
 */
#include "Transformer_Y_Delta_real.h"

TransformerYDelta_real::TransformerYDelta_real(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values)
    : Transformer_real(symbol, location, pins, values) {
    applyWindingConnection(false, true);
}

TransformerYDelta_real::~TransformerYDelta_real() = default;
