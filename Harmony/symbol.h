#ifndef SYMBOL_H
#define SYMBOL_H

#include <iostream>
#include <cmath>
#include <unordered_map>

class Symbol {
public:
	std::string name;
	double value;

	Symbol(const std::string& name_, double value_) : name(name_), value(value_) {}
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
