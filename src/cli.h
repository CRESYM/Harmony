#pragma once

#include <filesystem>
#include <functional>
#include <optional>
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
	/** @brief From `--json-path` (replaces defaults when non-empty). */
	std::vector<std::filesystem::path> jsonPathOverrides;
	/** @brief From `--search-path` (appended after defaults). */
	std::vector<std::filesystem::path> extraSearchPaths;
	/** @brief Effective search list built in @ref parseCli. */
	std::vector<std::filesystem::path> searchPaths;
};

using ExampleFn = std::function<void(bool plotting_enabled)>;

/** @brief Resolve executable location for repo-root detection (call once from main). */
void initCliPaths(const char* argv0);

/** @brief Detected Harmony repository root, if any. */
std::optional<std::filesystem::path> harmonyRepoRoot();

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
int runJsonSimulation(const std::filesystem::path& jsonPath, bool verbose, bool plot = true);

/** @brief Built-in JSON search directories (used when env/CLI overrides are unset). */
std::vector<std::filesystem::path> builtinJsonSearchPaths();

/** @brief Paths from `HARMONY_JSON_PATH`, or empty if the variable is unset. */
std::vector<std::filesystem::path> jsonSearchPathsFromEnvironment();

/** @brief Default paths: `HARMONY_JSON_PATH` if set, otherwise @ref builtinJsonSearchPaths. */
std::vector<std::filesystem::path> defaultJsonSearchPaths();

/**
 * @brief Build the effective JSON search path list for one invocation.
 * @param jsonPathOverrides If non-empty, replaces defaults (`--json-path`).
 * @param extraSearchPaths Appended after defaults (`--search-path`).
 */
std::vector<std::filesystem::path> buildJsonSearchPaths(
	const std::vector<std::filesystem::path>& jsonPathOverrides,
	const std::vector<std::filesystem::path>& extraSearchPaths);

/** @brief Resolve a JSON path (absolute, relative, or filename in search paths). */
std::filesystem::path resolveJsonPath(
	const std::string& input,
	const std::vector<std::filesystem::path>& searchPaths);

/** @brief Print why a JSON file was not found (search paths, repo root, env). */
void printJsonNotFoundHelp(
	const std::string& input,
	const std::vector<std::filesystem::path>& searchPaths);
