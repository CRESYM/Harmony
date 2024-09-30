#include "network.h"
#include "Bus.h"
#include "Include_components.h"

int main() {

	Admittance* y = new Admittance("y1", 3, DenseMatrix(1, 1, { symbol("y")}));
	//y->printElementValues();

	AC_source* ac = new AC_source("ac", 3, DenseMatrix(1, 1, { integer(10) }));

	Network* myNetwork = new Network();

	// Create Bus objects
	Bus* bus1 = new Bus("Bus1", 3);
	Bus* gnd = new Bus("gnd", 3);

	// Add elements to the network
	myNetwork->addElement(y);
	myNetwork->addElement(ac);


	// Add buses to the network
	myNetwork->addBus("Bus1", bus1);
	myNetwork->addBus("gnd", gnd);

	// Connect elements to buses
	myNetwork->connectElementToBus(y, 1, bus1);
	myNetwork->connectElementToBus(y, 2, gnd);
	myNetwork->connectElementToBus(ac, 1, bus1);
	myNetwork->connectElementToBus(ac, 2, gnd);

	// Print the connections to verify the network
	myNetwork->printConnections();

	vector<Bus*> start_buses;
	vector<Bus*> end_buses;
	vector<Element*> elem;
	start_buses.push_back(bus1);
	end_buses.push_back(gnd);
	elem.push_back(ac);

	myNetwork->compute_equivalent_impedance(start_buses, end_buses, elem);

	delete myNetwork;

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
