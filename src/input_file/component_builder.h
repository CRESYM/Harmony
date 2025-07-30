#pragma once
#include <iostream>
#include "../Include_components.h"

#include <nlohmann/json.hpp>
using JSON = nlohmann::json;


using ComponentType = std::variant<
	AC_source,
	Admittance,
	Cable,
	Capacitor,
	Controller,
	Filter,
	Impedance,
	Inductor,
	Generator,
	Load,
	MMC,
	Overhead_Line,
	Resistor,
	Switch,
	TransformerDeltaDelta,
	TransformerDeltaY,
	Transformer_real,
	TransformerYDelta,
	TransformerYY,
	TransmissionLine
>;

class ComponentBuilder {

public:

	static ComponentType buildFromJSON(const JSON& comp, const unsigned int i);

private:

	// One builder function per component type

	static Load buildLoad(const JSON& comp);

	static Transformer_real buildTransformerReal(const JSON& comp);

	// Helper functions to check for a specific key within a JSON component
	// Throws invalid_argument if not found or invalid
	static void findNumber(std::string key, const JSON& j);
	static void findNonEmptyString(std::string key, const JSON& j);
	static void findNonEmptyNumericArray(std::string key, const JSON& j, int arraysize = -1);
};

