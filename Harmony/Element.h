#ifndef ELEMENT_H
#define ELEMENT_H

#include <string>
#include <unordered_map>
#include <stdexcept>

class Element {
public:
	std::string symbol;
	std::unordered_map<std::string, std::string> pins;
	int input_pins;
	int output_pins;
	bool transformation;

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

#endif // ELEMENT_H


