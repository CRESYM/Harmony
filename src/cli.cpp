/**
 * @file cli.cpp
 * @brief Implementation of the unified Harmony command-line interface.
 */
#include "cli.h"

#include "examples/Examples.h"
#include "json/simulation_builder.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>

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


std::vector<std::filesystem::path> defaultJsonSearchPaths() {
	return {
		"src/examples",
		"src/json",
		"examples",
		"."
	};
}


CliOptions parseCli(int argc, char* argv[]) {
	CliOptions opts;
	opts.searchPaths = defaultJsonSearchPaths();

	for (int i = 1; i < argc; ++i) {
		const std::string arg = argv[i];

		if (arg == "--help" || arg == "-h") {
			opts.mode = CliOptions::Mode::Help;
			return opts;
		}
		if (arg == "--list-cpp") {
			opts.mode = CliOptions::Mode::ListCpp;
			return opts;
		}
		if (arg == "--list-json") {
			opts.mode = CliOptions::Mode::ListJson;
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
		if (arg == "--search-path" && i + 1 < argc) {
			opts.searchPaths.emplace_back(argv[++i]);
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
	return opts;
}


void printCliHelp() {
	std::cout <<
		"Harmony — hybrid AC/DC power-system framework\n\n"
		"Usage:\n"
		"  Harmony --cpp <example>     Run a C++ example program\n"
		"  Harmony --json <file>       Run a JSON simulation file\n\n"
		"Options:\n"
		"  --search-path <dir>         Add JSON search directory (repeatable)\n"
		"  --list-cpp                  List available C++ examples\n"
		"  --list-json                 List JSON files in search paths\n"
		"  --no-plot                   Disable GUI plotting in examples\n"
		"  --verbose, -v               Verbose output\n"
		"  --help, -h                  Show this help\n\n"
		"Default JSON search paths:\n"
		"  src/examples, src/json, examples, .\n\n"
		"Examples:\n"
		"  Harmony --cpp stability_check\n"
		"  Harmony --json example.json\n"
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
	std::cout << "JSON files in search paths:\n";
	bool found = false;
	for (const auto& dir : searchPaths) {
		if (!std::filesystem::is_directory(dir)) {
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
		std::cout << "  (none found — check --search-path directories)\n";
	}
}


std::filesystem::path resolveJsonPath(
	const std::string& input,
	const std::vector<std::filesystem::path>& searchPaths)
{
	const std::filesystem::path direct(input);
	if (std::filesystem::exists(direct)) {
		return direct;
	}

	for (const auto& dir : searchPaths) {
		const std::filesystem::path candidate = dir / input;
		if (std::filesystem::exists(candidate)) {
			return candidate;
		}
	}

	return direct;
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


int runJsonSimulation(const std::filesystem::path& jsonPath, const bool verbose) {
	if (!std::filesystem::exists(jsonPath)) {
		std::cerr << "Input file not found: " << jsonPath << "\n";
		return EXIT_FAILURE;
	}

	if (verbose) {
		std::cout << "Using JSON file: " << jsonPath << "\n";
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
		builder.runComputations(config, network);
	}
	catch (const std::exception& e) {
		std::cerr << "[WARN] Computation step failed: " << e.what() << "\n";
	}

	if (verbose) {
		std::cout << "\nSimulation finished.\n";
	}
	return EXIT_SUCCESS;
}
