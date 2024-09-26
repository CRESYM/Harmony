// element.cpp

#include "Element.h"
#include "Constants.h"
#include "Load.h"
#include "Generator.h"
#include "Impedance.h"
#include <iostream>
#include <vector>


using namespace SymEngine;

// Destructor
Element::~Element() {}

// Implementation of compute_y_parameters for single-phase (default implementation)
void Element::compute_y_parameters(double frequency) {
    std::cout << "This should be overridden in derived classes." << std::endl;
}
void Element::printElementValues() {}