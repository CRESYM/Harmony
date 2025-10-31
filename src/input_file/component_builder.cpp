#include "component_builder.h"


ComponentType ComponentBuilder::buildFromJSON(const JSON& comp, const unsigned int i) {

	std::string comptype = "errtype";

	try {
		// check for keys comomon to all components 
		findNonEmptyString("type", comp);
		findNonEmptyString("id", comp);
		findNumber("pins", comp);

		// extract type from json
		comptype = comp["type"];

		// convert comptype to lower case
		std::transform(comptype.begin(), comptype.end(), comptype.begin(),
			[](unsigned char c) { return std::tolower(c); });

		// create element
		if (comptype == "load") { return buildLoad(comp); }
		else if (comptype == "transformer_real") { return buildTransformerReal(comp); }
		else {
			// Unknown component type
			throw std::invalid_argument("ERROR: Component [" + std::to_string(i) + "] has unknown type '" + comptype + "'.\n");
		}
	}
	catch (const std::invalid_argument& e) {
		std::cerr << e.what();
		throw std::invalid_argument("ERROR: unable to create component [" + std::to_string(i) + "] of type " + comptype + ".\n");	
	}
}

Capacitor ComponentBuilder::buildCapacitor(const JSON& comp) {
	findNonEmptyNumericArray("values", comp);

	std::string id = comp["id"];
	std::string location = comp["location"];
	int pins = comp["pins"];
	std::vector<double> values = comp["values"].get<std::vector<double>>();
	return Capacitor(id, location, pins, values);
}

Inductor ComponentBuilder::buildInductor(const JSON& comp) {
	findNonEmptyNumericArray("values", comp);

	std::string id = comp["id"];
	std::string location = comp["location"];
	int pins = comp["pins"];
	std::vector<double> values = comp["values"].get<std::vector<double>>();
	return Inductor(id, location, pins, values);
}

Resistor ComponentBuilder::buildResistor(const JSON& comp) {
	findNonEmptyNumericArray("values", comp);

	std::string id = comp["id"];
	std::string location = comp["location"];
	int pins = comp["pins"];
	std::vector<double> values = comp["values"].get<std::vector<double>>();
	return Load(id, location, pins, values);
}

Load ComponentBuilder::buildLoad(const JSON& comp) {
	findNonEmptyNumericArray("values", comp);

	std::string id = comp["id"];
	std::string location = comp["location"];
	int pins = comp["pins"];
	std::vector<double> values = comp["values"].get<std::vector<double>>();
	return Load(id, location, pins, values);
}

LoadPQ ComponentBuilder::buildLoadPQ(const JSON& comp) {
	findNonEmptyNumericArray("values", comp);

	std::string id = comp["id"];
	std::string location = comp["location"];
	int pins = comp["pins"];
	std::vector<double> values = comp["values"].get<std::vector<double>>();
	return LoadPQ(id, location, pins, values);
}

AC_source ComponentBuilder::buildACSource(const JSON& comp) {
	findNonEmptyNumericArray("values", comp);
	std::string id = comp["id"];
	std::string location = comp["location"];
	int pins = comp["pins"];
	double V = comp["voltage"];
	std::vector<double> values = comp["values"].get<std::vector<double>>();
	return AC_source(id, location, pins, V, values);
}

Generator ComponentBuilder::buildGenerator(const JSON& comp) {
	findNonEmptyNumericArray("values", comp);
	std::string id = comp["id"];
	std::string location = comp["location"];
	int pins = comp["pins"];
	double V = comp["voltage"];
	std::vector<double> values = comp["values"].get<std::vector<double>>();
	return Generator(id, location, pins, V, values);
}

Transformer_real ComponentBuilder::buildTransformerReal(const JSON& comp) {

	if (comp.contains("values")) {
		findNumber("R_primary", comp["values"]);
		findNumber("X_primary", comp["values"]);
		findNumber("R_secondary", comp["values"]);
		findNumber("X_secondary", comp["values"]);
		findNumber("turns_ratio", comp["values"]);
		findNumber("phase_shift", comp["values"]);

		std::vector<double> values;
		values.push_back(comp["values"]["R_primary"]);
		values.push_back(comp["values"]["X_primary"]);
		values.push_back(comp["values"]["R_secondary"]);
		values.push_back(comp["values"]["X_secondary"]);
		values.push_back(comp["values"]["turns_ratio"]);
		values.push_back(comp["values"]["phase_shift"]);

		std::string id = comp["id"];
		std::string location = comp["location"];
		int pins = comp["pins"];
		return Transformer_real(id, location, pins, values);

	}
}

void ComponentBuilder::findNumber(std::string key, const JSON& j) {
	if (!j.contains(key) || !j[key].is_number()) {
		throw std::invalid_argument("ERROR: missing or invalid '" + key + "' in component.\n");
	}
}

void ComponentBuilder::findNonEmptyString(std::string key, const JSON& j) {
	if (!j.contains(key) || !j[key].is_string() || j[key].get<std::string>().empty()) {
		throw std::invalid_argument("ERROR: missing, invalid or empty '" + key + "' in component.\n");
	}
}

void ComponentBuilder::findNonEmptyNumericArray(std::string key, const JSON& j, int arraysize) {
	// Check key exists and has right type
	if (!j.contains(key) || !j[key].is_array() || j[key].empty()) {
		throw std::invalid_argument("ERROR: missing, invalid or empty '" + key + "' in component.\n");
	}

	// Check all elements are numeric
	for (size_t i = 0; i < j[key].size(); ++i) {
		if (!j[key][i].is_number()) {
			throw std::invalid_argument("ERROR: '" + key + "' in component contains a non-numeric element at index " + std::to_string(i));
		}
	}

	// Check size if arraysize is specified
	if (arraysize != -1 && static_cast<int>(j[key].size()) != arraysize) {
		throw std::runtime_error("ERROR: '" + key + "' in component must have exactly " + std::to_string(arraysize) + " elements.");
	}
}