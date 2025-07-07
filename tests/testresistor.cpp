#include <gtest/gtest.h>
#include "Resistor.h"
#include "Bus.h"

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
    Resistor R1("R1", 1, 10.0);
    const auto& Y = R1.getYMatrix();
    ASSERT_EQ(Y.size(), 2);         // 2x2 matrix
    ASSERT_EQ(Y[0].size(), 2);

    EXPECT_NEAR(Y[0][0], 0.1, 1e-9);
    EXPECT_NEAR(Y[1][1], 0.1, 1e-9);
}

TEST_F(TestResistor, ConstructorThreePhaseSameR) {
    Resistor R2("R2", 3, 5.0);
    const auto& Y = R2.getYMatrix();
    ASSERT_EQ(Y.size(), 6);         // 3 phases → 6x6
    ASSERT_EQ(Y[0].size(), 6);

    for (int p = 0; p < 3; ++p) {
        EXPECT_NEAR(Y[p][p], 0.2, 1e-9);           // top-left
        EXPECT_NEAR(Y[p + 3][p + 3], 0.2, 1e-9);   // bottom-right
    }
}

TEST_F(TestResistor, ConstructorThreePhaseDifferentR) {
    std::vector<double> Rs = { 1.0, 2.0, 4.0 };
    Resistor R3("R3", 3, Rs);
    const auto& Y = R3.getYMatrix();
    ASSERT_EQ(Y.size(), 6);
    ASSERT_EQ(Y[0].size(), 6);

    EXPECT_NEAR(Y[0][0], 1.0, 1e-9);
    EXPECT_NEAR(Y[1][1], 0.5, 1e-9);
    EXPECT_NEAR(Y[2][2], 0.25, 1e-9);
    EXPECT_NEAR(Y[3][3], 1.0, 1e-9);
    EXPECT_NEAR(Y[4][4], 0.5, 1e-9);
    EXPECT_NEAR(Y[5][5], 0.25, 1e-9);
}

// Single-phase writeMNAmatrix test
TEST_F(TestResistor, WriteMNAMatrixSinglePhase) {
    // nodes
    Bus* bus0 = new Bus("bus0", 1);
    Bus* bus1 = new Bus("bus1", 1);

    // Create a 2-pin resistor with resistance 10 Ohms
    Resistor* R = new Resistor("R", 2, 10.0);

    // Connect the resistor to bus0 and bus1
    R->attachBus(bus0, 1);  // inherited from Element
    R->attachBus(bus1, 1);

    // A =  2 node eqns  +  1 branch current eqn
    DenseMatrix A(3, 3);
    for (int i = 0; i < A.nrows(); ++i)
        for (int j = 0; j < A.ncols(); ++j)
            A.set(i, j, zero);

    std::unordered_map<Bus*, int> idx = {
     {bus0, 0},
     {bus1, 1}
    };

    // Write MNA contribution of resistor
    RCP<const Basic> val = symbol("R");
    R->writeMNAmatrix(A, 2, 0, val, idx);

    // Check that off-diagonal and diagonal entries are properly filled
    EXPECT_FALSE(eq(*A.get(0, 0), *zero));
    EXPECT_FALSE(eq(*A.get(0, 1), *zero));
    EXPECT_FALSE(eq(*A.get(1, 0), *zero));
    EXPECT_FALSE(eq(*A.get(1, 1), *zero));

    delete R;
    delete bus0;
    delete bus1;
}
