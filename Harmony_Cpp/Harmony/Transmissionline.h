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
};

#endif

