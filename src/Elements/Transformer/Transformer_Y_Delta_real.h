#ifndef TRANSFORMER_Y_DELTA_REAL_H
#define TRANSFORMER_Y_DELTA_REAL_H

/**
 * @file Transformer_Y_Delta_real.h
 * @brief Wye-delta (Y-Δ) real transformer topology.
 */

#include "Transformer_real.h"

/**
 * @class TransformerYDelta_real
 * @brief Wye-delta connected real transformer (paper eq. (9)).
 * @ingroup transformer
 */
class TransformerYDelta_real : public Transformer_real {
public:
    /**
     * @brief Construct a Y-Δ real transformer.
     * @param symbol Element identifier.
     * @param location Network area or location string.
     * @param pins Number of pins (phases) per winding (must be 3).
     * @param values Turns ratio, phase shift, and winding parameter vector.
     */
    TransformerYDelta_real(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values);

    ~TransformerYDelta_real() override;
};

#endif
