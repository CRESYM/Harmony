#include <gtest/gtest.h>
#include "Inductor.h"
#include "Bus.h"
#include <symengine/basic.h>


class TestInductor : public testing::Test {};

// Test cases for Inductor class
/*
    //Inductor Single-phase
    Bus* bus_L0 = new Bus("busL0", 1);
    Bus* bus_L1 = new Bus("busL1", 1);

    // Create single-phase inductor with 10 mH
    Inductor* L1 = new Inductor("L1", { bus_L0 }, { bus_L1 }, 10e-3);

    std::vector<Bus*> busesLsp = { bus_L0, bus_L1 };
    std::unordered_map<Bus*, int> idxLsp = { {bus_L0, 0}, {bus_L1, 1} };

    DenseMatrix ALsp(3, 3);  // 2 buses + 1 branch
    for (int i = 0; i < ALsp.nrows(); ++i)
        for (int j = 0; j < ALsp.ncols(); ++j)
            ALsp.set(i, j, zero);

    // Stamp the inductor MNA matrix
    L1->writeMNAmatrix(ALsp, 2, 0, symbol("L1"), idxLsp);

    std::cout << "--- Single-phase Inductor ---\n";
    std::cout << "ALsp(2,0)=" << *ALsp.get(2, 0) << "  ALsp(0,2)=" << *ALsp.get(0, 2) << "\n";
    std::cout << "ALsp(2,1)=" << *ALsp.get(2, 1) << "  ALsp(1,2)=" << *ALsp.get(1, 2) << "\n";


    // Inductor three-phase
    Bus* la1 = new Bus("la1", 1);
    Bus* la2 = new Bus("la2", 1);
    Bus* la3 = new Bus("la3", 1);
    Bus* lb1 = new Bus("lb1", 1);
    Bus* lb2 = new Bus("lb2", 1);
    Bus* lb3 = new Bus("lb3", 1);

    std::vector<Bus*> l_from = { la1, la2, la3 };
    std::vector<Bus*> l_to = { lb1, lb2, lb3 };

    // Create three-phase inductor with 5 mH
    Inductor* L3 = new Inductor("L3", l_from, l_to, 5e-3);

    std::vector<Bus*> busesLmp = { la1, la2, la3, lb1, lb2, lb3 };
    std::unordered_map<Bus*, int> idxLmp;
    for (int i = 0; i < 6; ++i) idxLmp[busesLmp[i]] = i;

    DenseMatrix ALmp(9, 9); // 6 buses + 3 branches
    for (int i = 0; i < ALmp.nrows(); ++i)
        for (int j = 0; j < ALmp.ncols(); ++j)
            ALmp.set(i, j, zero);

    // Stamp the inductor
    L3->writeMNAmatrix(ALmp, 6, 0, symbol("L3"), idxLmp);

    std::cout << "\nThree-phase Inductor:\n";
    for (int p = 0; p < 3; ++p) {
        int row = 6 + p;
        int n1 = idxLmp[l_from[p]];
        int n2 = idxLmp[l_to[p]];
        std::cout << "phase " << p << ":\n";
        std::cout << "  A(" << row << "," << n1 << ") = " << *ALmp.get(row, n1) << "\n";
        std::cout << "  A(" << row << "," << n2 << ") = " << *ALmp.get(row, n2) << "\n";
        std::cout << "  A(" << n1 << "," << row << ") = " << *ALmp.get(n1, row) << "\n";
        std::cout << "  A(" << n2 << "," << row << ") = " << *ALmp.get(n2, row) << "\n";
        std::cout << "  A(" << row << "," << row << ") = " << *ALmp.get(row, row) << "\n";
    }
*/

TEST_F(TestInductor, ConstructorSinglePhase)
{
    Bus* n1 = new Bus("n1", 1);
    Bus* n2 = new Bus("n2", 1);

    Inductor L1("L1", { n1 }, { n2 }, 10e-3);      // 10 mH

    EXPECT_EQ(L1.getElementSymbol(), "L1");
    EXPECT_EQ(L1.getInputPins(), 1);
    EXPECT_EQ(L1.getOutputPins(), 1);

    delete n1; delete n2;
}

TEST_F(TestInductor, ConstructorThreePhase)
{
    // three line‑to‑line pairs
    Bus* a1 = new Bus("a1", 1), * a2 = new Bus("a2", 1), * a3 = new Bus("a3", 1);
    Bus* b1 = new Bus("b1", 1), * b2 = new Bus("b2", 1), * b3 = new Bus("b3", 1);

    std::vector<Bus*> from = { a1, a2, a3 };
    std::vector<Bus*> to = { b1, b2, b3 };

    Inductor L3("L3", from, to, 5e-3);         // 5 mH / phase

    EXPECT_EQ(L3.getInputPins(), 3);
    EXPECT_EQ(L3.getOutputPins(), 3);
    EXPECT_EQ(L3.getElementSymbol(), "L3");

    delete a1; delete a2; delete a3;
    delete b1; delete b2; delete b3;
}

TEST_F(TestInductor, WriteMNAMatrixSinglePhase)
{
    Bus* n1 = new Bus("n1", 1);
    Bus* n2 = new Bus("n2", 1);

    Inductor* L = new Inductor("L1", { n1 }, { n2 }, 10e-3);

    // node index map
    std::unordered_map<Bus*, int> idx{ {n1,0},{n2,1} };

    // 2 node eqns + 1 branch current
    DenseMatrix A(3, 3);
    for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j) A.set(i, j, zero);

    L->writeMNAmatrix(A, /*num_eqns=*/2, /*firstBranchIndex=*/0,
        symbol("L1"), idx);

    // Incidence (+1, -1) should be present
    EXPECT_FALSE(eq(*A.get(2, 0), *zero));
    EXPECT_FALSE(eq(*A.get(2, 1), *zero));
    EXPECT_FALSE(eq(*A.get(0, 2), *zero));
    EXPECT_FALSE(eq(*A.get(1, 2), *zero));

    // Branch diagonal (‑sL or similar) should be non‑zero
    EXPECT_FALSE(eq(*A.get(2, 2), *zero));

    delete L;
    delete n1; delete n2;
}

TEST_F(TestInductor, WriteMNAMatrixThreePhase)
{
    Bus* a1 = new Bus("a1", 1), * a2 = new Bus("a2", 1), * a3 = new Bus("a3", 1);
    Bus* b1 = new Bus("b1", 1), * b2 = new Bus("b2", 1), * b3 = new Bus("b3", 1);

    std::vector<Bus*> from = { a1,a2,a3 };
    std::vector<Bus*> to = { b1,b2,b3 };

    Inductor* L3 = new Inductor("L3", from, to, 5e-3);

    // build node‑to‑column map
    std::unordered_map<Bus*, int> idx;
    std::vector<Bus*> all = { a1,a2,a3,b1,b2,b3 };
    for (int i = 0; i < 6; ++i) idx[all[i]] = i;

    DenseMatrix A( /*rows=*/9, /*cols=*/9);
    for (int i = 0; i < 9; ++i) for (int j = 0; j < 9; ++j) A.set(i, j, zero);

    L3->writeMNAmatrix(A, 6, 0,
        symbol("L3"), idx);

    // For each phase, check the four incidence entries are non‑zero
    for (int p = 0; p < 3; ++p) {
        int row = 6 + p;                 // branch equation row
        int n_from = idx[from[p]];
        int n_to = idx[to[p]];

        EXPECT_FALSE(eq(*A.get(row, n_from), *zero));
        EXPECT_FALSE(eq(*A.get(row, n_to), *zero));
        EXPECT_FALSE(eq(*A.get(n_from, row), *zero));
        EXPECT_FALSE(eq(*A.get(n_to, row), *zero));

        // diagonal (‑sL) should also be non‑zero
        EXPECT_FALSE(eq(*A.get(row, row), *zero));
    }

    delete L3;
    delete a1; delete a2; delete a3;
    delete b1; delete b2; delete b3;
}
