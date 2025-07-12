#include <gtest/gtest.h>
#include "Inductor.h"
#include "Bus.h"
#include <symengine/basic.h>


class TestInductor : public testing::Test {};


TEST_F(TestInductor, ConstructorSinglePhase)
{
    Bus* n1 = new Bus("n1", 1);
    Bus* n2 = new Bus("n2", 1);

    Inductor L1("L1", 1, { 10e-3 });      // 10 mH
	L1.attachBus(n1, 1);               // attach to n1
	L1.attachBus(n2, 2);               // attach to n2

    EXPECT_EQ(L1.getElementSymbol(), "L1");
    EXPECT_EQ(L1.getInputPins(), 1);
    EXPECT_EQ(L1.getOutputPins(), 1);

    delete n1; delete n2;
}

TEST_F(TestInductor, ConstructorThreePhase)
{
    // three line‑to‑line pairs
    Bus* a1 = new Bus("a1", 3), * a2 = new Bus("a2", 3);

    Inductor L3("L3", 3, { 5e-3 });         // 5 mH / phase
	L3.attachBus(a1, 1);               // attach to a1
	L3.attachBus(a2, 2);               // attach to a2

    EXPECT_EQ(L3.getInputPins(), 3);
    EXPECT_EQ(L3.getOutputPins(), 3);
    EXPECT_EQ(L3.getElementSymbol(), "L3");

    delete a1; delete a2; 
}


