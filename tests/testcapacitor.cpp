#include <gtest/gtest.h>
#include "Capacitor.h"
#include "Bus.h"
#include <symengine/basic.h>  


using namespace SymEngine;

class TestCapacitor : public testing::Test {};

// Test cases for Capacitor class
/*
    //Capacitor single-phase
    Bus* bus_0 = new Bus("bus0",1);
    Bus* bus_1 = new Bus("bus1",1);
    Capacitor* C1 = new Capacitor("C1", { bus_0 }, { bus_1 }, 1e-6);

    std::vector<Bus*> busesSp = { bus_0, bus_1 };
    std::unordered_map<Bus*, int> idxSp = { {bus_0,0},{bus_1,1} };
    DenseMatrix Asp(3, 3);                       // 2 nodes + 1 branch
    for (int i = 0; i < Asp.nrows(); ++i) for (int j = 0; j < Asp.ncols(); ++j) Asp.set(i, j, zero);

    C1->writeMNAmatrix(Asp, 2, 0, symbol("C1"), idxSp);
    std::cout << "single phase:\n";
    std::cout << "Asp(2,0)=" << *Asp.get(2, 0) << "  Asp(0,2)=" << *Asp.get(0, 2) << "\n";
    std::cout << "Asp(2,1)=" << *Asp.get(2, 1) << "  Asp(1,2)=" << *Asp.get(1, 2) << "\n";

    //capacitor three‑phase
    Bus* a_1 = new Bus("a_1",1), * a_2 = new Bus("a_2",1), * a_3 = new Bus("a_3",1);
    Bus* b_1 = new Bus("b_1",1), * b_2 = new Bus("b_2",1), * b_3 = new Bus("b_3",1);
    std::vector<Bus*> from = { a_1,a_2,a_3 };
    std::vector<Bus*> to = { b_1,b_2,b_3 };

    Capacitor* C3 = new Capacitor("C3", from, to, 2e-6);

    std::vector<Bus*> busesMp = { a_1,a_2,a_3,b_1,b_2,b_3 };
    std::unordered_map<Bus*, int> idxMp;
    for (int i = 0; i < 6; ++i) idxMp[busesMp[i]] = i;

    DenseMatrix Amp(9, 9);                       // 6 nodes + 3 branches
    for (int i = 0; i < Amp.nrows(); ++i) for (int j = 0; j < Amp.ncols(); ++j) Amp.set(i, j, zero);

    C3->writeMNAmatrix(Amp, 6, 0, symbol("C3"), idxMp);

    std::cout << "\nthree phase:\n";
    for (int p = 0; p < 3; ++p) {
        int row = 6 + p;
        int n1 = idxMp[from[p]];
        int n2 = idxMp[to[p]];
        std::cout << "phase " << p << ":  Amp(" << row << "," << n1 << ")=" << *Amp.get(row, n1)
            << "  Amp(" << row << "," << n2 << ")=" << *Amp.get(row, n2) << "\n";
    }
*/

TEST_F(TestCapacitor, TestConstructorSinglePhase) {
    Bus* bus0 = new Bus("bus0", 1);
    Bus* bus1 = new Bus("bus1", 1);

    // Single-phase capacitor constructor: 1 node pair, single capacitance
    Capacitor C1("C1", { bus0 }, { bus1 }, 1e-6);

    EXPECT_EQ(C1.getInputPins(), 1); 
    EXPECT_EQ(C1.getOutputPins(), 1);
    EXPECT_EQ(C1.getElementSymbol(), "C1");

    delete bus0;
    delete bus1;
}

TEST_F(TestCapacitor, TestConstructorMultiPhase) {
    Bus* a1 = new Bus("a1", 1);
    Bus* a2 = new Bus("a2", 1);
    Bus* a3 = new Bus("a3", 1);
    Bus* b1 = new Bus("b1", 1);
    Bus* b2 = new Bus("b2", 1);
    Bus* b3 = new Bus("b3", 1);

    std::vector<Bus*> from = { a1, a2, a3 };
    std::vector<Bus*> to = { b1, b2, b3 };

    // Multi-phase capacitor constructor: 3 node pairs, single capacitance
    Capacitor C3("C3", from, to, 2e-6);

    EXPECT_EQ(C3.getInputPins(), 3);
    EXPECT_EQ(C3.getOutputPins(), 3);
    EXPECT_EQ(C3.getElementSymbol(), "C3");

    delete a1; delete a2; delete a3;
    delete b1; delete b2; delete b3;
}

TEST_F(TestCapacitor, TestWriteMNAmatrixSinglePhase) {
    Bus* bus0 = new Bus("bus0", 1);
    Bus* bus1 = new Bus("bus1", 1);

    Capacitor C1("C1", { bus0 }, { bus1 }, 1e-6);

    std::vector<Bus*> buses = { bus0, bus1 };
    std::unordered_map<Bus*, int> idx = { {bus0, 0}, {bus1, 1} };

    DenseMatrix A(3, 3);
    for (int i = 0; i < A.nrows(); ++i)
        for (int j = 0; j < A.ncols(); ++j)
            A.set(i, j, zero);

    C1.writeMNAmatrix(A, 2, 0, symbol("C1"), idx);

    // Compare using SymEngine::eq 
    EXPECT_FALSE(eq(*A.get(2, 0), *zero));
    EXPECT_FALSE(eq(*A.get(2, 1), *zero));
    EXPECT_FALSE(eq(*A.get(0, 2), *zero));
    EXPECT_FALSE(eq(*A.get(1, 2), *zero));

    delete bus0;
    delete bus1;
}