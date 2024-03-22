#ifndef _Impedance_h_

#define _Impedance_h_

#include "Element.h"
#include <vector>
#include <complex>

class Impedance : public Element {
public:
	struct ImpedanceData {
		std::vector<std::complex<double>> value; // impedance value
		std::vector<std::complex<double>> ABCD;
	};

	// Default constructor
	Impedance() = default;

	~Impedance() {};

	// Custom constructor
	/*Impedance(const std::vector<std::complex<double>>& value,
		const std::vector<std::complex<double>>& ABCD)
		: data{ value, ABCD } {} */

private:
	ImpedanceData data;

	
};

#endif

