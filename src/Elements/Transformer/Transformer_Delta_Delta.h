#ifndef TRANSFORMER_DELTA_DELTA_H
#define TRANSFORMER_DELTA_DELTA_H

/**
 * @file Transformer_Delta_Delta.h
 * @brief Delta-delta (Δ-Δ) classic transformer topology.
 */

#include "Transformer_classic.h"

/**
 * @class TransformerDeltaDelta
 * @brief Delta-delta connected classic transformer.
 * @ingroup transformer
 */
class TransformerDeltaDelta : public Transformer_classic {
public:
    /**
     * @brief Construct a Δ-Δ classic transformer.
     * @param symbol Element identifier.
     * @param location Network area or location string.
     * @param pins Number of pins (phases) per winding.
     * @param values Winding parameters including phase lag and coupling coefficients.
     */
    TransformerDeltaDelta(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values);

    ~TransformerDeltaDelta() override;

private:
    // inherited from the parent class
};

#endif // TRANSFORMER_DELTA_DELTA_H
