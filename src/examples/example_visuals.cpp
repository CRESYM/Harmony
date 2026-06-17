#include "Examples.h"

#include "../Include_components.h"

void example_visuals(bool plotting_enabled /*=true*/) {
	//// TL constructor check
	std::vector<double> transmission_line_values = { 0.01, 2.5e-7, 1e-9, 1e-11, 1000 };
	TransmissionLine* transmission_line = new TransmissionLine("tl1", "DC1", 1, transmission_line_values);
	
	if (plotting_enabled){
		transmission_line->plotYParameters(10, 10000000, 1000);
	}

	transmission_line->writeFile(10, 10000000, 1000);

	delete transmission_line;

	cout << "Press Enter to continue...\n";
	cin.get();
}