#include "Element.h"
//#include "TransmissionLine.h"
//#include "Generator.h"
//#include "Load.h"
//#include "Transformer.h"
#include "Bus.h"
//#include "Impedance.h"
#include "network.h"
#include "Admittance.h"
//#include "AC_source.h"




int main() {

	Admittance* y = new Admittance("y1", 3, DenseMatrix(1, 1, { symbol("y")}));
	y->printElementValues();

	Network* myNetwork = new Network();

	// Create Bus objects
	Bus* bus1 = new Bus("Bus1", 3);
	Bus* gnd = new Bus("gnd", 3);

	// Add elements to the network
	myNetwork->addElement(y->getElementSymbol(), y);


	// Add buses to the network
	myNetwork->addBus("Bus1", bus1);
	myNetwork->addBus("gnd", gnd);

	// Connect elements to buses
	myNetwork->connectElementToBus(y, 1, bus1);
	myNetwork->connectElementToBus(y, 2, gnd);

	// Print the connections to verify the network
	myNetwork->printConnections();

	vector<Bus*> start_buses;
	vector<Bus*> end_buses;
	vector<Element*> elem;
	start_buses.push_back(bus1);
	end_buses.push_back(gnd);


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
