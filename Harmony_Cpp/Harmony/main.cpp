#include "Element.h"
//#include "TransmissionLine.h"
#include "Generator.h"
//#include "Load.h"
//#include "Transformer.h"
#include "Bus.h"
#include "Impedance.h"
#include "network.h"


#include <symengine/expression.h>
#include <symengine/complex_double.h>
#include <symengine/complex.h>
#include <symengine/eval_double.h>
#include <symengine/symengine_config.h>

#include <iostream>
#include <complex>
#include <vector>

using namespace std;

//int main()
//{
	/*using namespace SymEngine;

	// Define a Cable object
	Cable c;

	std::unordered_map<Element, int, ElementHash> myMap;

	// Define some sample values for P and Z matrices
	std::vector<std::vector<double>> P = {
		{1.0, 2.0, 3.0},
		{4.0, 5.0, 6.0},
		{7.0, 8.0, 9.0}
	};

	std::vector<std::vector<double>> Z = {
		{9.0, 8.0, 7.0},
		{6.0, 5.0, 4.0},
		{3.0, 2.0, 1.0}
	};

	// Define some sample kwargs
	std::unordered_map<std::string, std::vector<std::pair<double, double>>> kwargs;
	// Populate kwargs as needed

	// Call the cable function
	bool transformation = true;
	cable(c, P, Z, kwargs, transformation);

	// Insert an element into myMap
	std::unordered_map<std::string, std::string> args = {
	{"symbol", "some_symbol"},
	{"input_pins", "4"},
	{"output_pins", "2"},
	{"transformation", "true"}
	};

	//std::unordered_map<Element, int, ElementHash> myMap;
	Element elem(args);
	myMap.emplace(elem, 42); // Inserting the element with an arbitrary value

	//test the eval_parameters function
	// Define a sample Complex number
	const std::complex<double> s(1.0, 2.0);

	std::cout << "yes!.\n";

	// Call the eval_parameters function

	Eval_parameter evalParam;
	auto result = evalParam.eval_parameters(c, s);

	// Print the result
	std::cout << "Z matrix:" << std::endl;
	for (const auto& row : result.first) {
		for (const auto& elem : row) {
			/// Evaluate the SymEngine expression to get a real number
			std::cout << elem.real() << " + " << elem.imag() << "i ";
		}
		std::cout << std::endl;
	}

	std::cout << "Y matrix:" << std::endl;
	for (const auto& row : result.second) {
		for (const auto& elem : row) {
			//std::cout << elem.getReal() << " + " << elem.getImag() << "i ";
			std::cout << elem.real() << " + " << elem.imag() << "i ";
		}
		std::cout << std::endl;
	}*/


	//Test for element
	// Create an Element with symbol "R1", 2 input pins, and 2 output pins
	/*Element elem("R1", 2, 2);

	// Print out the element's information
	elem.printElementInfo();

	// Get all nodes (pins)
	auto nodes = elem.getNodes();
	std::cout << "All Nodes:" << std::endl;
	for (const auto& node : nodes) {
		std::cout << "  " << node << std::endl;
	}

	// Get nodes excluding a specific pin (e.g., "1.1")
	auto filtered_nodes = elem.getNodesByPin("1.1");
	std::cout << "Nodes excluding '1.1':" << std::endl;
	for (const auto& node : filtered_nodes) {
		std::cout << "  " << node << std::endl;
	}*/

/*
// Test for Transmission Line Parameters
	double R_tl = 0.01;       // Resistance per unit length (ohms/m)
	double L_tl = 2.5e-7;     // Inductance per unit length (H/m)
	double G_tl = 1e-9;       // Conductance per unit length (S/m)
	double C_tl = 1e-11;      // Capacitance per unit length (F/m)
	double length = 1000;     // Length of the transmission line (m)
	double frequency = 1e6;   // Frequency (Hz)


	// Create an instance of TransmissionLine and compute Y-parameters
	TransmissionLine transmissionLine;
	// Compute the Y-parameters for the transmission line
	transmissionLine.compute_y_parameters(R_tl, L_tl, G_tl, C_tl, length, frequency);

	//Test for Transformer Parameters
	double R_p = 0.5;  // Primary winding resistance (ohms)
	double X_p = 1.0;  // Primary winding leakage reactance (ohms)
	double R_s = 0.1;  // Secondary winding resistance (ohms)
	double X_s = 0.2;  // Secondary winding leakage reactance (ohms)
	double a = 10.0;   // Turns ratio (primary to secondary)

	// Create an instance of Transformer and compute Y-parameters
	Transformer transformer;
	// Compute the Y-parameters for the transformer
	transformer.compute_y_parameters_transformer(R_p, X_p, R_s, X_s, a);

	// Test for Generator Parameters
	double R_f = 1.0;
	double L_f = 0.01;
	double X_d = 1.0;
	double T_f = 0.1;
	double gen_frequency = 60;

	// Create an instance of Generator and compute Y-parameters
	Generator generator;
	generator.compute_y_parameters_generator(R_f, L_f, X_d, T_f, gen_frequency);

	//Test for Load (RLC) Parameters
	double R = 10.0;
	double L = 0.01;
	double C = 0.001;
	double load_frequency = 60;

	// Create an instance of Load and compute Y-parameters
	Load load;
	load.compute_y_parameters_rlc(R, L, C, load_frequency);*/

	// Example for the IEEE 4-bus system
	//int numBuses = 4;
	//int numBranches = 5;

	// Define the branches as pairs of (fromBus, toBus)
	/*std::vector<std::pair<int, int>> branches = {
		{1, 2},  // Line 1: Bus 1 to Bus 2
		{1, 3},  // Line 2: Bus 1 to Bus 3
		{2, 3},  // Line 3: Bus 2 to Bus 3
		{2, 4},  // Line 4: Bus 2 to Bus 4
		{3, 4}   // Line 5: Bus 3 to Bus 4
	};*/

	// Create a Bus object and print the incidence matrix
	//Bus busSystem(numBuses, numBranches, branches);
	//busSystem.printIncidenceMatrix();
int main()
{
	Network myNetwork;

	// Create Bus objects
	Bus* bus1 = new Bus("Bus1");
	Bus* bus2 = new Bus("Bus2");

	// Create Impedance and Generator objects
	Impedance* impedance1 = new Impedance("Z1", 2, 2);  // Symbol, inputPins, outputPins
	Impedance* impedance2 = new Impedance("Z2", 2, 2);
	Generator* generator = new Generator("G1", 1, 1);

	// Add buses to the network
	myNetwork.addBus("Bus1", bus1);
	myNetwork.addBus("Bus2", bus2);

	// Add elements to the network
	myNetwork.addElement("Z1", impedance1);
	myNetwork.addElement("Z2", impedance2);
	myNetwork.addElement("G1", generator);

	// Connect elements to buses
	myNetwork.connectElementToBus(impedance1, bus1);
	myNetwork.connectElementToBus(impedance2, bus2);
	myNetwork.connectElementToBus(generator, bus1);

	// Print the connections to verify the network
	myNetwork.printConnections();

	// Compute Y parameters for the generator
	generator->compute_y_parameters(0.02, 0.05, 1.0, 0.1, 60.0);


	// Clean up dynamically allocated memory
	delete bus1;
	delete bus2;
	delete impedance1;
	delete impedance2;
	delete generator;


	return 0; 
}