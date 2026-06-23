#ifndef TRANSFORMER_DELTA_Y_REAL_H
#define TRANSFORMER_DELTA_Y_REAL_H

/**
 * @file Transformer_Delta_Y_real.h
 * @brief Delta-wye (Δ-Y) real transformer topology.
 */

#include "Transformer_real.h"

/**
 * @class TransformerDeltaY_real
 * @brief Delta-wye connected real transformer with turns ratio and phase shift.
 * @ingroup transformer
 */
class TransformerDeltaY_real : public Transformer_real {
public:
    /**
     * @brief Construct a Δ-Y real transformer.
     * @param symbol Element identifier.
     * @param location Network area or location string.
     * @param pins Number of pins (phases) per winding.
     * @param values Turns ratio, phase shift, and winding parameter vector.
     */
    TransformerDeltaY_real(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values);

    ~TransformerDeltaY_real() override;


private:
    // inherited from parent class
};

#endif // TRANSFORMER_DELTA_Y_H
#pragma once
