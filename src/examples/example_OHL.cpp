#include "Examples.h"

#include "../network.h"
#include "../Bus.h"
#include "../Include_components.h"

/**
 * @brief Example function to model and analyze an overhead line (OHL).
 *
 * This function demonstrates how to create an overhead line with specific
 * geometric and material properties, compute its electrical parameters,
 * and analyze its behavior over a frequency range.
 */
void example_OHL() {
	// OHL constructor check
	std::vector<double> distances = { 11.8, 27.5 }; // deltaXbc, ybc
	std::vector<int> numbers = { 2, 2 }; // number of bundles, number of conductors per bundle
	std::vector<double> values_g = { 0.9196, 0.0062, 10.0, 7.5, 6.5 }; // Rgdc, rg, dgsag, DeltaYg, DeltaXg
	Overhead_Line* ohl = new Overhead_Line("ohl", "2phase", 227e3, make_tuple(1.0, 1.0, 1.0),
		make_tuple("flat", numbers, distances, 0.01436, 0.06266, 10.0, 0.4572), // organization, number of bundles, distance parameters, rc, Rdc, dsag, dsb
		make_tuple(2, values_g, 1.0));

	ohl->writeFile(1, 1e4, 1000);

	cout << "Y-parameters printed in file." << endl;

	ohl->plotYParameters(10, 1e3, 1000);

	cout << "Press Enter to continue...\n";
	cin.get();
}