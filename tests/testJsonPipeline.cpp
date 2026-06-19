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
	for (const auto& path : jsonExampleFiles()) {
		EXPECT_EQ(runJsonSimulation(path, false), 0) << path;
	}
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
