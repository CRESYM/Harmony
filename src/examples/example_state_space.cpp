/**
 * @file example_state_space.cpp
 * @brief Runnable example: Build state-space (A, B, C, D) matrices for an RLC network.
 */
#include "Examples.h"

#include "../Solver/State_Space_Model/State_Space_Model.h"
#include "../network.h"
#include "../Bus.h"
#include "../Include_components.h"

void example_state_space() {
	Network myNetwork;

	Bus* bus0 = new Bus("gnd", "AC1", 1);
	Bus* bus1 = new Bus("1", "AC1", 1);
	Bus* bus2 = new Bus("2", "AC1", 1);

	myNetwork.addBus(bus0->getBusName(), bus0);
	myNetwork.addBus(bus1->getBusName(), bus1);
	myNetwork.addBus(bus2->getBusName(), bus2);

	AC_source* ac = new AC_source("AC1", "AC1", 1, 345e3, 0.0);
	myNetwork.addElement(ac);
	myNetwork.connectElementToBus(ac, 1, bus1);
	myNetwork.connectElementToBus(ac, 2, bus0);

	Resistor* r1 = new Resistor("R1", "AC1", 1, { 2.0 });
	myNetwork.addElement(r1);
	myNetwork.connectElementToBus(r1, 1, bus1);
	myNetwork.connectElementToBus(r1, 2, bus2);

	Capacitor* c1 = new Capacitor("C1", "AC1", 1, { 1e-6 });
	myNetwork.addElement(c1);
	myNetwork.connectElementToBus(c1, 2, bus0);
	myNetwork.connectElementToBus(c1, 1, bus2);

	StateSpaceModel model;
	model.formState(&myNetwork, { bus1 });

	cout << "State-space matrices:" << endl;
	cout << "A matrix:" << endl << model.getA() << endl;
	cout << "B matrix:" << endl << model.getB() << endl;
	cout << "C matrix:" << endl << model.getC() << endl;
	cout << "D matrix:" << endl << model.getD() << endl;
}
