#include <gtest/gtest.h>

#include "../src/Solver/OPF/Powerflow.h"
#include "../src/network.h"


TEST(OPFSmoke, LoadCaseDataDoesNotCrash) {
	PowerFlow pf;
	EXPECT_NO_THROW(pf.create_ac("ac9ac14"));
	EXPECT_NO_THROW(pf.create_dc("mtdc3slack_a"));
}

TEST(OPFSmoke, MakeOPFOnEmptyNetworkDoesNotCrash) {
	Network net;
	PowerFlow pf;

	std::map<std::string, double> globalParams;
	globalParams["baseMVA"] = 100.0;
	globalParams["ACbaseKV"] = 345.0;
	globalParams["DCbaseKV"] = 400.0;

	auto dataAc = pf.create_ac("ac9ac14");
	auto dataDc = pf.create_dc("mtdc3slack_a");
	pf.load_params_ac("AC1", dataAc);
	pf.load_params_dc("DC1", dataDc);

	EXPECT_NO_THROW(pf.make_OPF(&net, globalParams, true, false, false, false));
}
