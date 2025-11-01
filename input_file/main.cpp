#include <iostream>
#include <fstream>
#include <filesystem>

#include "simulation_builder.h"


int main() {

	std::string inputfilepath = "../../../src/examples/example.json";

	// Check input file exists
	if (!std::filesystem::exists(inputfilepath)) {
		std::cerr << "Input file could not be found at: " << inputfilepath << std::endl;
		return EXIT_FAILURE;
	}
	else {
		std::cout << "Using input file: " << inputfilepath << std::endl;
	}

	// Load file contents to stream
	std::ifstream inputfilestream(inputfilepath);

	// Read file to JSON object
	JSON configJSON;
	try {
		configJSON = JSON::parse(inputfilestream);
	}
	catch (JSON::parse_error& ex)
	{
		std::cerr << "ERROR: reading input file at byte " << ex.byte << std::endl;
		return EXIT_FAILURE;
	}

	// Build simulation
	Network network;
	SimulationBuilder sim;
	try {
		sim.buildFromJSON(configJSON, network);
	}
	catch (const std::exception& e) {
		std::cerr << e.what() <<"Terminating application.";
		return EXIT_FAILURE;
	}

	// Print information of each element
	std::vector<ComponentType> components = sim.getComponents();
	for (const auto& c : components) {
		std::visit([](const auto& elem) { elem.printElementInfo();
			}, c);
	}


	return 0;

}

