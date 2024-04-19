#ifndef SYMBOL_H
#define SYMBOL_H

#include <iostream>
#include <cmath>
#include <unordered_map>
#include <vector>
#include <utility>
#include <string>

#include "Cable.h"
#include "symengine_n.h"
class Complex;

class Symbol {
public:
	std::string name;
	double value;

	Symbol(const std::string& name_, double value_) : name(name_), value(value_) {}
	// Default constructor initializes name with an empty string and value with 0.0
	Symbol() : name(""), value(0.0) {}

	// Static method to create a symbolic variable
	static Symbol symbols(const std::string& name) {
		return Symbol(name, 0.0); // Initialize the value to 0.0
	}
};

class SymbolTable {
private:
	std::unordered_map<std::string, double> table;

public:
	// Function to add a symbol-value pair to the table
	void addSymbol(const std::string& name, double value) {
		table[name] = value;
	}

	// Function to retrieve the numeric value associated with a symbol
	double getNumericValue(const std::string& name) const {
		auto it = table.find(name);
		if (it != table.end()) {
			return it->second;
		}
		else {
			return 0.0; // Return a default value
		}
	}
};
#endif
