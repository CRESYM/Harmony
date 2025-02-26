#include <string>
#include <fstream>
#include "utils.h"

std::string readFile(std::string filepath) {
	// open the file for reading
	std::ifstream ifs(filepath, std::ifstream::in);
	if (!ifs.is_open() || ifs.fail()) {
		throw std::runtime_error("Failed to read testing file.");
	}

	// read
	std::string line, file;
	while (std::getline(ifs, line)) {
		file.append(line + "\n");
	}

	ifs.close();
	return file;
}
