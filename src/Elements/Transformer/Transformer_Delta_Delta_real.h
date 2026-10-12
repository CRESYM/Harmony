#ifndef TRANSFORMER_DELTA_DELTA_REAL_H
#define TRANSFORMER_DELTA_DELTA_REAL_H

/**
 * @file Transformer_Delta_Delta_real.h
 * @brief Delta-delta (Δ-Δ) real transformer topology.
 */

#include "Transformer_real.h"

/**
 * @class TransformerDeltaDelta_real
 * @brief Delta-delta connected real transformer (paper eq. (12)).
 * @ingroup transformer
 */
class TransformerDeltaDelta_real : public Transformer_real {
public:
    /**
     * @brief Construct a Δ-Δ real transformer.
     * @param symbol Element identifier.
     * @param location Network area or location string.
     * @param pins Number of pins (phases) per winding (must be 3).
     * @param values Turns ratio, phase shift, and winding parameter vector.
     */
    TransformerDeltaDelta_real(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values);

    ~TransformerDeltaDelta_real() override;
};

#endif
