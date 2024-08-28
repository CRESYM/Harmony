#ifndef TRANSFORMER
#define TRANSFORMER

#include "Element.h"

#include <symengine/expression.h>
#include <symengine/symbol.h>
#include <symengine/add.h>
#include <symengine/mul.h>
#include <symengine/pow.h>
#include <symengine/real_double.h>
#include <symengine/eval.h>
#include <symengine/functions.h>
#include <symengine/complex.h>
#include <symengine/complex_double.h>


class Transformer : public Element {
public:
	Transformer() {}
	~Transformer() {}

	void compute_y_parameters_transformer(double R_p, double X_p, double R_s, double X_s, double a);
private:
};

#endif
