#include "network.h"
#include "Bus.h"
#include "Include_components.h"

int main() {
	// Create Transformer object with 2 pins and parameters
	std::vector<double> transformer_values = { 10.0, 5.0, 12.0, 6.0, 2.0 }; // R_primary, X_primary, R_secondary, X_secondary, Turns Ratio
	Transformer* transformer = new Transformer("T1", 2, transformer_values);

	Admittance* y = new Admittance("y1", 2, DenseMatrix(1, 1, { symbol("y")}));
	//Admittance* y = new Admittance("y1", 3, DenseMatrix(1, 1, { symbol("y") }));
	//y->printElementValues();

	AC_source* ac = new AC_source("ac", 2, DenseMatrix(1, 1, { integer(10) }));
	//AC_source* ac = new AC_source("ac", 3, DenseMatrix(1, 1, { integer(10) }));
	Network* myNetwork = new Network();

	// Create Bus objects
	//Bus* bus1 = new Bus("Bus1", 3);
	//Bus* gnd = new Bus("gnd", 3);
	Bus* bus1 = new Bus("Bus1", 2);
	Bus* gnd = new Bus("gnd", 2);

	// Add elements to the network
	myNetwork->addElement(y);
	myNetwork->addElement(transformer);  // Add the transformer to the network
	myNetwork->addElement(ac);

	// Add buses to the network
	myNetwork->addBus("Bus1", bus1);
	myNetwork->addBus("gnd", gnd);

	// Connect elements to buses
	myNetwork->connectElementToBus(transformer, 1, bus1);  // Connect transformer to bus1
	myNetwork->connectElementToBus(transformer, 2, gnd);    // Connect transformer to ground
	//myNetwork->connectElementToBus(y, 1, bus1);
	//myNetwork->connectElementToBus(y, 2, gnd);
	myNetwork->connectElementToBus(ac, 1, bus1);
	myNetwork->connectElementToBus(ac, 2, gnd);

	// Print the connections to verify the network
	myNetwork->printConnections();

	// Frequency for Y-parameter computation
	double frequency = 50.0; // Example frequency in Hz
	// Compute Y-parameters for the transformer
	transformer->compute_y_parameters(frequency);

	vector<Bus*> start_buses;
	vector<Bus*> end_buses;
	vector<Element*> elem;

	start_buses.push_back(bus1);
	end_buses.push_back(gnd);
	elem.push_back(ac);

	myNetwork->compute_equivalent_impedance(start_buses, end_buses, elem);

	
	// Clean up dynamically allocated memory
	delete myNetwork;
	delete transformer;  // Clean up transformer
	delete y;           // Clean up Admittance
	delete ac;          // Clean up AC source
	delete bus1;        // Clean up Bus1
	delete gnd;        // Clean up ground bus


	//delete load1;

	return 0;
}
