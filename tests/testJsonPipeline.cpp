#include <gtest/gtest.h>

#include <algorithm>
#include <fstream>
#include <filesystem>
#include <vector>

#include "../src/cli.h"
#include "../src/json/simulation_builder.h"
#include "../src/network.h"


namespace fs = std::filesystem;


static fs::path harmonyRoot() {
	return fs::path(__FILE__).parent_path().parent_path();
}


/** CSV OPF reads ../../src/data relative to build/Release (same as C++ examples). */
class HarmonyReleaseCwd {
public:
	HarmonyReleaseCwd() {
		previous_ = fs::current_path();
		const fs::path release = harmonyRoot() / "build" / "Release";
		if (fs::exists(release)) {
			fs::current_path(release);
		}
	}

	~HarmonyReleaseCwd() {
		fs::current_path(previous_);
	}

private:
	fs::path previous_;
};


static bool needsOpfCsvCwd(const fs::path& path) {
	return path.stem() == "opf_csv";
}


static bool isHeavyJsonRun(const fs::path& path) {
	const std::string stem = path.stem().string();
	return stem == "opf_csv" || stem == "stability_check" || stem == "dqsym_mmc";
}


static bool opfCsvDataAvailable() {
	const fs::path data = harmonyRoot() / "src" / "data";
	return fs::exists(data / "ac5_bus_ac.csv") && fs::exists(data / "mtdc3_bus_dc.csv");
}


static std::vector<fs::path> jsonExampleFiles() {
	std::vector<fs::path> files;

	const fs::path jsonDir = harmonyRoot() / "src" / "examples" / "json";
	if (fs::exists(jsonDir)) {
		for (const auto& entry : fs::directory_iterator(jsonDir)) {
			if (entry.is_regular_file() && entry.path().extension() == ".json") {
				files.push_back(entry.path());
			}
		}
	}

	const fs::path legacy = harmonyRoot() / "src" / "examples" / "example.json";
	if (fs::exists(legacy)) {
		files.push_back(legacy);
	}

	std::sort(files.begin(), files.end());
	return files;
}


static JSON loadJsonFile(const fs::path& path) {
	std::ifstream in(path);
	if (!in) {
		throw std::runtime_error("unable to open " + path.string());
	}
	return JSON::parse(in);
}


class JsonExampleFileTest : public testing::TestWithParam<fs::path> {};


TEST_P(JsonExampleFileTest, ValidatesAndBuildsNetwork) {
	const fs::path path = GetParam();
	ASSERT_TRUE(fs::exists(path)) << path;

	const JSON config = loadJsonFile(path);
	SimulationBuilder builder;

	EXPECT_NO_THROW(builder.validateJSON(config));

	Network network;
	EXPECT_NO_THROW(builder.buildFromJSON(config, network));
	EXPECT_FALSE(network.getElements().empty()) << path;
	EXPECT_FALSE(network.getBuses().empty()) << path;
}


INSTANTIATE_TEST_SUITE_P(
	AllJsonExamples,
	JsonExampleFileTest,
	testing::ValuesIn(jsonExampleFiles()),
	[](const testing::TestParamInfo<fs::path>& info) {
		return info.param.stem().string();
	});


TEST(JsonPipelineSmoke, DiscoverAtLeastOneExample) {
	EXPECT_GE(jsonExampleFiles().size(), 1u);
}


TEST(JsonPipelineSmoke, RunAllExamplesViaCli) {
	HarmonyReleaseCwd cwd;
	for (const auto& path : jsonExampleFiles()) {
		if (needsOpfCsvCwd(path) || isHeavyJsonRun(path)) {
			continue;
		}
		EXPECT_EQ(runJsonSimulation(path, false, false), 0) << path;
	}
}


TEST(JsonComplexCases, StabilityCheckValidatesBuildsAndRuns) {
	const fs::path path = harmonyRoot() / "src" / "examples" / "json" / "stability_check.json";
	ASSERT_TRUE(fs::exists(path));

	const JSON config = loadJsonFile(path);
	SimulationBuilder builder;
	EXPECT_NO_THROW(builder.validateJSON(config));

	Network network;
	ASSERT_NO_THROW(builder.buildFromJSON(config, network));
	EXPECT_GE(network.getElements().size(), 7u);
	EXPECT_EQ(runJsonSimulation(path, false, false), 0);
}


TEST(JsonComplexCases, DqsymMmcValidatesBuildsAndRuns) {
	const fs::path path = harmonyRoot() / "src" / "examples" / "json" / "dqsym_mmc.json";
	ASSERT_TRUE(fs::exists(path));

	const JSON config = loadJsonFile(path);
	SimulationBuilder builder;
	EXPECT_NO_THROW(builder.validateJSON(config));

	Network network;
	ASSERT_NO_THROW(builder.buildFromJSON(config, network));
	EXPECT_EQ(runJsonSimulation(path, false, false), 0);
}


TEST(JsonComplexCases, OpfCsvRunsWithCsvData) {
	if (!opfCsvDataAvailable()) {
		GTEST_SKIP() << "src/data CSV cases not found";
	}

	HarmonyReleaseCwd cwd;
	const fs::path path = harmonyRoot() / "src" / "examples" / "json" / "opf_csv.json";
	ASSERT_TRUE(fs::exists(path));

	const JSON config = loadJsonFile(path);
	SimulationBuilder builder;
	EXPECT_NO_THROW(builder.validateJSON(config));

	const int rc = runJsonSimulation(path, false, false);
	if (rc != 0) {
		GTEST_SKIP() << "OPF solve failed (Gurobi license or solver setup may be required)";
	}
	EXPECT_EQ(rc, 0);
}


TEST(JsonComplexCases, ImpedanceComplexFieldBuilds) {
	const JSON config = JSON::parse(R"({
		"simulation": { "title": "z" },
		"buses": [
			{ "id": "b1", "location": "AC1", "pins": 3 },
			{ "id": "b2", "location": "AC1", "pins": 3 }
		],
		"components": [{
			"id": "br1", "type": "impedance", "location": "AC1", "pins": 3,
			"complex": [1.0, 40.0],
			"connected_buses": [
				{ "bus_id": "b1", "terminal": 1 },
				{ "bus_id": "b2", "terminal": 2 }
			]
		}]
	})");

	SimulationBuilder builder;
	EXPECT_NO_THROW(builder.validateJSON(config));
	Network network;
	EXPECT_NO_THROW(builder.buildFromJSON(config, network));
}


TEST(JsonComplexCases, DcSourceSplitVoltageBuilds) {
	const JSON config = JSON::parse(R"({
		"simulation": { "title": "dc" },
		"buses": [
			{ "id": "dc_bus", "location": "DC1", "pins": 2 },
			{ "id": "gnd", "location": "GND", "pins": 1 }
		],
		"components": [{
			"id": "Vs_dc", "type": "dc_source", "location": "DC1", "pins": 2,
			"voltage": [320000.0, -320000.0],
			"resistance": 0.0,
			"connected_buses": [
				{ "bus_id": "dc_bus", "terminal": 1 },
				{ "bus_id": "gnd", "terminal": 2 }
			]
		}]
	})");

	SimulationBuilder builder;
	EXPECT_NO_THROW(builder.validateJSON(config));
	Network network;
	EXPECT_NO_THROW(builder.buildFromJSON(config, network));
}


TEST(JsonComplexCases, StabilityCheckIncludesOpfInfo) {
	const fs::path path = harmonyRoot() / "src" / "examples" / "json" / "stability_check.json";
	ASSERT_TRUE(fs::exists(path));

	const JSON config = loadJsonFile(path);
	bool foundOpfInfo = false;
	for (const auto& comp : config.at("components")) {
		if (comp.value("id", std::string()) == "SRC01" && comp.contains("opf_info")) {
			foundOpfInfo = comp.at("opf_info").value("Ref", 0.0) == 1.0;
			break;
		}
	}
	EXPECT_TRUE(foundOpfInfo);

	SimulationBuilder builder;
	EXPECT_NO_THROW(builder.validateJSON(config));
}


TEST(JsonComplexCases, BuiltNetworkOpfWithoutCaseName) {
	const fs::path path = harmonyRoot() / "src" / "examples" / "json" / "stability_check.json";
	ASSERT_TRUE(fs::exists(path));

	const JSON config = loadJsonFile(path);
	Network network;
	SimulationBuilder builder;
	ASSERT_NO_THROW(builder.buildFromJSON(config, network));
	EXPECT_EQ(builder.runComputationsWithStatus(config, network, false), 0);
}


TEST(JsonComputationPlots, PlotKeysValidate) {
	const JSON config = JSON::parse(R"({
		"simulation": { "title": "plots", "frequency_range": { "start": 1, "end": 100, "points": 5 } },
		"buses": [
			{ "id": "ac", "location": "AC1", "pins": 3 },
			{ "id": "dc", "location": "DC1", "pins": 2 }
		],
		"components": [{
			"id": "MMC1", "type": "mmc", "location": "AC1_DC1",
			"converter_params": [314.0, 1.0, 0.0, 0.0, 1.0, 1.0, 200000.0, 0.05, 1.07, 0.01, 400.0, 0.06, 0.535, 0.00015],
			"connected_buses": [
				{ "bus_id": "ac", "terminal": 1 },
				{ "bus_id": "dc", "terminal": 2 }
			]
		}],
		"computations": [
			{ "type": "y_matrix", "component_id": "MMC1", "plot": true },
			{
				"type": "stability_assessment",
				"converter_id": "MMC1",
				"location": "DC",
				"plot": true,
				"plot_type": "nyquist"
			},
			{ "type": "dqsym", "plot": true, "t_end": 0.001, "output_bus_ids": ["ac"] },
			{ "type": "opf", "case_name": "case", "plot_result": true }
		]
	})");

	SimulationBuilder builder;
	EXPECT_NO_THROW(builder.validateJSON(config));
}


TEST(JsonComputationPlots, RejectsInvalidPlotType) {
	const JSON bad = JSON::parse(R"({
		"simulation": { "title": "t" },
		"buses": [{ "id": "b1", "location": "AC1", "pins": 1 }],
		"components": [{
			"id": "R1", "type": "resistor", "location": "AC1", "pins": 1, "values": [1.0],
			"connected_bus": { "bus_id": "b1", "terminal": 1 }
		}],
		"computations": [
			{ "type": "stability_assessment", "plot": true, "plot_type": "smith" }
		]
	})");

	SimulationBuilder builder;
	EXPECT_THROW(builder.validateJSON(bad), std::invalid_argument);
}


TEST(JsonComputationPlots, YMatrixPlotRunsWhenCliPlotDisabled) {
	const fs::path path = harmonyRoot() / "src" / "examples" / "json" / "passives_rlc.json";
	ASSERT_TRUE(fs::exists(path));

	const JSON config = loadJsonFile(path);
	ASSERT_TRUE(config.contains("computations"));
	bool hasPlotFlag = false;
	for (const auto& calc : config.at("computations")) {
		if (calc.value("type", std::string()) == "y_matrix" && calc.value("plot", false)) {
			hasPlotFlag = true;
			break;
		}
	}
	EXPECT_TRUE(hasPlotFlag);

	Network network;
	SimulationBuilder builder;
	ASSERT_NO_THROW(builder.buildFromJSON(config, network));
	EXPECT_EQ(builder.runComputationsWithStatus(config, network, false), 0);
}


TEST(JsonPipelineSmoke, ResolveJsonByFilename) {
	const auto paths = buildJsonSearchPaths({}, {});
	const fs::path resolved = resolveJsonPath("example.json", paths);
	EXPECT_TRUE(fs::exists(resolved)) << resolved;
}


TEST(JsonSearchPaths, RepoRootIsDetectedFromWorkingDirectory) {
	const auto root = harmonyRepoRoot();
	ASSERT_TRUE(root.has_value()) << "Run tests from the Harmony repository tree";
	EXPECT_TRUE(fs::exists(*root / "src/examples/example.json"));
}


TEST(JsonSearchPaths, BuiltinDefaultsIncludeExamplesJson) {
	const auto paths = builtinJsonSearchPaths();
	ASSERT_GE(paths.size(), 2u);
	EXPECT_EQ(paths.front(), "src/examples");
	EXPECT_NE(std::find(paths.begin(), paths.end(), fs::path("src/examples/json")), paths.end());
}


TEST(JsonSearchPaths, JsonPathOverridesReplaceDefaults) {
	const auto paths = buildJsonSearchPaths({ "custom/dir" }, {});
	ASSERT_EQ(paths.size(), 2u);
	EXPECT_EQ(paths.front(), "custom/dir");
	EXPECT_EQ(paths.back(), ".");
}


TEST(JsonSearchPaths, SearchPathAppendsToDefaults) {
	const auto paths = buildJsonSearchPaths({}, { "extra/dir" });
	EXPECT_EQ(paths.back(), "extra/dir");
	EXPECT_NE(std::find(paths.begin(), paths.end(), fs::path("src/examples")), paths.end());
}


TEST(JsonSearchPaths, JsonPathOverridesTakePrecedenceOverExtras) {
	const auto paths = buildJsonSearchPaths({ "only/here" }, { "also/here" });
	ASSERT_EQ(paths.size(), 3u);
	EXPECT_EQ(paths.front(), "only/here");
	EXPECT_EQ(paths[1], "also/here");
	EXPECT_EQ(paths.back(), ".");
}


TEST(JsonValidatorPins, MmcAndResTypesDoNotRequirePins) {
	const JSON mmc = JSON::parse(R"({
		"simulation": { "title": "t" },
		"buses": [
			{ "id": "ac", "location": "AC1", "pins": 3 },
			{ "id": "dc", "location": "DC1", "pins": 2 }
		],
		"components": [{
			"id": "MMC1", "type": "mmc", "location": "AC1_DC1",
			"converter_params": [314.0, 1.0, 0.0, 0.0, 1.0, 1.0, 200000.0, 0.05, 1.07, 0.01, 400.0, 0.06, 0.535, 0.00015],
			"connected_buses": [
				{ "bus_id": "ac", "terminal": 1 },
				{ "bus_id": "dc", "terminal": 2 }
			]
		}]
	})");

	SimulationBuilder builder;
	EXPECT_NO_THROW(builder.validateJSON(mmc));
}


TEST(JsonValidatorPins, PassiveTypesStillRequirePins) {
	const JSON bad = JSON::parse(R"({
		"simulation": { "title": "t" },
		"buses": [{ "id": "b1", "location": "AC1", "pins": 1 }],
		"components": [{
			"id": "R1", "type": "resistor", "location": "AC1",
			"values": [10.0],
			"connected_bus": { "bus_id": "b1", "terminal": 1 }
		}]
	})");

	SimulationBuilder builder;
	EXPECT_THROW(builder.validateJSON(bad), std::invalid_argument);
}


TEST(JsonParameters, ResolvesNamedValuesInPassives) {
	const JSON config = JSON::parse(R"({
		"simulation": { "title": "params" },
		"parameters": { "R_val": 12.5, "L_val": 0.002 },
		"buses": [
			{ "id": "b1", "location": "AC1", "pins": 1 },
			{ "id": "gnd", "location": "AC1", "pins": 1 }
		],
		"components": [{
			"id": "R1", "type": "resistor", "location": "AC1", "pins": 1,
			"values": ["R_val"],
			"connected_buses": [
				{ "bus_id": "b1", "terminal": 1 },
				{ "bus_id": "gnd", "terminal": 2 }
			]
		}, {
			"id": "L1", "type": "inductor", "location": "AC1", "pins": 1,
			"values": ["L_val"],
			"connected_buses": [
				{ "bus_id": "b1", "terminal": 1 },
				{ "bus_id": "gnd", "terminal": 2 }
			]
		}]
	})");

	SimulationBuilder builder;
	EXPECT_NO_THROW(builder.validateJSON(config));
	Network network;
	EXPECT_NO_THROW(builder.buildFromJSON(config, network));
}


TEST(JsonParameters, MmcNamedParamsExampleValidates) {
	const fs::path path = harmonyRoot() / "src" / "examples" / "json" / "mmc_named_params.json";
	ASSERT_TRUE(fs::exists(path));

	const JSON config = loadJsonFile(path);
	SimulationBuilder builder;
	EXPECT_NO_THROW(builder.validateJSON(config));
}


TEST(JsonParameters, RejectsUnknownParameterReference) {
	const JSON bad = JSON::parse(R"({
		"simulation": { "title": "params" },
		"parameters": { "R_val": 10.0 },
		"buses": [{ "id": "b1", "location": "AC1", "pins": 1 }],
		"components": [{
			"id": "R1", "type": "resistor", "location": "AC1", "pins": 1,
			"values": ["missing_param"],
			"connected_bus": { "bus_id": "b1", "terminal": 1 }
		}]
	})");

	SimulationBuilder builder;
	EXPECT_THROW(builder.validateJSON(bad), std::invalid_argument);
}


TEST(JsonParameters, LocalParametersOverrideRoot) {
	const JSON config = JSON::parse(R"({
		"simulation": { "title": "params" },
		"parameters": { "R_val": 10.0 },
		"buses": [{ "id": "b1", "location": "AC1", "pins": 1 }],
		"components": [{
			"id": "R1", "type": "resistor", "location": "AC1", "pins": 1,
			"local_parameters": { "R_val": 99.0 },
			"values": ["R_val"],
			"connected_bus": { "bus_id": "b1", "terminal": 1 }
		}]
	})");

	SimulationBuilder builder;
	Network network;
	ASSERT_NO_THROW(builder.buildFromJSON(config, network));
}


TEST(JsonExpressions, PassivesRlcExprExampleValidatesAndBuilds) {
	const fs::path path = harmonyRoot() / "src" / "examples" / "json" / "passives_rlc_expr.json";
	ASSERT_TRUE(fs::exists(path));

	const JSON config = loadJsonFile(path);
	SimulationBuilder builder;
	EXPECT_NO_THROW(builder.validateJSON(config));
	Network network;
	EXPECT_NO_THROW(builder.buildFromJSON(config, network));
}


TEST(JsonExpressions, ParsesZexprForImpedance) {
	const JSON config = JSON::parse(R"({
		"simulation": { "title": "z" },
		"parameters": { "R": 5.0, "L": 0.002 },
		"buses": [
			{ "id": "b1", "location": "AC1", "pins": 1 },
			{ "id": "gnd", "location": "AC1", "pins": 1 }
		],
		"components": [{
			"id": "Z1", "type": "impedance", "location": "AC1", "pins": 1,
			"z_expr": "R + s*L",
			"connected_buses": [
				{ "bus_id": "b1", "terminal": 1 },
				{ "bus_id": "gnd", "terminal": 2 }
			]
		}]
	})");

	SimulationBuilder builder;
	EXPECT_NO_THROW(builder.validateJSON(config));
}


TEST(JsonExpressions, RejectsUnresolvedSymbol) {
	const JSON bad = JSON::parse(R"({
		"simulation": { "title": "bad" },
		"buses": [{ "id": "b1", "location": "AC1", "pins": 1 }],
		"components": [{
			"id": "R1", "type": "resistor", "location": "AC1", "pins": 1,
			"y_expr": "1/unknown_R",
			"connected_bus": { "bus_id": "b1", "terminal": 1 }
		}]
	})");

	SimulationBuilder builder;
	EXPECT_THROW(builder.validateJSON(bad), std::invalid_argument);
}


TEST(JsonExpressions, RejectsValuesAndYexprTogether) {
	const JSON bad = JSON::parse(R"({
		"simulation": { "title": "bad" },
		"buses": [{ "id": "b1", "location": "AC1", "pins": 1 }],
		"components": [{
			"id": "R1", "type": "resistor", "location": "AC1", "pins": 1,
			"values": [10.0],
			"y_expr": "1/10",
			"connected_bus": { "bus_id": "b1", "terminal": 1 }
		}]
	})");

	SimulationBuilder builder;
	EXPECT_THROW(builder.validateJSON(bad), std::invalid_argument);
}
