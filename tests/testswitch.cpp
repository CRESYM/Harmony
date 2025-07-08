#include <gtest/gtest.h>
#include "Switch.h"
#include "Bus.h"
#include <symengine/basic.h>

using namespace SymEngine;   

class TestSwitch : public ::testing::Test {};
/*
* 	//switch single-phase 
	Bus* a = new Bus("a",1);
	Bus* b = new Bus("b",1);

	Switch* sw = new Switch("SW1", { a }, { b }, { true }); //closed

	std::vector<Bus*> buses1 = { a, b };
	std::unordered_map<Bus*, int> busIndex1 = { {a,0},{b,1} };

	int num_eq = static_cast<int>(buses1.size());
	DenseMatrix A(num_eq + 1, num_eq + 1);      // +1 auxiliary row

	sw->writeMNAmatrix(A, num_eq, 1, symbol("SW1"), busIndex1);

	std::cout << "A(2,0)=" << *A.get(2, 0) << "  A(0,2)=" << *A.get(0, 2) << '\n'; 
	std::cout << "A(2,1)=" << *A.get(2, 1) << "  A(1,2)=" << *A.get(1, 2) << '\n'; 

    //switch three-phase 
	Bus* a1 = new Bus("a1", 1);
	Bus* a2 = new Bus("a2", 1);
	Bus* a3 = new Bus("a3", 1);
	Bus* b1 = new Bus("b1", 1);
	Bus* b2 = new Bus("b2", 1);
	Bus* b3 = new Bus("b3", 1);

	std::vector<Bus*> node1s = { a1, a2, a3 };
	std::vector<Bus*> node2s = { b1, b2, b3 };
	std::vector<bool> phaseStates = { true, false, true };  // for example: phase 0 and 2 closed, phase 1 open

	Switch* sw3p = new Switch("SW3P", node1s, node2s, phaseStates);

	std::vector<Bus*> buses3p = { a1, a2, a3, b1, b2, b3 };
	std::unordered_map<Bus*, int> busIndex3p;
	for (int i = 0; i < 6; ++i) {
		busIndex3p[buses3p[i]] = i;
	}

    int num_eq_3p = static_cast<int>(buses3p.size());
	DenseMatrix A3p(num_eq_3p + 3, num_eq_3p + 3);  // +3 auxiliary rows, one per phase
	
	// Initialize A3p to zero
	for (int i = 0; i < A3p.nrows(); ++i)
		for (int j = 0; j < A3p.ncols(); ++j)
			A3p.set(i, j, zero);

	
	sw3p->writeMNAmatrix(A3p, num_eq_3p, 1, symbol("SW3P"), busIndex3p);

	std::cout << "\nThree-phase switch matrix entries:\n";

	for (int p = 0; p < 3; ++p) {
		int aux_row = num_eq_3p + p;   // auxiliary row for phase p
		int idx_n1 = busIndex3p[node1s[p]];
		int idx_n2 = busIndex3p[node2s[p]];

		std::cout << "Diagonal A(aux_row=" << aux_row << ", aux_row=" << aux_row << ") = " << *A3p.get(aux_row, aux_row) << "\n";
		std::cout << "A(aux_row=" << aux_row << ", n1=" << idx_n1 << ") = " << *A3p.get(aux_row, idx_n1) << "\n";
		std::cout << "A(n1=" << idx_n1 << ", aux_row=" << aux_row << ") = " << *A3p.get(idx_n1, aux_row) << "\n";
		std::cout << "A(aux_row=" << aux_row << ", n2=" << idx_n2 << ") = " << *A3p.get(aux_row, idx_n2) << "\n";
		std::cout << "A(n2=" << idx_n2 << ", aux_row=" << aux_row << ") = " << *A3p.get(idx_n2, aux_row) << "\n";


		// For open phase (phase 1), expect different diagonal entries on auxiliary row
		if (!phaseStates[p]) {
			std::cout << "Diagonal A(aux_row=" << aux_row << ", aux_row=" << aux_row << ") = " << *A3p.get(aux_row, aux_row) << "\n";
		}
	}

	// Print full matrix
	std::cout << "\n--- MNA Matrix A3p (9x9) ---\n";
	for (int i = 0; i < A3p.nrows(); ++i) {
		for (int j = 0; j < A3p.ncols(); ++j) {
			std::cout << A3p.get(i, j)->__str__() << "\t";
		}
		std::cout << "\n";
	}
*/

TEST_F(TestSwitch, MNASinglePhaseClosed)
{
    Bus a("a", 1), b("b", 1);

    Switch sw("SW1", { &a }, { &b }, { true });          // closed

    std::unordered_map<Bus*, int> idx{ {&a,0},{&b,1} };
    DenseMatrix A(3, 3);                           // 2 nodes + 1 branch
    for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j) A.set(i, j, zero);

    sw.writeMNAmatrix(A, /*num_eq=*/2, /*firstBranchIndex=*/0,
        symbol("SW1"), idx);

    EXPECT_TRUE(eq(*A.get(2, 0), *one));
    EXPECT_TRUE(eq(*A.get(2, 1), *minus_one));
    EXPECT_TRUE(eq(*A.get(0, 2), *one));
    EXPECT_TRUE(eq(*A.get(1, 2), *minus_one));
    EXPECT_TRUE(eq(*A.get(2, 2), *zero));         // ideal short → diag 0
}

TEST_F(TestSwitch, MNAThreePhaseMixed)
{
    // buses
    Bus a1("a1", 1), a2("a2", 1), a3("a3", 1);
    Bus b1("b1", 1), b2("b2", 1), b3("b3", 1);

    std::vector<Bus*> n1s{ &a1,&a2,&a3 };
    std::vector<Bus*> n2s{ &b1,&b2,&b3 };
    std::vector<bool>  state{ true, false, true };

    Switch sw3("SW3P", n1s, n2s, state);

    // node map
    std::unordered_map<Bus*, int> idx{
        {&a1,0},{&a2,1},{&a3,2},
        {&b1,3},{&b2,4},{&b3,5}
    };

    DenseMatrix A(9, 9);                       // 6 nodes + 3 branches
    for (int i = 0; i < 9; ++i) for (int j = 0; j < 9; ++j) A.set(i, j, zero);

    sw3.writeMNAmatrix(A, 6, 0, symbol("SW3P"), idx);

    // Phase‑0 (closed) rows / cols
    EXPECT_TRUE(eq(*A.get(6, 0), *one));
    EXPECT_TRUE(eq(*A.get(6, 3), *minus_one));
    EXPECT_TRUE(eq(*A.get(0, 6), *one));
    EXPECT_TRUE(eq(*A.get(3, 6), *minus_one));
    EXPECT_TRUE(eq(*A.get(6, 6), *zero));

    // Phase‑1 (open) should leave the entire 7‑th row & col zero
    for (int c = 0; c < 9; ++c)  EXPECT_TRUE(eq(*A.get(7, c), *zero));
    for (int r = 0; r < 9; ++r)  EXPECT_TRUE(eq(*A.get(r, 7), *zero));

    // Phase‑2 (closed)
    EXPECT_TRUE(eq(*A.get(8, 2), *one));
    EXPECT_TRUE(eq(*A.get(8, 5), *minus_one));
    EXPECT_TRUE(eq(*A.get(2, 8), *one));
    EXPECT_TRUE(eq(*A.get(5, 8), *minus_one));
}
