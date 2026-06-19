#include "component_builder.h"


Element* ComponentBuilder::buildFromJSON(const JSON& comp, const unsigned int i) {

	std::string comptype = "unknown";

	try {
		findNonEmptyString("type", comp);
		findNonEmptyString("id", comp);
		findNonEmptyString("location", comp);
		findNumber("pins", comp);

		comptype = comp["type"];
		std::transform(comptype.begin(), comptype.end(), comptype.begin(),
			[](unsigned char c) { return static_cast<char>(std::tolower(c)); });

		if (comptype == "load") { return buildLoad(comp); }
		if (comptype == "load_pq") { return buildLoadPQ(comp); }
		if (comptype == "ac_source") { return buildACSource(comp); }
		if (comptype == "generator") { return buildGenerator(comp); }
		if (comptype == "capacitor") { return buildCapacitor(comp); }
		if (comptype == "inductor") { return buildInductor(comp); }
		if (comptype == "resistor") { return buildResistor(comp); }
		if (comptype == "transformer_real") { return buildTransformerReal(comp); }
		if (comptype == "transformer_classic") { return buildTransformerClassic(comp); }
		if (comptype == "transformer_yy") { return buildTransformerYY(comp); }
		if (comptype == "transformer_yy_real") { return buildTransformerYYReal(comp); }
		if (comptype == "transformer_deltay") { return buildTransformerDeltaY(comp); }
		if (comptype == "transformer_deltay_real") { return buildTransformerDeltaYReal(comp); }
		if (comptype == "transformer_ydelta") { return buildTransformerYDelta(comp); }
		if (comptype == "transformer_deltadelta") { return buildTransformerDeltaDelta(comp); }

		throw std::invalid_argument("unknown type '" + comptype + "'");
	}
	catch (const std::invalid_argument& e) {
		std::cerr << e.what();
		throw std::invalid_argument(
			"ERROR: unable to create component [" + std::to_string(i) + "] of type " + comptype + ".\n");
	}
}

Capacitor* ComponentBuilder::buildCapacitor(const JSON& comp) {
	findNonEmptyNumericArray("values", comp);
	return new Capacitor(
		comp["id"], comp["location"], comp["pins"],
		comp["values"].get<std::vector<double>>());
}

Inductor* ComponentBuilder::buildInductor(const JSON& comp) {
	findNonEmptyNumericArray("values", comp);
	return new Inductor(
		comp["id"], comp["location"], comp["pins"],
		comp["values"].get<std::vector<double>>());
}

Resistor* ComponentBuilder::buildResistor(const JSON& comp) {
	findNonEmptyNumericArray("values", comp);
	return new Resistor(
		comp["id"], comp["location"], comp["pins"],
		comp["values"].get<std::vector<double>>());
}

Load* ComponentBuilder::buildLoad(const JSON& comp) {
	findNonEmptyNumericArray("values", comp);
	return new Load(
		comp["id"], comp["location"], comp["pins"],
		comp["values"].get<std::vector<double>>());
}

LoadPQ* ComponentBuilder::buildLoadPQ(const JSON& comp) {
	findNonEmptyNumericArray("values", comp);
	return new LoadPQ(
		comp["id"], comp["location"], comp["pins"],
		comp["values"].get<std::vector<double>>());
}

AC_source* ComponentBuilder::buildACSource(const JSON& comp) {
	findNonEmptyNumericArray("values", comp);
	findNumber("voltage", comp);
	return new AC_source(
		comp["id"], comp["location"], comp["pins"],
		comp["voltage"].get<double>(),
		comp["values"].get<std::vector<double>>());
}

Generator* ComponentBuilder::buildGenerator(const JSON& comp) {
	findNonEmptyNumericArray("values", comp);
	findNumber("voltage", comp);
	return new Generator(
		comp["id"], comp["location"], comp["pins"],
		comp["voltage"].get<double>(),
		comp["values"].get<std::vector<double>>());
}

std::vector<double> ComponentBuilder::readTransformerClassicValues(const JSON& comp) {
	if (!comp.contains("values") || !comp["values"].is_object()) {
		throw std::invalid_argument("ERROR: transformer requires object 'values'.\n");
	}
	const JSON& v = comp["values"];
	findNumber("R_primary", v);
	findNumber("L_primary", v);
	findNumber("R_secondary", v);
	findNumber("L_secondary", v);
	findNumber("M", v);
	return {
		v["R_primary"].get<double>(),
		v["L_primary"].get<double>(),
		v["R_secondary"].get<double>(),
		v["L_secondary"].get<double>(),
		v["M"].get<double>()
	};
}

std::vector<double> ComponentBuilder::readTransformerRealValues(const JSON& comp) {
	if (!comp.contains("values") || !comp["values"].is_object()) {
		throw std::invalid_argument("ERROR: transformer requires object 'values'.\n");
	}
	const JSON& v = comp["values"];
	findNumber("R_primary", v);
	findNumber("L_primary", v);
	findNumber("R_secondary", v);
	findNumber("L_secondary", v);
	findNumber("turns_ratio", v);
	findNumber("phase_shift", v);
	return {
		v["R_primary"].get<double>(),
		v["L_primary"].get<double>(),
		v["R_secondary"].get<double>(),
		v["L_secondary"].get<double>(),
		v["turns_ratio"].get<double>(),
		v["phase_shift"].get<double>()
	};
}

Transformer_real* ComponentBuilder::buildTransformerReal(const JSON& comp) {
	auto values = readTransformerRealValues(comp);
	return new Transformer_real(comp["id"], comp["location"], comp["pins"], values);
}

Transformer_classic* ComponentBuilder::buildTransformerClassic(const JSON& comp) {
	auto values = readTransformerClassicValues(comp);
	return new Transformer_classic(comp["id"], comp["location"], comp["pins"], values);
}

TransformerYY* ComponentBuilder::buildTransformerYY(const JSON& comp) {
	auto values = readTransformerClassicValues(comp);
	return new TransformerYY(comp["id"], comp["location"], comp["pins"], values);
}

TransformerYY_real* ComponentBuilder::buildTransformerYYReal(const JSON& comp) {
	auto values = readTransformerRealValues(comp);
	return new TransformerYY_real(comp["id"], comp["location"], comp["pins"], values);
}

TransformerDeltaY* ComponentBuilder::buildTransformerDeltaY(const JSON& comp) {
	auto values = readTransformerClassicValues(comp);
	return new TransformerDeltaY(comp["id"], comp["location"], comp["pins"], values);
}

TransformerDeltaY_real* ComponentBuilder::buildTransformerDeltaYReal(const JSON& comp) {
	auto values = readTransformerRealValues(comp);
	return new TransformerDeltaY_real(comp["id"], comp["location"], comp["pins"], values);
}

TransformerYDelta* ComponentBuilder::buildTransformerYDelta(const JSON& comp) {
	auto values = readTransformerClassicValues(comp);
	return new TransformerYDelta(comp["id"], comp["location"], comp["pins"], values);
}

TransformerDeltaDelta* ComponentBuilder::buildTransformerDeltaDelta(const JSON& comp) {
	auto values = readTransformerClassicValues(comp);
	return new TransformerDeltaDelta(comp["id"], comp["location"], comp["pins"], values);
}

void ComponentBuilder::findNumber(const std::string& key, const JSON& j) {
	if (!j.contains(key) || !j[key].is_number()) {
		throw std::invalid_argument("ERROR: missing or invalid '" + key + "' in component.\n");
	}
}

void ComponentBuilder::findNonEmptyString(const std::string& key, const JSON& j) {
	if (!j.contains(key) || !j[key].is_string() || j[key].get<std::string>().empty()) {
		throw std::invalid_argument("ERROR: missing, invalid or empty '" + key + "' in component.\n");
	}
}

void ComponentBuilder::findNonEmptyNumericArray(const std::string& key, const JSON& j, int arraysize) {
	if (!j.contains(key) || !j[key].is_array() || j[key].empty()) {
		throw std::invalid_argument("ERROR: missing, invalid or empty '" + key + "' in component.\n");
	}

	for (size_t i = 0; i < j[key].size(); ++i) {
		if (!j[key][i].is_number()) {
			throw std::invalid_argument(
				"ERROR: '" + key + "' contains a non-numeric element at index " + std::to_string(i));
		}
	}

	if (arraysize != -1 && static_cast<int>(j[key].size()) != arraysize) {
		throw std::runtime_error(
			"ERROR: '" + key + "' must have exactly " + std::to_string(arraysize) + " elements.");
	}
}
