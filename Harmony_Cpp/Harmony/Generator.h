#ifndef GENERATOR
#define GENERATOR

#include "Element.h"

class Generator : public Element {
public:
	Generator() {}
	~Generator() {}

	void compute_y_parameters_generator(double R_f, double L_f, double X_d, double T_f, double frequency);
private:
};

#endif

