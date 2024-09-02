#ifndef TRANSFORMER
#define TRANSFORMER

#include "Element.h"


class Transformer : public Element {
public:
	Transformer() {}
	~Transformer() {}

	void compute_y_parameters_transformer(double R_p, double X_p, double R_s, double X_s, double a);
private:
};

#endif
