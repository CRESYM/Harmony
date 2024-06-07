/*#ifndef MYSYMBOL_H
#define MYSYMBOL_H

#include <iostream>
#include <cmath>
#include <unordered_map>
#include <vector>
#include <utility>
#include <string>
#include <symbol.h>
#include <basic.h>

#include "Cable.h"
#include "symengine_n.h"
#include "MyBasic.h" // Include the header file where Basic is defined


//using CMapBasicBasic = std::unordered_map<SymEngine::Basic, SymEngine::Basic, BasicHash, BasicEqual>;


class Complex;

class MySymbol {
public:
	std::string name;
	double value;

	MySymbol(const std::string& name_, double value_) : name(name_), value(value_) {}
	// Default constructor initializes name with an empty string and value with 0.0
	MySymbol() : name(""), value(0.0) {}

	// Static method to create a symbolic variable
	static SymEngine::Basic symbols(const std::string& name) {
		// Assuming Basic has a constructor that takes a name and a default value
		return Basic(name);
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
#endif*/
