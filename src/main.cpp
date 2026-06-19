/**
 * @file main.cpp
 * @brief Unified Harmony entry point: C++ examples and JSON simulations.
 */
#include "cli.h"

#include <filesystem>
#include <iostream>


int main(int argc, char* argv[]) {
	if (argc > 0) {
		initCliPaths(argv[0]);
	}

	const CliOptions opts = parseCli(argc, argv);

	switch (opts.mode) {
	case CliOptions::Mode::Help:
		printCliHelp();
		return opts.target.empty() && argc == 1 ? EXIT_SUCCESS : EXIT_FAILURE;

	case CliOptions::Mode::ListCpp:
		listCppExamples();
		return EXIT_SUCCESS;

	case CliOptions::Mode::ListJson:
		listJsonFiles(opts.searchPaths);
		return EXIT_SUCCESS;

	case CliOptions::Mode::Cpp:
		return runCppExample(opts.target, opts.plot, opts.verbose);

	case CliOptions::Mode::Json: {
		const auto path = resolveJsonPath(opts.target, opts.searchPaths);
		if (!std::filesystem::exists(path)) {
			printJsonNotFoundHelp(opts.target, opts.searchPaths);
			return EXIT_FAILURE;
		}
		return runJsonSimulation(path, opts.verbose);
	}

	default:
		printCliHelp();
		return EXIT_FAILURE;
	}
}
