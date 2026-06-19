#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

/**
 * @file cli.h
 * @brief Command-line interface for the unified Harmony executable.
 */

struct CliOptions {
	enum class Mode {
		Help,
		ListCpp,
		ListJson,
		Cpp,
		Json
	};

	Mode mode = Mode::Help;
	std::string target;
	bool plot = true;
	bool verbose = false;
	std::vector<std::filesystem::path> searchPaths;
};

using ExampleFn = std::function<void(bool plotting_enabled)>;

/** @brief Parse argc/argv into @ref CliOptions. */
CliOptions parseCli(int argc, char* argv[]);

/** @brief Print usage text to stdout. */
void printCliHelp();

/** @brief List registered C++ examples. */
void listCppExamples();

/** @brief List JSON files found under configured search paths. */
void listJsonFiles(const std::vector<std::filesystem::path>& searchPaths);

/** @brief Run a named C++ example. @return 0 on success, non-zero on failure. */
int runCppExample(const std::string& name, bool plot, bool verbose);

/** @brief Run a JSON simulation file. @return 0 on success, non-zero on failure. */
int runJsonSimulation(const std::filesystem::path& jsonPath, bool verbose);

/** @brief Default directories searched for JSON input files. */
std::vector<std::filesystem::path> defaultJsonSearchPaths();

/** @brief Resolve a JSON path (absolute, relative, or filename in search paths). */
std::filesystem::path resolveJsonPath(
	const std::string& input,
	const std::vector<std::filesystem::path>& searchPaths);
