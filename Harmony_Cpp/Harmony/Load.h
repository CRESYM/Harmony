#ifndef LOAD
#define LOAD

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


class Load : public Element {
public:
	Load() {}
	~Load() {}

	void compute_y_parameters_rlc(double R, double L, double C, double frequency);
private:
};

#endif