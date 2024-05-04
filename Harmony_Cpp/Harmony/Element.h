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
	Element(const std::unordered_map<std::string, std::string>& args) {
		for (auto it = args.begin(); it != args.end(); ++it) {
			const std::string& key = it->first;
			const std::string& val = it->second;

			if (key == "symbol") {
				symbol = val;
			}
			else if (key == "input_pins") {
				input_pins = std::stoi(val);
			}
			else if (key == "output_pins") {
				output_pins = std::stoi(val);
			}
			else if (key == "element_value") {
				// This field is of type Any in Julia, here we can use a generic type or a variant
				// For simplicity, we can skip handling this field in the constructor
			}
			else if (key == "transformation") {
				transformation = (val == "true");
			}
			else {
				throw std::invalid_argument("The property name " + key + " is not defined.");
			}
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

bool operator==(const Element& lhs, const Element& rhs) {
	// Compare member variables for equality
	return lhs.symbol == rhs.symbol
		&& lhs.input_pins == rhs.input_pins
		&& lhs.output_pins == rhs.output_pins
		&& lhs.transformation == rhs.transformation;
};

#endif // ELEMENT_H


