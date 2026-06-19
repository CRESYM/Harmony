/**
 * @file cli.cpp
 * @brief Implementation of the unified Harmony command-line interface.
 */
#include "cli.h"

#include "examples/Examples.h"
#include "json/simulation_builder.h"
#include "Solver/Helper_Functions/Visualization.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>

namespace {

constexpr const char* kHarmonyJsonPathEnv = "HARMONY_JSON_PATH";

std::filesystem::path g_executableDir;

bool isHarmonyRepoRoot(const std::filesystem::path& dir) {
	return std::filesystem::exists(dir / "CMakeLists.txt")
		&& std::filesystem::exists(dir / "src/examples/example.json");
}

std::optional<std::filesystem::path> findRepoRootFrom(std::filesystem::path start) {
	if (start.empty()) {
		return std::nullopt;
	}

	try {
		start = std::filesystem::absolute(start);
	}
	catch (const std::filesystem::filesystem_error&) {
		return std::nullopt;
	}

	for (int depth = 0; depth < 10; ++depth) {
		if (isHarmonyRepoRoot(start)) {
			return start;
		}
		const auto parent = start.parent_path();
		if (parent == start) {
			break;
		}
		start = parent;
	}
	return std::nullopt;
}

void appendUniquePaths(
	std::vector<std::filesystem::path>& dst,
	const std::vector<std::filesystem::path>& src)
{
	for (const auto& path : src) {
		if (std::find(dst.begin(), dst.end(), path) == dst.end()) {
			dst.push_back(path);
		}
	}
}

std::vector<std::filesystem::path> parsePathList(const std::string& value) {
	std::vector<std::filesystem::path> paths;
	std::string token;
	for (const char c : value) {
		if (c == ';' || c == ':') {
			if (!token.empty()) {
				paths.emplace_back(token);
				token.clear();
			}
		}
		else {
			token += c;
		}
	}
	if (!token.empty()) {
		paths.emplace_back(token);
	}
	return paths;
}

} // namespace


void initCliPaths(const char* argv0) {
	if (argv0 == nullptr || argv0[0] == '\0') {
		return;
	}
	try {
		g_executableDir = std::filesystem::absolute(std::filesystem::path(argv0)).parent_path();
	}
	catch (const std::filesystem::filesystem_error&) {
		g_executableDir.clear();
	}
}


std::optional<std::filesystem::path> harmonyRepoRoot() {
	if (auto root = findRepoRootFrom(std::filesystem::current_path())) {
		return root;
	}
	if (!g_executableDir.empty()) {
		if (auto root = findRepoRootFrom(g_executableDir)) {
			return root;
		}
	}
	return std::nullopt;
}


namespace {

std::vector<std::filesystem::path> repoAnchoredJsonSearchPaths() {
	std::vector<std::filesystem::path> paths;
	if (const auto root = harmonyRepoRoot()) {
		paths.push_back(*root / "src/examples");
		paths.push_back(*root / "src/examples/json");
		paths.push_back(*root / "src/json");
		paths.push_back(*root / "examples");
	}
	return paths;
}

} // namespace


namespace {

std::map<std::string, ExampleFn> exampleRegistry() {
	std::map<std::string, ExampleFn> registry;
	auto add = [&](const char* name, ExampleFn fn) { registry[name] = std::move(fn); };

	add("opf", example_OPF);
	add("opf_csv", example_OPF_csv);
	add("opf_1", example_OPF_1);
	add("opf_csv_1", example_OPF_csv_1);
	add("opf_pv", example_OPF_PV);
	add("opf_wt", example_OPF_WT);
	add("dqsym_math_operations", example_DQsym_math_operations);
	add("dqsym_dsss2", example_DQsym_DSSS2);
	add("dqsym_rlc", example_DQsym_RLC);
	add("dqsym_simple_mmc", example_DQsym_Simple_MMC);
	add("state_space", [](bool) { example_state_space(); });
	add("generator", example_generator);
	add("mmc", example_MMC);
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
	return registry;
}

std::string normalizeExampleName(std::string name) {
	std::transform(name.begin(), name.end(), name.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	if (name.rfind("example_", 0) == 0) {
		name.erase(0, 8);
	}
	return name;
}

} // namespace


std::vector<std::filesystem::path> builtinJsonSearchPaths() {
	return {
		"src/examples",
		"src/examples/json",
		"src/json",
		"examples",
		"."
	};
}


std::vector<std::filesystem::path> jsonSearchPathsFromEnvironment() {
	if (const char* env = std::getenv(kHarmonyJsonPathEnv)) {
		const std::string value(env);
		if (!value.empty()) {
			return parsePathList(value);
		}
	}
	return {};
}


std::vector<std::filesystem::path> defaultJsonSearchPaths() {
	const auto fromEnv = jsonSearchPathsFromEnvironment();
	if (!fromEnv.empty()) {
		return fromEnv;
	}
	return builtinJsonSearchPaths();
}


std::vector<std::filesystem::path> buildJsonSearchPaths(
	const std::vector<std::filesystem::path>& jsonPathOverrides,
	const std::vector<std::filesystem::path>& extraSearchPaths)
{
	std::vector<std::filesystem::path> paths;
	if (!jsonPathOverrides.empty()) {
		paths = jsonPathOverrides;
	}
	else {
		const auto fromEnv = jsonSearchPathsFromEnvironment();
		if (!fromEnv.empty()) {
			paths = fromEnv;
		}
		else {
			appendUniquePaths(paths, repoAnchoredJsonSearchPaths());
			appendUniquePaths(paths, builtinJsonSearchPaths());
		}
	}

	appendUniquePaths(paths, extraSearchPaths);

	if (std::find(paths.begin(), paths.end(), std::filesystem::path(".")) == paths.end()) {
		paths.push_back(".");
	}
	return paths;
}


CliOptions parseCli(int argc, char* argv[]) {
	CliOptions opts;

	for (int i = 1; i < argc; ++i) {
		const std::string arg = argv[i];

		if (arg == "--help" || arg == "-h") {
			opts.mode = CliOptions::Mode::Help;
			return opts;
		}
		if (arg == "--list-cpp") {
			opts.mode = CliOptions::Mode::ListCpp;
			opts.searchPaths = buildJsonSearchPaths(opts.jsonPathOverrides, opts.extraSearchPaths);
			return opts;
		}
		if (arg == "--list-json") {
			opts.mode = CliOptions::Mode::ListJson;
			opts.searchPaths = buildJsonSearchPaths(opts.jsonPathOverrides, opts.extraSearchPaths);
			return opts;
		}
		if (arg == "--no-plot") {
			opts.plot = false;
			continue;
		}
		if (arg == "--verbose" || arg == "-v") {
			opts.verbose = true;
			continue;
		}
		if (arg == "--json-path" && i + 1 < argc) {
			opts.jsonPathOverrides.emplace_back(argv[++i]);
			continue;
		}
		if (arg == "--search-path" && i + 1 < argc) {
			opts.extraSearchPaths.emplace_back(argv[++i]);
			continue;
		}
		if (arg == "--cpp" && i + 1 < argc) {
			opts.mode = CliOptions::Mode::Cpp;
			opts.target = argv[++i];
			continue;
		}
		if (arg == "--json" && i + 1 < argc) {
			opts.mode = CliOptions::Mode::Json;
			opts.target = argv[++i];
			continue;
		}

		std::cerr << "Unknown or incomplete argument: " << arg << "\n";
		opts.mode = CliOptions::Mode::Help;
		return opts;
	}

	if (opts.mode == CliOptions::Mode::Help && argc == 1) {
		opts.mode = CliOptions::Mode::Help;
	}

	opts.searchPaths = buildJsonSearchPaths(opts.jsonPathOverrides, opts.extraSearchPaths);
	return opts;
}


void printCliHelp() {
	std::cout <<
		"Harmony — hybrid AC/DC power-system framework\n\n"
		"Usage:\n"
		"  Harmony --cpp <example>     Run a C++ example program\n"
		"  Harmony --json <file>       Run a JSON simulation file\n\n"
		"Options:\n"
		"  --json-path <dir>           Replace default JSON search dirs (repeatable)\n"
		"  --search-path <dir>         Append JSON search directory (repeatable)\n"
		"  --list-cpp                  List available C++ examples\n"
		"  --list-json                 List JSON files in search paths\n"
		"  --no-plot                   Disable GUI plots (C++ examples and JSON computations)\n"
		"  --verbose, -v               Verbose output\n"
		"  --help, -h                  Show this help\n\n"
		"Default JSON search paths (when HARMONY_JSON_PATH is unset):\n"
		"  src/examples, src/examples/json, src/json, examples, .\n\n"
		"Override defaults for all runs:\n"
		"  set HARMONY_JSON_PATH=D:\\\\cases;D:\\\\other   (Windows)\n"
		"  export HARMONY_JSON_PATH=/cases:/other         (Linux/macOS)\n\n"
		"Examples:\n"
		"  Harmony --cpp stability_check\n"
		"  Harmony --json example.json\n"
		"  Harmony --json-path D:\\cases --json my_case.json\n"
		"  Harmony --json src/examples/example.json --verbose\n";
}


void listCppExamples() {
	const auto registry = exampleRegistry();
	std::cout << "Available C++ examples (" << registry.size() << "):\n";
	for (const auto& [name, fn] : registry) {
		(void)fn;
		std::cout << "  " << name << "\n";
	}
}


void listJsonFiles(const std::vector<std::filesystem::path>& searchPaths) {
	std::cout << "JSON files in search paths";
	if (const auto root = harmonyRepoRoot()) {
		std::cout << " (repo root: " << root->generic_string() << ")";
	}
	std::cout << ":\n";

	bool found = false;
	for (const auto& dir : searchPaths) {
		if (!std::filesystem::is_directory(dir)) {
			std::cout << "  [skip, not a directory] " << dir.generic_string() << "\n";
			continue;
		}
		for (const auto& entry : std::filesystem::directory_iterator(dir)) {
			if (!entry.is_regular_file()) {
				continue;
			}
			if (entry.path().extension() == ".json") {
				std::cout << "  " << entry.path().generic_string() << "\n";
				found = true;
			}
		}
	}
	if (!found) {
		std::cout << "  (none found)\n";
		std::cout << "Directories checked:\n";
		for (const auto& dir : searchPaths) {
			std::cout << "  " << std::filesystem::absolute(dir).generic_string() << "\n";
		}
		if (std::getenv(kHarmonyJsonPathEnv)) {
			std::cout << "Note: HARMONY_JSON_PATH is set and replaces built-in defaults.\n";
		}
		std::cout << "Use --json-path, --search-path, a full file path, or run from the repository root.\n";
	}
}


std::filesystem::path resolveJsonPath(
	const std::string& input,
	const std::vector<std::filesystem::path>& searchPaths)
{
	const std::filesystem::path direct(input);
	if (std::filesystem::exists(direct)) {
		return std::filesystem::absolute(direct);
	}

	if (const auto root = harmonyRepoRoot()) {
		const std::filesystem::path fromRoot = *root / direct;
		if (std::filesystem::exists(fromRoot)) {
			return std::filesystem::absolute(fromRoot);
		}
	}

	const std::filesystem::path filename = direct.filename();
	for (const auto& dir : searchPaths) {
		const std::filesystem::path candidate = dir / input;
		if (std::filesystem::exists(candidate)) {
			return std::filesystem::absolute(candidate);
		}
		if (!filename.empty() && filename != direct) {
			const std::filesystem::path byName = dir / filename;
			if (std::filesystem::exists(byName)) {
				return std::filesystem::absolute(byName);
			}
		}
	}

	return direct;
}


void printJsonNotFoundHelp(
	const std::string& input,
	const std::vector<std::filesystem::path>& searchPaths)
{
	std::cerr << "JSON file not found: " << input << "\n";

	if (const auto root = harmonyRepoRoot()) {
		std::cerr << "Repository root: " << root->generic_string() << "\n";
	}
	else {
		std::cerr << "Repository root: (not detected — run from the Harmony repo or use a full path)\n";
	}

	std::cerr << "Current directory: "
		<< std::filesystem::absolute(std::filesystem::path(".")).generic_string() << "\n";

	if (std::getenv(kHarmonyJsonPathEnv)) {
		std::cerr << "HARMONY_JSON_PATH=" << std::getenv(kHarmonyJsonPathEnv) << "\n";
	}

	std::cerr << "Search paths:\n";
	for (const auto& dir : searchPaths) {
		std::cerr << "  " << std::filesystem::absolute(dir).generic_string() << "\n";
	}

	std::cerr <<
		"Tips:\n"
		"  Harmony --list-json\n"
		"  Harmony --json src/examples/example.json\n"
		"  Harmony --json-path <dir> --json <file>\n"
		"  Set Working Directory to the repository root in Visual Studio\n";
}


int runCppExample(const std::string& name, const bool plot, const bool verbose) {
	const auto registry = exampleRegistry();
	const std::string key = normalizeExampleName(name);
	const auto it = registry.find(key);
	if (it == registry.end()) {
		std::cerr << "Unknown C++ example '" << name << "'. Use --list-cpp.\n";
		return EXIT_FAILURE;
	}

	if (verbose) {
		std::cout << "Running C++ example: " << key << " (plot=" << (plot ? "on" : "off") << ")\n";
	}

	try {
		it->second(plot);
	}
	catch (const std::exception& e) {
		std::cerr << "Example failed: " << e.what() << "\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


int runJsonSimulation(
	const std::filesystem::path& jsonPath,
	const bool verbose,
	const bool plot)
{
	if (!std::filesystem::exists(jsonPath)) {
		std::cerr << "Input file not found: " << jsonPath << "\n";
		return EXIT_FAILURE;
	}

	if (verbose) {
		std::cout << "Using JSON file: " << jsonPath
			<< " (plot=" << (plot ? "on" : "off") << ")\n";
	}

	std::ifstream stream(jsonPath);
	if (!stream) {
		std::cerr << "Unable to open: " << jsonPath << "\n";
		return EXIT_FAILURE;
	}

	JSON config;
	try {
		config = JSON::parse(stream);
	}
	catch (const JSON::parse_error& ex) {
		std::cerr << "JSON parse error at byte " << ex.byte << ": " << ex.what() << "\n";
		return EXIT_FAILURE;
	}

	Network network;
	SimulationBuilder builder;
	try {
		builder.buildFromJSON(config, network);
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << "\n";
		return EXIT_FAILURE;
	}

	if (verbose) {
		std::cout << "\n--- Built network ---\n";
	}
	network.printConnections();
	network.printElements();

	try {
		builder.runComputations(config, network, plot);
	}
	catch (const std::exception& e) {
		std::cerr << "[WARN] Computation step failed: " << e.what() << "\n";
	}

	if (verbose) {
		std::cout << "\nSimulation finished.\n";
	}

	// Keep the ImGui plot window alive until the user closes it (C++ examples use cin.get()).
	if (plot && visualization_is_running()) {
		if (verbose) {
			std::cout << "Close the Harmony Visualization window to exit.\n";
		}
		visualization_wait();
	}

	return EXIT_SUCCESS;
}
