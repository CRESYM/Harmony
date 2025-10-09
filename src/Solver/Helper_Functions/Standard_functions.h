#ifndef _STANDARD_FUNCTIONS_H_
#define _STANDARD_FUNCTIONS_H_

#include "../../Constants.h"

// Standard constants
extern const double mu_0; // vacuum permitivity
extern const double epsilon_0; // vacuum permeability
extern const double gamma_num;


// Standard mathematical functions

// Helper sign function
extern int sgn(int v);
extern MatrixXcd vectorToMatrix(const vector<vector<complex<double>>>& vec);

#endif // _STANDARD_FUNCTIONS_H_