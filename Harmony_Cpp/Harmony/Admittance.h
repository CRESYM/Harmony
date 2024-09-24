#ifndef ADMITTANCE_H
#define ADMITTANCE_H

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

using namespace SymEngine;


/*
Creates admittance with specified number of input/output pins `pins`. The admittance expression
 `exp` has to be given in Ω and can have both numerical and symbolic value (example: `z = s-2`).
Depending on the provided vector of admittance values, we differ three cases. If the number of 
element pins is greater than 1, admittance can be represented with vector with one, `pins` or 
`pins × pins` number of elements. Namely
-In the case of 1×1 vector, admittance has only one value. Then this value is given to all diagonal
admittance entries.
-In the case of `pins` elements, they are representing diagonal entiries of admittance.
-In case of `pins x pins` elements, they are representing all matrix of admittance.
*/

class Admittance : public Element {
public:
	Admittance(int pins, std::vector<RCP<Symbol>> admittance);
	~Admittance() {};
private:
	RCP<Symbol>** admittance;
};

#endif
