#include <gtest/gtest.h>
#include "State_Space_Model.h"
#include "network.h"
#include "Bus.h"
#include "Include_components.h"

class TestABCDmatrices : public testing::Test {};

// Test constructor 
TEST_F(TestABCDmatrices, TestSimpleSinglePhase) {

	Network* myNetwork = new Network();

	Bus* bus0 = new Bus("gnd", 1);
	Bus* bus1 = new Bus("1", 1);
	Bus* bus2 = new Bus("2", 1); // Bus for capacitor

	// Add buses to network
	myNetwork->addBus(bus0->getBusName(), bus0);
	myNetwork->addBus(bus1->getBusName(), bus1);
	myNetwork->addBus(bus2->getBusName(), bus2); // Add bus2 for capacitor

	// Create and add elements
	AC_source* ac = new AC_source("AC1", 1, { 0.0 });
	myNetwork->addElement(ac);
	myNetwork->connectElementToBus(ac, 1, bus1); // Connect AC source to bus1
	myNetwork->connectElementToBus(ac, 2, bus0); // Connect AC source to ground bus 

	Resistor* r1 = new Resistor("R1", 1, { 2.0 });
	myNetwork->addElement(r1);
	myNetwork->connectElementToBus(r1, 1, bus1); // Connect resistor to bus1
	myNetwork->connectElementToBus(r1, 2, bus2); // Connect resistor to ground bus

	// Add capacitor as you showed
	Capacitor* c1 = new Capacitor("C1", 1, { 1e-6 }); // Name, pins, capacitance
	myNetwork->addElement(c1);
	myNetwork->connectElementToBus(c1, 2, bus0); // Connect capacitor to ground bus
	myNetwork->connectElementToBus(c1, 1, bus2); // Connect capacitor to bus2

	// Create the StateSpaceModel object
	StateSpaceModel model;
	model.formState(myNetwork, { bus1 });

	Eigen::MatrixXd A = model.getA(), Aexpected = MatrixXd(1, 1);
	Aexpected <<  -500000.00;
	Eigen::MatrixXd B = model.getB(), Bexpected = MatrixXd(1, 1);
	Bexpected << 500000.00;
	Eigen::MatrixXd C = model.getC(), Cexpected = MatrixXd(1, 1);
	Cexpected << 0.0;
	Eigen::MatrixXd D = model.getD(), Dexpected = MatrixXd(1, 1);
	Dexpected <<  1.0;

	EXPECT_TRUE(A.isApprox(Aexpected, 1e-9));
	EXPECT_TRUE(B.isApprox(Bexpected, 1e-9));
	EXPECT_TRUE(C.isApprox(Cexpected, 1e-9));
	EXPECT_TRUE(D.isApprox(Dexpected, 1e-9));

	delete myNetwork;
}