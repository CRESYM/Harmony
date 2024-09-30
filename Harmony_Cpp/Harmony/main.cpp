#include "Element.h"
//#include "TransmissionLine.h"
//#include "Generator.h"
//#include "Load.h"
#include "Transformer.h"
#include "Bus.h"
//#include "Impedance.h"
#include "network.h"
#include "Admittance.h"
//#include "AC_source.h"




int main() {

	//Admittance* y = new Admittance("y1", 3, DenseMatrix(1, 1, { symbol("y")}));
	//y->printElementValues();

	Network* myNetwork = new Network();

	// Create Bus objects
	Bus* bus1 = new Bus("Bus1", 3);// Primary winding bus
	Bus* bus2 = new Bus("Bus2", 3);  // Secondary winding bus
	Bus* gnd = new Bus("gnd", 3); // Ground bus

	// Create a Transformer with 2 pins and a vector of 5 values (R_primary, X_primary, R_secondary, X_secondary, a)
	std::vector<double> values = { 4.0, 6.0, 3.0, 2.0, 1.5 }; // R1, X1, R2, X2, turns ratio a
	Transformer* transformer = new Transformer("T1", 2, values);

	// Compute the Y-parameters for the transformer at a given frequency
	double frequency = 60.0;  // 60 Hz frequency
	transformer->compute_y_parameters(frequency);

	// Add elements to the network
	//myNetwork->addElement(y->getElementSymbol(), y);

	// Print the transformer's Y-parameter matrix values
	transformer->printElementValues();

	// Add transformer to the network
	myNetwork->addElement(transformer->getElementSymbol(), transformer);


	// Add buses to the network
	//myNetwork->addBus("Bus1", bus1);
	//myNetwork->addBus("gnd", gnd);

	// Connect elements to buses
	//myNetwork->connectElementToBus(y, 1, bus1);
	//myNetwork->connectElementToBus(y, 2, gnd);
	// 
	// Connect the transformer's primary winding to bus1 and secondary winding to bus2
	myNetwork->connectElementToBus(transformer, 1, bus1);  // Primary side connection
	myNetwork->connectElementToBus(transformer, 2, bus2);  // Secondary side connection

	// Print the connections to verify the network
	myNetwork->printConnections();

	// Test equivalent impedance calculation between buses
	std::vector<Bus*> start_buses = { bus1 };
	std::vector<Bus*> end_buses = { bus2 };
	std::vector<Element*> elem;
	myNetwork->compute_equivalent_impedance(start_buses, end_buses, elem);

	//vector<Bus*> start_buses;
	//vector<Bus*> end_buses;
	//vector<Element*> elem;
	//start_buses.push_back(bus1);
	//end_buses.push_back(gnd);


	//myNetwork->compute_equivalent_impedance(start_buses, end_buses, elem);

	//delete myNetwork;
	// Clean up dynamically allocated memory
	delete transformer;
	delete myNetwork;
	delete bus1;
	delete bus2;
	delete gnd;

	//// Frequency for Y-parameter computation
	//double frequency = 50.0; // Example frequency in Hz


	//// Clean up dynamically allocated memory
	//delete bus1;
	//delete bus2;
	//delete load1;
	//delete load2;
	//delete generator;


	//delete load1;

	return 0;
}
