/**
 * @file example_registry.cpp
 * @brief Maps Harmony --cpp names to example entry points.
 */
#include "examples/example_registry.h"
#include "examples/Examples.h"

std::map<std::string, ExampleFn> harmonyCppExampleRegistry() {
	std::map<std::string, ExampleFn> registry;
	auto add = [&](const char* name, ExampleFn fn) { registry[name] = std::move(fn); };

	add("opf_single_area", example_OPF_single_area);
	add("opf_csv", example_OPF_csv);
	add("opf_ac", example_OPF_ac);
	add("opf_slo", example_OPF_SLO);
	add("opf_double_area", example_OPF_double_area);
	add("opf_csv_1", example_OPF_csv_1);
	add("opf_ieee9", example_OPF_ieee9);
	add("opf_ieee9_hvdc", example_OPF_ieee9_hvdc);
	add("opf_ieee39", example_OPF_ieee39);
	add("opf_ieee39_hvdc", example_OPF_ieee39_hvdc);
	add("opf_rts24_hvdc", example_OPF_rts24_hvdc);
	add("opf_pv", example_OPF_PV);
	add("opf_wt", example_OPF_WT);
	add("dqsym_math_operations", example_DQsym_math_operations);
	add("dqsym_dsss2", example_DQsym_DSSS2);
	add("dqsym_rlc", example_DQsym_RLC);
	add("dqsym_simple_mmc", example_DQsym_Simple_MMC);
	add("dqsym_mmc_controlled", [](bool plotting_enabled) { example_DQsym_MMC_controlled(plotting_enabled); });
	add("state_space", [](bool) { example_state_space(); });
	add("generator", example_generator);
	add("mmc", example_MMC);
	add("mmc_gfm", example_MMC_gfm);
	add("certificate_figures", example_certificate_figures);
	add("wt_type_3", example_WT_type_3);
	add("wt_type_4", example_WT_type_4);
	add("pv_plant", example_PV_plant);
	add("ohl", example_OHL);
	add("cable", example_cable);
	add("transformer", [](bool) { example_transformer(); });
	add("constructors", [](bool) { example_constructors(); });
	add("visuals", example_visuals);
	add("stability_check", example_stability_check);
	add("admittance_parameters", [](bool) { example_admittance_parameters(); });
	add("point2point_case", [](bool) { example_point2point_case(); });
	add("pll_test", [](bool) { example_PLL_test(); });
	return registry;
}
