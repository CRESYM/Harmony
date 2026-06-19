#include <gtest/gtest.h>
#include "network.h"
#include "Bus.h"
#include "Include_components.h"

class TestNetwork : public testing::Test {};

TEST_F(TestNetwork, TestSimpleNetwork) {
	// Create a simple network with one bus and one component
	Impedance* z = new Impedance("z1", "AC1", 1, DenseMatrix(1, 1, { div(integer(1), mul(j, omega)) }));
	AC_source* ac = new AC_source("ac", "AC1", 1, 345e3, DenseMatrix(1, 1, { integer(10) }));

	Network* myNetwork = new Network();

	// Create Bus objects
	Bus* b1 = new Bus("Bus1", "AC1", 1);
	Bus* gnd = new Bus("gnd", "AC1", 1);

	// Add elements to the network
	myNetwork->addElement(z);
	myNetwork->addElement(ac);

	// Add buses to the network
	myNetwork->addBus("Bus1", b1);
	myNetwork->addBus("gnd", gnd);


	// Connect elements to buses
	myNetwork->connectElementToBus(ac, 1, b1);
	myNetwork->connectElementToBus(ac, 2, gnd);
	myNetwork->connectElementToBus(z, 1, b1);
	myNetwork->connectElementToBus(z, 2, gnd);

	// Print the connections to verify the network
	myNetwork->printConnections();

	// Check if the bus and component are added correctly
	EXPECT_EQ(myNetwork->getBuses().size(), 2);
	EXPECT_EQ(myNetwork->getElements().size(), 2);
	EXPECT_EQ(z->getBuses().size(), 2);
	EXPECT_EQ(ac->getBuses().size(), 2);
	//EXPECT_EQ(ac->getBuses()[0], b1);

	delete myNetwork;
}

