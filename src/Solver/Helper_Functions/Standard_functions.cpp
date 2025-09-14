#include "Standard_functions.h"

// Constants
const double mu_0 = 4 * M_PI * 1e-7; // Standard mu_0
const double epsilon_0 = 8.854e-12; // Standard epsilon_0
const double gamma_num = 0.5772156649; // Euler-Mascheroni constant

int sgn(int v) {
	return (v > 0) - (v < 0);
}