#ifndef ELEMENT_H
#define ELEMENT_H

#include <string>
#include <unordered_map>
#include <stdexcept>
#include <functional> // Include the functional header for std::hash

class Element {
public:
	std::string symbol;
	std::unordered_map<std::string, std::string> pins;
	int input_pins;
	int output_pins;
	bool transformation;

	// Default constructor
	Element() {}

	// Constructor
	/*Element(const std::string& symbol_, int input_pins_, int output_pins_, bool transformation_) :
		symbol(symbol_), input_pins(input_pins_), output_pins(output_pins_), transformation(transformation_) {

		// Definition of pins
		if (!transformation) {
			input_pins -= 1;
			output_pins -= 1;
		}

		for (int i = 1; i <= input_pins; ++i) {
			pins["1." + std::to_string(i)] = "";
		}

		for (int i = 1; i <= output_pins; ++i) {
			pins["2." + std::to_string(i)] = "";
		}
	}*/
	Element(const std::unordered_map<std::string, std::string>& args) {
		// Initialize member variables from args
		if (args.find("symbol") != args.end()) {
			symbol = args.at("symbol");
		}
		else {
			throw std::invalid_argument("Symbol is missing in constructor arguments.");
		}
		if (args.find("input_pins") != args.end()) {
			input_pins = std::stoi(args.at("input_pins"));
		}
		else {
			throw std::invalid_argument("Input pins are missing in constructor arguments.");
		}
		if (args.find("output_pins") != args.end()) {
			output_pins = std::stoi(args.at("output_pins"));
		}
		else {
			throw std::invalid_argument("Output pins are missing in constructor arguments.");
		}
		if (args.find("transformation") != args.end()) {
			transformation = (args.at("transformation") == "true");
		}
		else {
			throw std::invalid_argument("Transformation flag is missing in constructor arguments.");
		}

		// Definition of pins
		if (!transformation) {
			input_pins -= 1;
			output_pins -= 1;
		}

		for (int i = 1; i <= input_pins; ++i) {
			pins["1." + std::to_string(i)] = "";
		}

		for (int i = 1; i <= output_pins; ++i) {
			pins["2." + std::to_string(i)] = "";
		}
	}



};

// Define the hash function for Element
struct ElementHash {
	size_t operator()(const Element& elem) const {
		// Calculate a hash value based on the unique properties of Element
		// Combine hash values of individual data members using std::hash_combine or other techniques
		// Example: hashing based on symbol and pins
		size_t hashValue = std::hash<std::string>{}(elem.symbol);
		for (const auto& pair : elem.pins) {
			hashValue ^= std::hash<std::string>{}(pair.first) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
			hashValue ^= std::hash<std::string>{}(pair.second) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
		}
		return hashValue;
	}
};

inline bool operator==(const Element& lhs, const Element& rhs) {
	// Compare member variables for equality
	return lhs.symbol == rhs.symbol
		&& lhs.input_pins == rhs.input_pins
		&& lhs.output_pins == rhs.output_pins
		&& lhs.transformation == rhs.transformation;
};

#endif // ELEMENT_H