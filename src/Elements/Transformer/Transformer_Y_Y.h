#ifndef TRANSFORMER_Y_Y_H
#define TRANSFORMER_Y_Y_H

/**
 * @file Transformer_Y_Y.h
 * @brief Wye-wye (Y-Y) classic transformer topology.
 */

#include "Transformer_classic.h"

/**
 * @class TransformerYY
 * @brief Wye-wye connected classic transformer.
 * @ingroup transformer
 */
class TransformerYY : public Transformer_classic {
public:
    /**
     * @brief Construct a Y-Y classic transformer.
     * @param symbol Element identifier.
     * @param location Network area or location string.
     * @param pins Number of pins (phases) per winding.
     * @param values Winding and mutual inductance parameter vector.
     */
    TransformerYY(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values);

    ~TransformerYY();

private:
    // inherited from parent class
  };


#endif // TRANSFORMER_Y_Y_H
