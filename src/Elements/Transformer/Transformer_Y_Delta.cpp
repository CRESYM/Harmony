/**
 * @file Transformer_Y_Delta.cpp
 * @brief Implementation of Wye-delta (Y-Δ) classic transformer topology.
 */
#include "Transformer_Y_Delta.h"

TransformerYDelta::TransformerYDelta(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values)
    : Transformer_classic(symbol, location, pins, values) {
    applyWindingConnection(false, true);
}

TransformerYDelta::~TransformerYDelta() = default;
