/**
 * @file example_generator.cpp
 * @brief Runnable example: Generator element demonstration.
 */
#include "Examples.h"

#include "../network.h"
#include "../Include_components.h"

void example_generator(bool plotting_enabled /*=true*/) {
	std::vector<double> values = {0.0952, 0.5, 2.285}; // R_f, T_f, X_d

	Generator* gen = new Generator("G1", "Bus1", 1, 345e3, values);

	if (plotting_enabled){
		gen->plotYParameters(1.0, 100.0, 500);
	}

	cout << "Press Enter to continue...\n";
	cin.get();
}