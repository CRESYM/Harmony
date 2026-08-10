#ifndef TRANSFORMER_DELTA_Y_H
#define TRANSFORMER_DELTA_Y_H

/**
 * @file Transformer_Delta_Y.h
 * @brief Delta-wye (Δ-Y) classic transformer topology.
 */

#include "Transformer_classic.h"

/**
 * @class TransformerDeltaY
 * @brief Delta-wye connected classic transformer.
 * @ingroup transformer
 */
class TransformerDeltaY : public Transformer_classic {
public:
    /**
     * @brief Construct a Δ-Y classic transformer.
     * @param symbol Element identifier.
     * @param location Network area or location string.
     * @param pins Number of pins (phases) per winding.
     * @param values Winding and mutual inductance parameter vector.
     */
    TransformerDeltaY(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values);

    ~TransformerDeltaY() override;


private:
    // inherited from parent class
};

#endif // TRANSFORMER_DELTA_Y_H
