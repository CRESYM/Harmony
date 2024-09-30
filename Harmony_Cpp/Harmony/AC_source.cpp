#include "AC_source.h"

AC_source::AC_source(const std::string& symbol, int pins, DenseMatrix Z)
	: Element(symbol, pins, pins)
{
    if (Z.ncols() != 0)  // if there are entries
    {
        if (pins > 0) { // Check for valid number of pins
            if (Z.ncols() == 1) {
                for (int i = 0; i < pins; i++)
                    Y_matrix.set(i, i, div(integer(1), Z.get(0, 0)));
            }
            else if (Z.ncols() == pins) {
                for (int i = 0; i < pins; i++)
                    Y_matrix.set(i, i, div(integer(1), Z.get(0, 0)));
            }
            else
                throw invalid_argument("Invalid number of series impedance vector entries: " + Z.ncols());
        }
    }
    else
        throw invalid_argument("Invalid number of pins, must be greater than 0!");
}

// Destructor
AC_source::~AC_source() {
    // No need for manual memory management for DenseMatrix or other standard library components
    std::cout << "AC source object for " << getElementSymbol() << " destroyed." << std::endl;
}

void AC_source::printElementValues() {
	printElementInfo();

	//std::cout << "AC source has " << input_pins << " phases and it is represented with AC source with " <<
	//	V << " magnitude and " << theta << " phase and series impedance " << Z << std::endl;
}

