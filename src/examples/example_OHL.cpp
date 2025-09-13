#include "Examples.h"

#include "../network.h"
#include "../Bus.h"
#include "../Include_components.h"

void example_OHL() {
	// OHL constructor check
	std::vector<double> distances = { 11.8, 27.5 };
	std::vector<int> numbers = { 1, 1 };
	std::vector<double> values_g = { 0.9196, 0.0062, 10.0, 7.5, 6.5 };
	Overhead_Line* ohl = new Overhead_Line("ohl", "2phase", 227e3, make_tuple(1.0, 1.0, 1.0),
		make_tuple("flat", numbers, distances, 0.01436, 0.06266, 10.0, 0.4572),
		make_tuple(2, values_g, 1.0));

	ohl->writeFile(1, 1e4, 1000);

	cout << "Y-parameters printed in file." << endl;

	ohl->plotYParameters(10, 1e3, 100);
}