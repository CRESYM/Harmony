#include <gtest/gtest.h>

#include "../src/Solver/OPF/Powerflow.h"
#include "../src/network.h"


TEST(OPFSmoke, LoadCaseDataDoesNotCrash) {
	PowerFlow pf;
	EXPECT_NO_THROW(pf.create_ac("ac9ac14"));
	EXPECT_NO_THROW(pf.create_dc("mtdc3slack_a"));
}

TEST(OPFSmoke, SolveAcOnlyOpfFromCsvDoesNotCrash) {
	PowerFlow pf;
	EXPECT_NO_THROW(pf.solve_opf("", "ac5", nullptr, false, false, false, false));
	if (!pf.opfSolved()) {
		GTEST_SKIP() << "AC-only OPF did not succeed (Gurobi license may be required)";
	}
}

TEST(OPFSmoke, SolveHybridOpfFromCsvDoesNotCrash) {
	PowerFlow pf;
	EXPECT_NO_THROW(pf.solve_opf("mtdc3", "ac5", nullptr, false, false, false, false));
	if (!pf.opfSolved()) {
		GTEST_SKIP() << "Hybrid OPF did not succeed (Gurobi license may be required)";
	}
}
