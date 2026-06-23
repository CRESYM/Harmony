#ifndef TRANSFORMER_Y_Y_REAL_H
#define TRANSFORMER_Y_Y_REAL_H

/**
 * @file Transformer_Y_Y_real.h
 * @brief Wye-wye (Y-Y) real transformer topology.
 */

#include "Transformer_real.h"

/**
 * @class TransformerYY_real
 * @brief Wye-wye connected real transformer with turns ratio and phase shift.
 * @ingroup transformer
 */
class TransformerYY_real : public Transformer_real {
public:
    /**
     * @brief Construct a Y-Y real transformer.
     * @param symbol Element identifier.
     * @param location Network area or location string.
     * @param pins Number of pins (phases) per winding.
     * @param values Turns ratio, phase shift, and winding parameter vector.
     */
    TransformerYY_real(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values);

    ~TransformerYY_real();

private:
    // inherited from parent class
};


#endif // TRANSFORMER_Y_Y_REAL_H
