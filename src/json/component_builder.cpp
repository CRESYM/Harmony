/**
 * @file component_builder.cpp
 * @brief Implementation of JSON-based Element factory (ComponentBuilder).
 */
#include "component_builder.h"

#include <symengine/matrix.h>


std::unique_ptr<Element> ComponentBuilder::buildFromJSON(const JSON& comp, const unsigned int i) {
	std::string comptype = "unknown";

	try {
		findNonEmptyString("type", comp);
		findNonEmptyString("id", comp);

		comptype = comp["type"];
		std::transform(comptype.begin(), comptype.end(), comptype.begin(),
			[](unsigned char c) { return static_cast<char>(std::tolower(c)); });

		Element* raw = nullptr;
		if (comptype == "load") { raw = buildLoad(comp); }
		else if (comptype == "load_pq") { raw = buildLoadPQ(comp); }
		else if (comptype == "ac_source") { raw = buildACSource(comp); }
		else if (comptype == "generator") { raw = buildGenerator(comp); }
		else if (comptype == "dc_source") { raw = buildDCSource(comp); }
		else if (comptype == "capacitor") { raw = buildCapacitor(comp); }
		else if (comptype == "inductor") { raw = buildInductor(comp); }
		else if (comptype == "resistor") { raw = buildResistor(comp); }
		else if (comptype == "impedance") { raw = buildImpedance(comp); }
		else if (comptype == "admittance") { raw = buildAdmittance(comp); }
		else if (comptype == "switch") { raw = buildSwitch(comp); }
		else if (comptype == "transmission_line") { raw = buildTransmissionLine(comp); }
		else if (comptype == "cable") { raw = buildCable(comp); }
		else if (comptype == "overhead_line") { raw = buildOverheadLine(comp); }
		else if (comptype == "mmc") { raw = buildMMC(comp); }
		else if (comptype == "wt_type_3") { raw = buildWTtype3(comp); }
		else if (comptype == "wt_type_4") { raw = buildWTtype4(comp); }
		else if (comptype == "wp_plant") { raw = buildWPplant(comp); }
		else if (comptype == "pv_plant") { raw = buildPVplant(comp); }
		else if (comptype == "transformer_real") { raw = buildTransformerReal(comp); }
		else if (comptype == "transformer_classic") { raw = buildTransformerClassic(comp); }
		else if (comptype == "transformer_yy") { raw = buildTransformerYY(comp); }
		else if (comptype == "transformer_yy_real") { raw = buildTransformerYYReal(comp); }
		else if (comptype == "transformer_deltay") { raw = buildTransformerDeltaY(comp); }
		else if (comptype == "transformer_deltay_real") { raw = buildTransformerDeltaYReal(comp); }
		else if (comptype == "transformer_ydelta") { raw = buildTransformerYDelta(comp); }
		else if (comptype == "transformer_deltadelta") { raw = buildTransformerDeltaDelta(comp); }
		else {
			throw std::invalid_argument("unknown type '" + comptype + "'");
		}

		return std::unique_ptr<Element>(raw);
	}
	catch (const std::invalid_argument& e) {
		std::cerr << e.what();
		throw std::invalid_argument(
			"ERROR: unable to create component [" + std::to_string(i) + "] of type " + comptype + ".\n");
	}
}


std::vector<double> ComponentBuilder::readNumericArray(const JSON& comp, const std::string& key) {
	findNonEmptyNumericArray(key, comp);
	return comp.at(key).get<std::vector<double>>();
}


DenseMatrix ComponentBuilder::readDenseMatrixValues(const JSON& comp, const std::string& key) {
	const auto values = readNumericArray(comp, key);
	DenseMatrix matrix(1, static_cast<unsigned>(values.size()));
	for (size_t i = 0; i < values.size(); ++i) {
		matrix.set(0, static_cast<unsigned>(i), real_double(values[i]));
	}
	return matrix;
}


std::vector<double> ComponentBuilder::readVoltageVector(const JSON& comp) {
	if (comp.at("voltage").is_number()) {
		return { comp.at("voltage").get<double>() };
	}
	if (comp.at("voltage").is_array()) {
		return comp.at("voltage").get<std::vector<double>>();
	}
	throw std::invalid_argument("ERROR: 'voltage' must be a number or numeric array.\n");
}


Capacitor* ComponentBuilder::buildCapacitor(const JSON& comp) {
	return new Capacitor(comp["id"], comp["location"], comp["pins"], readNumericArray(comp, "values"));
}

Inductor* ComponentBuilder::buildInductor(const JSON& comp) {
	return new Inductor(comp["id"], comp["location"], comp["pins"], readNumericArray(comp, "values"));
}

Resistor* ComponentBuilder::buildResistor(const JSON& comp) {
	return new Resistor(comp["id"], comp["location"], comp["pins"], readNumericArray(comp, "values"));
}

Impedance* ComponentBuilder::buildImpedance(const JSON& comp) {
	return new Impedance(comp["id"], comp["location"], comp["pins"], readNumericArray(comp, "values"));
}

Admittance* ComponentBuilder::buildAdmittance(const JSON& comp) {
	return new Admittance(comp["id"], comp["location"], comp["pins"], readDenseMatrixValues(comp, "values"));
}

Load* ComponentBuilder::buildLoad(const JSON& comp) {
	return new Load(comp["id"], comp["location"], comp["pins"], readNumericArray(comp, "values"));
}

LoadPQ* ComponentBuilder::buildLoadPQ(const JSON& comp) {
	return new LoadPQ(comp["id"], comp["location"], comp["pins"], readNumericArray(comp, "values"));
}

AC_source* ComponentBuilder::buildACSource(const JSON& comp) {
	findNumber("voltage", comp);
	return new AC_source(
		comp["id"], comp["location"], comp["pins"],
		comp["voltage"].get<double>(),
		readNumericArray(comp, "values"));
}

Generator* ComponentBuilder::buildGenerator(const JSON& comp) {
	findNumber("voltage", comp);
	return new Generator(
		comp["id"], comp["location"], comp["pins"],
		comp["voltage"].get<double>(),
		readNumericArray(comp, "values"));
}

DC_source* ComponentBuilder::buildDCSource(const JSON& comp) {
	const auto voltages = readVoltageVector(comp);
	const double resistance = comp.value("resistance", 0.0);
	if (voltages.size() == 1) {
		return new DC_source(comp["id"], comp["location"], comp["pins"], voltages[0], resistance);
	}
	if (comp.contains("values")) {
		return new DC_source(comp["id"], comp["location"], comp["pins"], voltages, readNumericArray(comp, "values"));
	}
	return new DC_source(comp["id"], comp["location"], comp["pins"], voltages, resistance);
}

Switch* ComponentBuilder::buildSwitch(const JSON& comp) {
	const int pinCount = comp["pins"].get<int>();
	std::vector<bool> state(static_cast<size_t>(pinCount), true);
	if (comp.contains("state") && comp.at("state").is_array()) {
		const auto& arr = comp.at("state");
		for (size_t i = 0; i < arr.size() && i < state.size(); ++i) {
			state[i] = arr.at(i).get<bool>();
		}
	}
	else if (comp.contains("closed")) {
		const bool closed = comp.at("closed").get<bool>();
		std::fill(state.begin(), state.end(), closed);
	}
	return new Switch(comp["id"], comp["location"], pinCount, state);
}

TransmissionLine* ComponentBuilder::buildTransmissionLine(const JSON& comp) {
	return new TransmissionLine(comp["id"], comp["location"], comp["pins"], readNumericArray(comp, "values"));
}

Cable* ComponentBuilder::buildCable(const JSON& comp) {
	std::map<string, Cable::Conductor*> conductors;
	for (const auto& c : comp.at("conductors")) {
		const std::string key = c.at("id").get<std::string>();
		conductors[key] = new Cable::Conductor(
			c.value("ri", 0.0),
			c.value("ro", 0.0),
			c.value("resistivity", 0.0),
			c.value("permeability", 1.0),
			c.value("area", 0.0));
	}

	std::map<string, Cable::Insulator*> insulators;
	for (const auto& ins : comp.at("insulators")) {
		const std::string key = ins.at("id").get<std::string>();
		insulators[key] = new Cable::Insulator(
			ins.value("ri", 0.0),
			ins.value("ro", 0.0),
			ins.value("permittivity", 1.0),
			ins.value("permeability", 1.0),
			ins.value("inner_semiconductor_outer_radius", 0.0),
			ins.value("outer_semiconductor_inner_radius", 0.0));
	}

	const auto& earthArr = comp.at("earth");
	std::tuple<double, double, double> earth = {
		earthArr.at(0).get<double>(),
		earthArr.at(1).get<double>(),
		earthArr.at(2).get<double>()
	};

	std::vector<std::pair<double, double>> positions;
	for (const auto& pos : comp.at("positions")) {
		positions.emplace_back(pos.at(0).get<double>(), pos.at(1).get<double>());
	}

	return new Cable(
		comp["id"], comp["location"], comp["pins"],
		comp.at("cable_type").get<std::string>(),
		comp.at("length").get<double>(),
		earth, conductors, insulators, positions);
}

Overhead_Line* ComponentBuilder::buildOverheadLine(const JSON& comp) {
	const auto& earthArr = comp.at("earth");
	const std::tuple<double, double, double> earth = {
		earthArr.at(0).get<double>(),
		earthArr.at(1).get<double>(),
		earthArr.at(2).get<double>()
	};

	const JSON& cond = comp.at("conductor");
	std::vector<int> numbers = cond.at("number_bundles").get<std::vector<int>>();
	std::vector<double> geometry = cond.at("geometry").get<std::vector<double>>();
	const auto conductor = std::make_tuple(
		cond.at("organization").get<std::string>(),
		numbers,
		geometry,
		cond.value("ybc", 0.0),
		cond.value("delta_ybc", 0.0),
		cond.value("delta_xbc", 0.0),
		cond.value("delta_tilde_xbc", 0.0));

	const JSON& gw = comp.at("groundwire");
	std::vector<double> gwGeo = gw.value("geometry", std::vector<double>{});
	const auto groundwire = std::make_tuple(
		gw.at("count").get<int>(),
		gwGeo,
		gw.value("delta_xg", 0.0));

	return new Overhead_Line(
		comp["id"],
		comp.value("location", "AC1"),
		comp.at("length_km").get<double>(),
		earth, conductor, groundwire);
}

MMC* ComponentBuilder::buildMMC(const JSON& comp) {
	const auto converterParams = readNumericArray(comp, "converter_params");
	if (comp.contains("filter_params")) {
		return new MMC(
			comp["id"], comp["location"], converterParams,
			readNumericArray(comp, "controller_params"),
			readNumericArray(comp, "filter_params"));
	}
	if (comp.contains("controller_params")) {
		return new MMC(
			comp["id"], comp["location"], converterParams,
			readNumericArray(comp, "controller_params"));
	}
	return new MMC(comp["id"], comp["location"], converterParams);
}

WTtype3* ComponentBuilder::buildWTtype3(const JSON& comp) {
	return new WTtype3(comp["id"], comp["location"], readNumericArray(comp, "parameters"));
}

WTtype4* ComponentBuilder::buildWTtype4(const JSON& comp) {
	return new WTtype4(comp["id"], comp["location"], readNumericArray(comp, "parameters"));
}

WPplant* ComponentBuilder::buildWPplant(const JSON& comp) {
	return new WPplant(
		comp["id"], comp["location"],
		comp.at("turbine_type").get<int>(),
		comp.at("number_wt").get<int>(),
		readNumericArray(comp, "parameters"));
}

PVplant* ComponentBuilder::buildPVplant(const JSON& comp) {
	return new PVplant(comp["id"], comp["location"], readNumericArray(comp, "parameters"));
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
	return new Transformer_real(comp["id"], comp["location"], comp["pins"], readTransformerRealValues(comp));
}

Transformer_classic* ComponentBuilder::buildTransformerClassic(const JSON& comp) {
	return new Transformer_classic(comp["id"], comp["location"], comp["pins"], readTransformerClassicValues(comp));
}

TransformerYY* ComponentBuilder::buildTransformerYY(const JSON& comp) {
	return new TransformerYY(comp["id"], comp["location"], comp["pins"], readTransformerClassicValues(comp));
}

TransformerYY_real* ComponentBuilder::buildTransformerYYReal(const JSON& comp) {
	return new TransformerYY_real(comp["id"], comp["location"], comp["pins"], readTransformerRealValues(comp));
}

TransformerDeltaY* ComponentBuilder::buildTransformerDeltaY(const JSON& comp) {
	return new TransformerDeltaY(comp["id"], comp["location"], comp["pins"], readTransformerClassicValues(comp));
}

TransformerDeltaY_real* ComponentBuilder::buildTransformerDeltaYReal(const JSON& comp) {
	return new TransformerDeltaY_real(comp["id"], comp["location"], comp["pins"], readTransformerRealValues(comp));
}

TransformerYDelta* ComponentBuilder::buildTransformerYDelta(const JSON& comp) {
	return new TransformerYDelta(comp["id"], comp["location"], comp["pins"], readTransformerClassicValues(comp));
}

TransformerDeltaDelta* ComponentBuilder::buildTransformerDeltaDelta(const JSON& comp) {
	return new TransformerDeltaDelta(comp["id"], comp["location"], comp["pins"], readTransformerClassicValues(comp));
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
