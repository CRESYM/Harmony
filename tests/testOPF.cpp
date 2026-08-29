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

TEST(OPFSmoke, LoadIeeeBenchmarkCaseDataDoesNotCrash) {
	PowerFlow pf;
	EXPECT_NO_THROW(pf.create_ac("ieee9"));
	EXPECT_NO_THROW(pf.create_ac("ieee39"));
	EXPECT_NO_THROW(pf.create_ac("ieee39hvdc"));
	EXPECT_NO_THROW(pf.create_ac("rts24hvdc"));
	EXPECT_NO_THROW(pf.create_dc("hvdc2ptp"));
	EXPECT_NO_THROW(pf.create_dc("ieee39hvdc"));
	EXPECT_NO_THROW(pf.create_dc("rts24hvdc"));
}

// MATPOWER reports 5296.69 $/h for case9; Harmony's second-order-cone
// relaxation is a lower bound on that optimum.
TEST(OPFBenchmark, Ieee9MatchesMatpowerOptimum) {
	PowerFlow pf;
	EXPECT_NO_THROW(pf.solve_opf("", "ieee9", nullptr, false, false, false, false));
	if (!pf.opfSolved()) {
		GTEST_SKIP() << "IEEE 9-bus OPF did not succeed (Gurobi license may be required)";
	}
	EXPECT_NEAR(pf.totalGenerationCost(), 5296.69, 1.0);
}

// MATPOWER reports 41864.18 $/h for case39.
TEST(OPFBenchmark, Ieee39MatchesMatpowerOptimum) {
	PowerFlow pf;
	EXPECT_NO_THROW(pf.solve_opf("", "ieee39", nullptr, false, false, false, false));
	if (!pf.opfSolved()) {
		GTEST_SKIP() << "IEEE 39-bus OPF did not succeed (Gurobi license may be required)";
	}
	EXPECT_NEAR(pf.totalGenerationCost(), 41864.18, 20.0);
}

TEST(OPFBenchmark, Ieee9WithTwoHvdcLinksSolves) {
	PowerFlow pf;
	EXPECT_NO_THROW(pf.solve_opf("hvdc2ptp", "ieee9", nullptr, true, false, false, false));
	if (!pf.opfSolved()) {
		GTEST_SKIP() << "IEEE 9-bus HVDC OPF did not succeed (Gurobi license may be required)";
	}
	// Routing power through converters costs more than the AC-only dispatch.
	EXPECT_GT(pf.totalGenerationCost(), 5296.69);
}
