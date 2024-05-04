#ifndef _Impedance_h_

#define _Impedance_h_

#include "Element.h"
#include "basic.h"

#include <vector>
#include <complex>

class Impedance : public Element {
public:
	// Constructor with default values
	Impedance(std::vector<std::vector<Basic>> val = {}, std::vector<std::vector<Basic>> abcd = {})
		: value(val), ABCD(abcd) {}

/*	// Default constructor
	Impedance() = default;*/

	~Impedance() {};

	// Function declaration for impedance
	friend Impedance impedance(const std::vector<std::vector<Basic>>& value, const std::vector<std::vector<Basic>>& z, int pins, bool transformation);

	//friend Impedance impedance(const Basic& z, int pins, bool transformation);

private:
	std::vector<std::vector<Basic>> value; // impedance value //default value for array
	std::vector<std::vector<Basic>> ABCD;
	double coth(double x); // Define coth function
};

#endif

