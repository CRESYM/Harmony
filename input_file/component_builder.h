#pragma once
#include <iostream>

#include <nlohmann/json.hpp>
using JSON = nlohmann::json;

#include "../src/network.h"
#include "../src/Include_components.h"

using ComponentType = std::variant<
	// Sources
	AC_source,
	Generator,
	// Impedance
	Admittance,
	Impedance,
	Capacitor,
	Inductor,
	Resistor,
	// Transmission Line
	TransmissionLine,
	Cable,
	Overhead_Line,
	// Load
	Load,
	LoadPQ,
	// Converter
	MMC,
	// Switches and relays
	Switch,
	// Transformer
	TransformerDeltaDelta,
	TransformerDeltaY_real,
	TransformerDeltaY,
	Transformer_real,
	Transformer_classic,
	TransformerYDelta,
	TransformerYY_real,
	TransformerYY
>;

class ComponentBuilder {

public:

	static ComponentType buildFromJSON(const JSON& comp, const unsigned int i);

private:

	// One builder function per component type
	// 
	// Impedance
	static Capacitor buildCapacitor(const JSON& comp);
	static Inductor buildInductor(const JSON& comp);
	static Resistor buildResistor(const JSON& comp);
	//static Impedance buildImpedance(const JSON& comp);
	//static Admittance buildAdmittance(const JSON& comp);
	// Loads
	static Load buildLoad(const JSON& comp);
	static LoadPQ buildLoadPQ(const JSON& comp);
	// Source
	static AC_source buildACSource(const JSON& comp);
	static Generator buildGenerator(const JSON& comp);
	// Transformer
	static Transformer_real buildTransformerReal(const JSON& comp);
	static Transformer_classic buildTransformerClassic(const JSON& comp);
	static TransformerDeltaDelta buildTransformerDeltaDelta(const JSON& comp);
	static TransformerDeltaY_real buildTransformerDeltaYReal(const JSON& comp);
	static TransformerDeltaY buildTransformerDeltaY(const JSON& comp);
	static TransformerYDelta buildTransformerYDelta(const JSON& comp);
	static TransformerYY_real buildTransformerYYReal(const JSON& comp);
	static TransformerYY buildTransformerYY(const JSON& comp);


	// Helper functions to check for a specific key within a JSON component
	// Throws invalid_argument if not found or invalid
	static void findNumber(std::string key, const JSON& j);
	static void findNonEmptyString(std::string key, const JSON& j);
	static void findNonEmptyNumericArray(std::string key, const JSON& j, int arraysize = -1);
};

