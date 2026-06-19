#ifndef TRANSFORMER_Y_DELTA_H
#define TRANSFORMER_Y_DELTA_H

/**
 * @file Transformer_Y_Delta.h
 * @brief Wye-delta (Y-Δ) classic transformer topology.
 */

#include "Transformer_classic.h"

/**
 * @class TransformerYDelta
 * @brief Wye-delta connected classic transformer.
 * @ingroup transformer
 */
class TransformerYDelta : public Transformer_classic {
public:
    /**
     * @brief Construct a Y-Δ classic transformer.
     * @param symbol Element identifier.
     * @param location Network area or location string.
     * @param pins Number of pins (phases) per winding.
     * @param values Winding parameters including phase lag and coupling coefficients.
     */
    TransformerYDelta(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values);

    ~TransformerYDelta() override;

private:
    // inherited from parent class
};

#endif // TRANSFORMER_Y_DELTA_H


