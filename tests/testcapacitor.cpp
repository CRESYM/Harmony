#include <gtest/gtest.h>
#include "Capacitor.h"
#include "Bus.h"
#include <symengine/basic.h>  


using namespace SymEngine;

class TestCapacitor : public testing::Test {};


TEST_F(TestCapacitor, TestConstructorSinglePhase) {
    Bus* bus0 = new Bus("bus0", "AC1", 1);
    Bus* bus1 = new Bus("bus1", "AC1", 1);

    // Single-phase capacitor constructor: 1 node pair, single capacitance
    Capacitor C1("C1", "AC1", 1, { 1e-6 });
	C1.attachBus(bus0, 1); // Attach first bus
	C1.attachBus(bus1, 2); // Attach second bus

    EXPECT_EQ(C1.getInputPins(), 1); 
    EXPECT_EQ(C1.getOutputPins(), 1);
    EXPECT_EQ(C1.getElementSymbol(), "C1");

    delete bus0;
    delete bus1;
}

TEST_F(TestCapacitor, TestConstructorMultiPhase) {
    Bus* a1 = new Bus("a1", "AC1", 3);
    Bus* a2 = new Bus("a2", "AC1", 3);

    // Multi-phase capacitor constructor: 3 node pairs, single capacitance
    Capacitor C3("C3", "AC1", 3, { 2e-6 });
	C3.attachBus(a1, 1); // Attach first bus
	C3.attachBus(a2, 2); // Attach second bus

    EXPECT_EQ(C3.getInputPins(), 3);
    EXPECT_EQ(C3.getOutputPins(), 3);
    EXPECT_EQ(C3.getElementSymbol(), "C3");

    delete a1; delete a2; 
}

