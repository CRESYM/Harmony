#pragma once
#include <iostream>

#include <nlohmann/json.hpp>
using JSON = nlohmann::json;

#include "../src/network.h"
#include "../src/Include_components.h"

class ComponentBuilder {

public:

	/// Parse a JSON component object and return a heap-allocated Element (caller/Network owns it).
	static Element* buildFromJSON(const JSON& comp, unsigned int index);

	static void findNumber(const std::string& key, const JSON& j);
	static void findNonEmptyString(const std::string& key, const JSON& j);
	static void findNonEmptyNumericArray(const std::string& key, const JSON& j, int arraysize = -1);

private:

	// Impedance
	static Capacitor* buildCapacitor(const JSON& comp);
	static Inductor* buildInductor(const JSON& comp);
	static Resistor* buildResistor(const JSON& comp);
	// Loads
	static Load* buildLoad(const JSON& comp);
	static LoadPQ* buildLoadPQ(const JSON& comp);
	// Source
	static AC_source* buildACSource(const JSON& comp);
	static Generator* buildGenerator(const JSON& comp);
	// Transformer
	static Transformer_real* buildTransformerReal(const JSON& comp);
	static Transformer_classic* buildTransformerClassic(const JSON& comp);
	static TransformerDeltaDelta* buildTransformerDeltaDelta(const JSON& comp);
	static TransformerDeltaY_real* buildTransformerDeltaYReal(const JSON& comp);
	static TransformerDeltaY* buildTransformerDeltaY(const JSON& comp);
	static TransformerYDelta* buildTransformerYDelta(const JSON& comp);
	static TransformerYY_real* buildTransformerYYReal(const JSON& comp);
	static TransformerYY* buildTransformerYY(const JSON& comp);

	static std::vector<double> readTransformerClassicValues(const JSON& comp);
	static std::vector<double> readTransformerRealValues(const JSON& comp);
};
