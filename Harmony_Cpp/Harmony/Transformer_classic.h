#ifndef TRANSFORMER_CLASSIC_H
#define TRANSFORMER_CLASSIC_H

#include "Transformer_base.h"

// Classic transformer represented with primary and secondary windings, and mutual inductance
class Transformer_classic : public Transformer_base {
	
	// Constructor
	Transformer_classic(const std::string& symbol, int pins, const std::vector<double>& values);

	~Transformer_classic();

    double getMutualInductance() const { return M; } // get mutual inductance

private:
	double M; // mutual inductance
};


#endif
