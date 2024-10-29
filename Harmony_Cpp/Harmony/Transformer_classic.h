#ifndef TRANSFORMER_CLASSIC_H
#define TRANSFORMER_CLASSIC_H

#include "Transformer_base.h"

class Transformer_classic : public Transformer_base {
	
	Transformer_classic();

	~Transformer_classic();

    double getPhaseLag() const { return phaseLag; }  // Method to get phase lag

    double getCoupling() const { return M; } // get mutual inductance

private:
	double M; // mutual inductance
	double phaseLag; // phase shift
};


#endif
