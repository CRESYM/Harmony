#ifndef GENERATOR
#define GENERATOR

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


class Generator : public Element {
public:
	Generator() {}
	~Generator() {}

	void compute_y_parameters_generator(double R_f, double L_f, double X_d, double T_f, double frequency);
private:
};

#endif

