#include "AC_source.h"

void AC_source::printElementValues() {
	printElementInfo();

	std::cout << "AC source has " << input_pins << " phases and it is represented with AC source with " <<
		V << " magnitude and " << theta << " phase and series impedance " << Z << std::endl;
}