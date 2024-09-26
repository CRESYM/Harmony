#include "Element.h"
//#include "TransmissionLine.h"
#include "Generator.h"
#include "Load.h"
//#include "Transformer.h"
#include "Bus.h"
#include "Impedance.h"
#include "network.h"
#include "Admittance.h"
#include "AC_source.h"


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

//Test Admittance
//std::vector<RCP<Symbol>> admittanceValues; // Initialize with your values
//Admittance* admittance = new Admittance("A1", 2, admittanceValues);

int main() {
	Network myNetwork;

	// Create Bus objects
	Bus* bus1 = new Bus("Bus1");
	Bus* bus2 = new Bus("Bus2");

	// Create Load and Generator objects
	vector<double> values{ 10.0, 0.01, 0.001 };
	Load* load1 = new Load("L1", 3, values); // Assuming it is a three-phase load
	load1->printElementValues();

	AC_source* source1 = new AC_source("s1", 3, 10.0, 100.0, 0.0, 100, 50, 0, 150, 0, 75);
	source1->printElementValues();

	//Load* load2 = new Load("L2", 3, values);
	//Generator* generator = new Generator("G1", 1, 1);


	//// Check if the generator was created properly
	//if (!generator) {
	//	std::cerr << "Failed to create generator." << std::endl;
	//	return -1;
	//}

	//// Add buses to the network
	//myNetwork.addBus("Bus1", bus1);
	//myNetwork.addBus("Bus2", bus2);

	//// Add elements to the network
	//myNetwork.addElement("L1", load1);
	//myNetwork.addElement("L2", load2);
	//myNetwork.addElement("G1", generator);

	//// Connect elements to buses
	//myNetwork.connectElementToBus(load1, bus1);
	//myNetwork.connectElementToBus(load2, bus2);
	//myNetwork.connectElementToBus(generator, bus1);

	//// Print the connections to verify the network
	//myNetwork.printConnections();

	//// Frequency for Y-parameter computation
	//double frequency = 60.0; // Example frequency in Hz

	//// Compute Y parameters with error handling
	//try {
	//	std::cout << "\nComputing Y-parameters for Generator:\n";
	//	generator->compute_y_parameters(frequency);
	//}
	//catch (const std::exception& e) {
	//	std::cerr << "Error computing Y-parameters for Generator: " << e.what() << std::endl;
	//}

	//try {
	//	std::cout << "\nComputing Y-parameters for Load L1:\n";
	//	load1->compute_y_parameters(frequency);
	//}
	//catch (const std::exception& e) {
	//	std::cerr << "Error computing Y-parameters for Load L1: " << e.what() << std::endl;
	//}

	//try {
	//	std::cout << "\nComputing Y-parameters for Load L2:\n";
	//	load2->compute_y_parameters(frequency);
	//}
	//catch (const std::exception& e) {
	//	std::cerr << "Error computing Y-parameters for Load L2: " << e.what() << std::endl;
	//}

	//// Clean up dynamically allocated memory
	//delete bus1;
	//delete bus2;
	//delete load1;
	//delete load2;
	//delete generator;


	delete load1;

	return 0;
}
