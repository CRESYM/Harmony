#include <iostream>
#include <fstream>
#include <filesystem>

#include "simulation_builder.h"


static std::string resolveInputPath(int argc, char* argv[]) {
	if (argc >= 2) {
		return argv[1];
	}
	return "../src/examples/example.json";
}


int main(int argc, char* argv[]) {

	const std::string inputfilepath = resolveInputPath(argc, argv);

	if (!std::filesystem::exists(inputfilepath)) {
		std::cerr << "Input file could not be found at: " << inputfilepath << std::endl;
		std::cerr << "Usage: Harmony [path/to/simulation.json]\n";
		return EXIT_FAILURE;
	}

	std::cout << "Using input file: " << inputfilepath << std::endl;

	std::ifstream inputfilestream(inputfilepath);
	if (!inputfilestream) {
		std::cerr << "ERROR: unable to open input file.\n";
		return EXIT_FAILURE;
	}

	JSON configJSON;
	try {
		configJSON = JSON::parse(inputfilestream);
	}
	catch (const JSON::parse_error& ex) {
		std::cerr << "ERROR: reading input file at byte " << ex.byte << std::endl;
		return EXIT_FAILURE;
	}

	Network network;
	SimulationBuilder sim;
	try {
		sim.buildFromJSON(configJSON, network);
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << " Terminating application.\n";
		return EXIT_FAILURE;
	}

	std::cout << "\n--- Built network ---\n";
	network.printConnections();
	network.printElements();

	try {
		sim.runComputations(configJSON, network);
	}
	catch (const std::exception& e) {
		std::cerr << "[WARN] Computation step failed: " << e.what() << "\n";
	}

	std::cout << "\nSimulation finished.\n";
	return EXIT_SUCCESS;
}
