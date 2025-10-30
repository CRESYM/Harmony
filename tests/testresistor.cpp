#include <gtest/gtest.h>
#include "Resistor.h"
#include "Bus.h"
#include "Constants.h"

#include <symengine/basic.h>

using namespace SymEngine;         // gives integer(), div(), mul(), zero, eq()

class TestResistor : public ::testing::Test {};

// Test cases for Resistor class
/*
    // Single-phase resistor with R=10 Ohms
    std::cout << "Resistor Testing=" << '\n';
    Resistor* r1 = new Resistor("R1", 1, 10.0);
    r1->printElementValues();

    // Three-phase resistor with same R=5 Ohms for all phases
    Resistor* r2 = new Resistor("R2", 3, 5.0);
    r2->printElementValues();

    // Three-phase resistor with different values
    std::vector<double> Rs = { 1.0, 2.0, 3.0 };
    Resistor* r3 = new Resistor("R3", 3, Rs);
    r3->printElementValues();
*/

TEST_F(TestResistor, ConstructorSinglePhase) {
    Resistor R1("R1", "AC1", 1, { 10.0 });
    const auto& Y = R1.get_y_parameters();
    ASSERT_EQ(Y.nrows(), 2);         // 2x2 matrix
    ASSERT_EQ(Y.ncols(), 2);

    EXPECT_NEAR(eval_basic(Y.get(0,0)), 0.1, 1e-9);
    EXPECT_NEAR(eval_basic(Y.get(1,1)), 0.1, 1e-9);
}

TEST_F(TestResistor, ConstructorThreePhaseSameR) {
    Resistor R2("R2", "AC1", 3, { 5.0 });
    const auto& Y = R2.get_y_parameters();
    ASSERT_EQ(Y.nrows(), 6);         // 3 phases → 6x6
    ASSERT_EQ(Y.ncols(), 6);

    for (int p = 0; p < 3; ++p) {
        EXPECT_NEAR(eval_basic(Y.get(p,p)), 0.2, 1e-9);           // top-left
        EXPECT_NEAR(eval_basic(Y.get(p + 3,p + 3)), 0.2, 1e-9);   // bottom-right
    }
}

TEST_F(TestResistor, ConstructorThreePhaseDifferentR) {
    std::vector<double> Rs = { 1.0, 2.0, 4.0 };
    Resistor R3("R3", "AC1", 3, Rs);
    const auto& Y = R3.get_y_parameters();
    ASSERT_EQ(Y.nrows(), 6);
    ASSERT_EQ(Y.ncols(), 6);

    EXPECT_NEAR(eval_basic(Y.get(0,0)), 1.0, 1e-9);
    EXPECT_NEAR(eval_basic(Y.get(1,1)), 0.5, 1e-9);
    EXPECT_NEAR(eval_basic(Y.get(2,2)), 0.25, 1e-9);
    EXPECT_NEAR(eval_basic(Y.get(3,3)), 1.0, 1e-9);
    EXPECT_NEAR(eval_basic(Y.get(4,4)), 0.5, 1e-9);
    EXPECT_NEAR(eval_basic(Y.get(5,5)), 0.25, 1e-9);
}

