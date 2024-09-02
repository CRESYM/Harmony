#ifndef LOAD
#define LOAD

#include "Element.h"


class Load : public Element {
public:
	Load() {}
	~Load() {}

	void compute_y_parameters_rlc(double R, double L, double C, double frequency);
private:
};

#endif