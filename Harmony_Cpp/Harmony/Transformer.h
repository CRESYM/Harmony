#ifndef TRANSFORMER
#define TRANSFORMER

#include "Element.h"


class Transformer : public Element {
public:
    // Parameterized constructor that calls the base class constructor
   // Transformer(const std::string& symbol, int inputPins, int outputPins)
   //     : Element(symbol, inputPins, outputPins) {}

  //  ~Transformer() {}

   // void compute_y_parameters_transformer(double R_p, double X_p, double R_s, double X_s, double a);
private:
	double R_p = 0.5;  // Primary winding resistance (ohms)
	double X_p = 1.0;  // Primary winding leakage reactance (ohms)
	double R_s = 0.1;  // Secondary winding resistance (ohms)
	double X_s = 0.2;  // Secondary winding leakage reactance (ohms)
	double a = 10.0;   // Turns ratio (primary to secondary)
};

#endif
