#ifndef TRANSMISSIONLINE_H
#define TRANSMISSIONLINE_H

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


class TransmissionLine : public Element {
public:
    // Parameterized constructor that calls the base class constructor
   // TransmissionLine(const std::string& symbol, int inputPins, int outputPins)
    //    : Element(symbol, inputPins, outputPins) {}

 //   ~TransmissionLine() {}

   // void compute_y_parameters(double R, double L, double G, double C, double length, double frequency);
private:
	double R_tl = 0.01;       // Resistance per unit length (ohms/m)
	double L_tl = 2.5e-7;     // Inductance per unit length (H/m)
	double G_tl = 1e-9;       // Conductance per unit length (S/m)
	double C_tl = 1e-11;      // Capacitance per unit length (F/m)
	double length = 1000;     // Length of the transmission line (m)
};

#endif

