#ifndef TRANSFORMER_CLASSIC_H
#define TRANSFORMER_CLASSIC_H

/**
 * @file Transformer_classic.h
 * @brief Classic two-winding transformer with mutual inductance.
 */

#include "Transformer_base.h"

/**
 * @class Transformer_classic
 * @brief Classic transformer with primary, secondary, and mutual inductance.
 * @ingroup transformer
 */
class Transformer_classic : public Transformer_base {
public:
	/**
	 * @brief Construct a classic transformer model.
	 * @param symbol Element identifier.
	 * @param location Network area or location string.
	 * @param pins Number of pins (phases) per winding.
	 * @param values Winding and mutual inductance parameter vector.
	 */
	Transformer_classic(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& values);

	~Transformer_classic();

    double getMutualInductance() const { return M; } // get mutual inductance

protected:
	double M; // mutual inductance
};


#endif
