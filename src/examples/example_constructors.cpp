/**
 * @file example_constructors.cpp
 * @brief Runnable example: Instantiate basic element types via constructors.
 */
#include "Examples.h"


#include "../network.h"
#include "../Bus.h"
#include "../Include_components.h"

void example_constructors() {


	// TL constructor check
	std::vector<double> transmission_line_values = { 0.01, 2.5e-7, 1e-9, 1e-11, 1000 };
	TransmissionLine* transmission_line = new TransmissionLine("tl", "AC1", 1, transmission_line_values);
	transmission_line->writeFile(10, 10000000, 1000);


	// Generator constructor check
	//std::vector<double> generator_values = { 1.0, 0.01, 1.0, 0.1 };
	//Generator* g = new Generator("gen", "AC1", 1, generator_values);

	//vector<Bus*> start_buses;
	//vector<Bus*> end_buses;
	//vector<Element*> skip_elements;

	//start_buses.push_back(bus1);
	//end_buses.push_back(gnd);
	//skip_elements.push_back(ac);


	//Eigen::MatrixXcd equivalent_impedance;
	//myNetwork->compute_equivalent_impedance(start_buses, end_buses, skip_elements);
}