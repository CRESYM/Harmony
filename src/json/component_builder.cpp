/**
 * @file component_builder.cpp
 * @brief Implementation of JSON-based Element factory (ComponentBuilder).
 */
#include "component_builder.h"

#include <complex>
#include <map>
#include <symengine/matrix.h>


namespace {

std::map<std::string, double> readOpfInfo(const JSON& comp, const JsonParameterTable& params) {
	std::map<std::string, double> info;
	if (!comp.contains("opf_info") || !comp.at("opf_info").is_object()) {
		return info;
	}
	for (auto it = comp.at("opf_info").begin(); it != comp.at("opf_info").end(); ++it) {
		info[it.key()] = params.resolveScalar(it.value(), "opf_info");
	}
	return info;
}

void applyOpfInfo(Element* element, const JSON& comp, const JsonParameterTable& params) {
	if (!comp.contains("opf_info")) {
		return;
	}
	auto info = readOpfInfo(comp, params);
	if (!info.empty()) {
		element->setOPFInfo(info);
	}
}

} // namespace


Admittance* ComponentBuilder::buildSymbolicAdmittance(
	const JSON& comp,
	const JsonParameterTable& params,
	const char* exprField)
{
	const auto y = JsonExpression::parseAdmittance(
		comp.at(exprField).get<std::string>(), params, exprField);
	DenseMatrix matrix(1, 1);
	matrix.set(0, 0, y);
	return new Admittance(comp["id"], comp["location"], comp["pins"], matrix);
}


std::unique_ptr<Element> ComponentBuilder::buildFromJSON(
	const JSON& comp,
	const unsigned int i,
	const JsonParameterTable& params)
{
	std::string comptype = "unknown";

	try {
		findNonEmptyString("type", comp);
		findNonEmptyString("id", comp);

		comptype = comp["type"];
		std::transform(comptype.begin(), comptype.end(), comptype.begin(),
			[](unsigned char c) { return static_cast<char>(std::tolower(c)); });

		Element* raw = nullptr;
		if (comptype == "load") { raw = buildLoad(comp, params); }
		else if (comptype == "load_pq") { raw = buildLoadPQ(comp, params); }
		else if (comptype == "ac_source") { raw = buildACSource(comp, params); }
		else if (comptype == "generator") { raw = buildGenerator(comp, params); }
		else if (comptype == "dc_source") { raw = buildDCSource(comp, params); }
		else if (comptype == "capacitor") {
			if (comp.contains("y_expr")) {
				raw = buildSymbolicAdmittance(comp, params, "y_expr");
			}
			else {
				raw = buildCapacitor(comp, params);
			}
		}
		else if (comptype == "inductor") {
			if (comp.contains("y_expr")) {
				raw = buildSymbolicAdmittance(comp, params, "y_expr");
			}
			else {
				raw = buildInductor(comp, params);
			}
		}
		else if (comptype == "resistor") {
			if (comp.contains("y_expr")) {
				raw = buildSymbolicAdmittance(comp, params, "y_expr");
			}
			else {
				raw = buildResistor(comp, params);
			}
		}
		else if (comptype == "impedance") { raw = buildImpedance(comp, params); }
		else if (comptype == "admittance") { raw = buildAdmittance(comp, params); }
		else if (comptype == "switch") { raw = buildSwitch(comp); }
		else if (comptype == "transmission_line") { raw = buildTransmissionLine(comp, params); }
		else if (comptype == "cable") { raw = buildCable(comp, params); }
		else if (comptype == "overhead_line") { raw = buildOverheadLine(comp, params); }
		else if (comptype == "mmc") { raw = buildMMC(comp, params); }
		else if (comptype == "wt_type_3") { raw = buildWTtype3(comp, params); }
		else if (comptype == "wt_type_4") { raw = buildWTtype4(comp, params); }
		else if (comptype == "wp_plant") { raw = buildWPplant(comp, params); }
		else if (comptype == "pv_plant") { raw = buildPVplant(comp, params); }
		else if (comptype == "transformer_real") { raw = buildTransformerReal(comp, params); }
		else if (comptype == "transformer_classic") { raw = buildTransformerClassic(comp, params); }
		else if (comptype == "transformer_yy") { raw = buildTransformerYY(comp, params); }
		else if (comptype == "transformer_yy_real") { raw = buildTransformerYYReal(comp, params); }
		else if (comptype == "transformer_deltay") { raw = buildTransformerDeltaY(comp, params); }
		else if (comptype == "transformer_deltay_real") { raw = buildTransformerDeltaYReal(comp, params); }
		else if (comptype == "transformer_ydelta") { raw = buildTransformerYDelta(comp, params); }
		else if (comptype == "transformer_deltadelta") { raw = buildTransformerDeltaDelta(comp, params); }
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


std::vector<double> ComponentBuilder::readNumericArray(
	const JSON& comp,
	const std::string& key,
	const JsonParameterTable& params)
{
	findNumericOrReferenceArray(key, comp, params);
	return params.resolveArray(comp.at(key), ("'" + key + "'").c_str());
}


DenseMatrix ComponentBuilder::readDenseMatrixValues(
	const JSON& comp,
	const std::string& key,
	const JsonParameterTable& params)
{
	const auto values = readNumericArray(comp, key, params);
	DenseMatrix matrix(1, static_cast<unsigned>(values.size()));
	for (size_t i = 0; i < values.size(); ++i) {
		matrix.set(0, static_cast<unsigned>(i), real_double(values[i]));
	}
	return matrix;
}


double ComponentBuilder::readScalarField(
	const JSON& comp,
	const std::string& key,
	const JsonParameterTable& params)
{
	findScalar(key, comp, params);
	return params.resolveScalar(comp.at(key), ("'" + key + "'").c_str());
}


std::vector<double> ComponentBuilder::readVoltageVector(const JSON& comp, const JsonParameterTable& params) {
	if (comp.at("voltage").is_number() || JsonParameterTable::isReference(comp.at("voltage"))) {
		return { readScalarField(comp, "voltage", params) };
	}
	if (comp.at("voltage").is_array()) {
		findNumericOrReferenceArray("voltage", comp, params);
		return params.resolveArray(comp.at("voltage"), "'voltage'");
	}
	throw std::invalid_argument("ERROR: 'voltage' must be a number, parameter name, or numeric array.\n");
}


Capacitor* ComponentBuilder::buildCapacitor(const JSON& comp, const JsonParameterTable& params) {
	return new Capacitor(comp["id"], comp["location"], comp["pins"], readNumericArray(comp, "values", params));
}

Inductor* ComponentBuilder::buildInductor(const JSON& comp, const JsonParameterTable& params) {
	return new Inductor(comp["id"], comp["location"], comp["pins"], readNumericArray(comp, "values", params));
}

Resistor* ComponentBuilder::buildResistor(const JSON& comp, const JsonParameterTable& params) {
	return new Resistor(comp["id"], comp["location"], comp["pins"], readNumericArray(comp, "values", params));
}

Impedance* ComponentBuilder::buildImpedance(const JSON& comp, const JsonParameterTable& params) {
	const int pins = comp["pins"].get<int>();
	if (comp.contains("z_expr")) {
		const auto z = JsonExpression::parseImpedance(
			comp.at("z_expr").get<std::string>(), params, "z_expr");
		DenseMatrix matrix(1, 1);
		matrix.set(0, 0, z);
		return new Impedance(comp["id"], comp["location"], pins, matrix);
	}
	if (comp.contains("complex")) {
		const auto z = readNumericArray(comp, "complex", params);
		if (z.size() != 2) {
			throw std::invalid_argument("impedance 'complex' must contain [R, X]");
		}
		return new Impedance(
			comp["id"], comp["location"], pins,
			std::complex<double>(z[0], z[1]));
	}

	const auto values = readNumericArray(comp, "values", params);
	if (values.size() == 1u) {
		return new Impedance(comp["id"], comp["location"], pins, values[0]);
	}
	return new Impedance(comp["id"], comp["location"], pins, values);
}

Admittance* ComponentBuilder::buildAdmittance(const JSON& comp, const JsonParameterTable& params) {
	if (comp.contains("y_expr")) {
		return buildSymbolicAdmittance(comp, params, "y_expr");
	}
	if (comp.contains("y_exprs")) {
		const auto exprs = JsonExpression::parseArray(comp.at("y_exprs"), params, "y_exprs");
		return new Admittance(
			comp["id"], comp["location"], comp["pins"],
			JsonExpression::toRowMatrix(exprs));
	}
	return new Admittance(comp["id"], comp["location"], comp["pins"], readDenseMatrixValues(comp, "values", params));
}

Load* ComponentBuilder::buildLoad(const JSON& comp, const JsonParameterTable& params) {
	return new Load(comp["id"], comp["location"], comp["pins"], readNumericArray(comp, "values", params));
}

LoadPQ* ComponentBuilder::buildLoadPQ(const JSON& comp, const JsonParameterTable& params) {
	return new LoadPQ(comp["id"], comp["location"], comp["pins"], readNumericArray(comp, "values", params));
}

AC_source* ComponentBuilder::buildACSource(const JSON& comp, const JsonParameterTable& params) {
	findScalar("voltage", comp, params);
	const auto values = readNumericArray(comp, "values", params);
	const double voltage = readScalarField(comp, "voltage", params);
	AC_source* src = nullptr;
	if (values.size() == 1u) {
		src = new AC_source(comp["id"], comp["location"], comp["pins"], voltage, values[0]);
	}
	else {
		src = new AC_source(comp["id"], comp["location"], comp["pins"], voltage, values);
	}
	applyOpfInfo(src, comp, params);
	return src;
}

Generator* ComponentBuilder::buildGenerator(const JSON& comp, const JsonParameterTable& params) {
	findScalar("voltage", comp, params);
	const auto values = readNumericArray(comp, "values", params);
	Generator* gen = new Generator(
		comp["id"], comp["location"], comp["pins"],
		readScalarField(comp, "voltage", params), values);
	applyOpfInfo(gen, comp, params);
	return gen;
}

DC_source* ComponentBuilder::buildDCSource(const JSON& comp, const JsonParameterTable& params) {
	const auto voltages = readVoltageVector(comp, params);
	const double resistance = comp.contains("resistance")
		? readScalarField(comp, "resistance", params)
		: 0.0;
	if (voltages.size() == 1) {
		if (comp.contains("values")) {
			const auto z = readNumericArray(comp, "values", params);
			if (z.size() == 1) {
				return new DC_source(comp["id"], comp["location"], comp["pins"], voltages[0], z[0]);
			}
			return new DC_source(comp["id"], comp["location"], comp["pins"], voltages[0], z);
		}
		return new DC_source(comp["id"], comp["location"], comp["pins"], voltages[0], resistance);
	}
	if (comp.contains("values")) {
		const auto z = readNumericArray(comp, "values", params);
		if (z.size() == 1) {
			return new DC_source(comp["id"], comp["location"], comp["pins"], voltages, z[0]);
		}
		return new DC_source(comp["id"], comp["location"], comp["pins"], voltages, z);
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

TransmissionLine* ComponentBuilder::buildTransmissionLine(const JSON& comp, const JsonParameterTable& params) {
	return new TransmissionLine(comp["id"], comp["location"], comp["pins"], readNumericArray(comp, "values", params));
}

Cable* ComponentBuilder::buildCable(const JSON& comp, const JsonParameterTable& params) {
	std::map<string, Cable::Conductor*> conductors;
	for (const auto& c : comp.at("conductors")) {
		const std::string key = c.at("id").get<std::string>();
		conductors[key] = new Cable::Conductor(
			params.resolveScalar(c.value("ri", JSON(0.0)), "cable conductor ri"),
			params.resolveScalar(c.value("ro", JSON(0.0)), "cable conductor ro"),
			params.resolveScalar(c.value("resistivity", JSON(0.0)), "cable conductor resistivity"),
			params.resolveScalar(c.value("permeability", JSON(1.0)), "cable conductor permeability"),
			params.resolveScalar(c.value("area", JSON(0.0)), "cable conductor area"));
	}

	std::map<string, Cable::Insulator*> insulators;
	for (const auto& ins : comp.at("insulators")) {
		const std::string key = ins.at("id").get<std::string>();
		insulators[key] = new Cable::Insulator(
			params.resolveScalar(ins.value("ri", JSON(0.0)), "cable insulator ri"),
			params.resolveScalar(ins.value("ro", JSON(0.0)), "cable insulator ro"),
			params.resolveScalar(ins.value("permittivity", JSON(1.0)), "cable insulator permittivity"),
			params.resolveScalar(ins.value("permeability", JSON(1.0)), "cable insulator permeability"),
			params.resolveScalar(ins.value("inner_semiconductor_outer_radius", JSON(0.0)), "cable insulator inner_semiconductor_outer_radius"),
			params.resolveScalar(ins.value("outer_semiconductor_inner_radius", JSON(0.0)), "cable insulator outer_semiconductor_inner_radius"));
	}

	const auto& earthArr = comp.at("earth");
	std::tuple<double, double, double> earth = {
		params.resolveScalar(earthArr.at(0), "cable earth"),
		params.resolveScalar(earthArr.at(1), "cable earth"),
		params.resolveScalar(earthArr.at(2), "cable earth")
	};

	std::vector<std::pair<double, double>> positions;
	for (const auto& pos : comp.at("positions")) {
		positions.emplace_back(
			params.resolveScalar(pos.at(0), "cable position"),
			params.resolveScalar(pos.at(1), "cable position"));
	}

	return new Cable(
		comp["id"], comp["location"],
		comp.contains("pins") ? static_cast<int>(readScalarField(comp, "pins", params)) : 1,
		comp.at("cable_type").get<std::string>(),
		readScalarField(comp, "length", params),
		earth, conductors, insulators, positions);
}

Overhead_Line* ComponentBuilder::buildOverheadLine(const JSON& comp, const JsonParameterTable& params) {
	const auto& earthArr = comp.at("earth");
	const std::tuple<double, double, double> earth = {
		params.resolveScalar(earthArr.at(0), "overhead_line earth"),
		params.resolveScalar(earthArr.at(1), "overhead_line earth"),
		params.resolveScalar(earthArr.at(2), "overhead_line earth")
	};

	const JSON& cond = comp.at("conductor");
	params.validateNumericOrReferenceArray(cond.at("number_bundles"), "overhead_line conductor number_bundles");
	params.validateNumericOrReferenceArray(cond.at("geometry"), "overhead_line conductor geometry");
	const auto numbers = params.resolveArray(cond.at("number_bundles"), "overhead_line conductor number_bundles");
	std::vector<int> numbersInt;
	numbersInt.reserve(numbers.size());
	for (double n : numbers) {
		numbersInt.push_back(static_cast<int>(n));
	}
	const auto geometry = params.resolveArray(cond.at("geometry"), "overhead_line conductor geometry");
	const auto conductor = std::make_tuple(
		cond.at("organization").get<std::string>(),
		numbersInt,
		geometry,
		params.resolveScalar(cond.value("ybc", JSON(0.0)), "overhead_line ybc"),
		params.resolveScalar(cond.value("delta_ybc", JSON(0.0)), "overhead_line delta_ybc"),
		params.resolveScalar(cond.value("delta_xbc", JSON(0.0)), "overhead_line delta_xbc"),
		params.resolveScalar(cond.value("delta_tilde_xbc", JSON(0.0)), "overhead_line delta_tilde_xbc"));

	const JSON& gw = comp.at("groundwire");
	std::vector<double> gwGeo;
	if (gw.contains("geometry")) {
		params.validateNumericOrReferenceArray(gw.at("geometry"), "overhead_line groundwire geometry");
		gwGeo = params.resolveArray(gw.at("geometry"), "overhead_line groundwire geometry");
	}
	const auto groundwire = std::make_tuple(
		static_cast<int>(params.resolveScalar(gw.at("count"), "overhead_line groundwire count")),
		gwGeo,
		params.resolveScalar(gw.value("delta_xg", JSON(0.0)), "overhead_line delta_xg"));

	return new Overhead_Line(
		comp["id"],
		comp.value("location", "AC1"),
		readScalarField(comp, "length_km", params),
		earth, conductor, groundwire);
}

MMC* ComponentBuilder::buildMMC(const JSON& comp, const JsonParameterTable& params) {
	const auto converterParams = readNumericArray(comp, "converter_params", params);
	if (comp.contains("filter_params")) {
		return new MMC(
			comp["id"], comp["location"], converterParams,
			readNumericArray(comp, "controller_params", params),
			readNumericArray(comp, "filter_params", params));
	}
	if (comp.contains("controller_params")) {
		return new MMC(
			comp["id"], comp["location"], converterParams,
			readNumericArray(comp, "controller_params", params));
	}
	return new MMC(comp["id"], comp["location"], converterParams);
}

WTtype3* ComponentBuilder::buildWTtype3(const JSON& comp, const JsonParameterTable& params) {
	return new WTtype3(comp["id"], comp["location"], readNumericArray(comp, "parameters", params));
}

WTtype4* ComponentBuilder::buildWTtype4(const JSON& comp, const JsonParameterTable& params) {
	return new WTtype4(comp["id"], comp["location"], readNumericArray(comp, "parameters", params));
}

WPplant* ComponentBuilder::buildWPplant(const JSON& comp, const JsonParameterTable& params) {
	return new WPplant(
		comp["id"], comp["location"],
		static_cast<int>(readScalarField(comp, "turbine_type", params)),
		static_cast<int>(readScalarField(comp, "number_wt", params)),
		readNumericArray(comp, "parameters", params));
}

PVplant* ComponentBuilder::buildPVplant(const JSON& comp, const JsonParameterTable& params) {
	return new PVplant(comp["id"], comp["location"], readNumericArray(comp, "parameters", params));
}

std::vector<double> ComponentBuilder::readTransformerClassicValues(
	const JSON& comp,
	const JsonParameterTable& params)
{
	if (!comp.contains("values") || !comp["values"].is_object()) {
		throw std::invalid_argument("ERROR: transformer requires object 'values'.\n");
	}
	const JSON& v = comp["values"];
	findScalar("R_primary", v, params);
	findScalar("L_primary", v, params);
	findScalar("R_secondary", v, params);
	findScalar("L_secondary", v, params);
	findScalar("M", v, params);
	return {
		readScalarField(v, "R_primary", params),
		readScalarField(v, "L_primary", params),
		readScalarField(v, "R_secondary", params),
		readScalarField(v, "L_secondary", params),
		readScalarField(v, "M", params)
	};
}

std::vector<double> ComponentBuilder::readTransformerRealValues(
	const JSON& comp,
	const JsonParameterTable& params)
{
	if (!comp.contains("values") || !comp["values"].is_object()) {
		throw std::invalid_argument("ERROR: transformer requires object 'values'.\n");
	}
	const JSON& v = comp["values"];
	findScalar("R_primary", v, params);
	findScalar("L_primary", v, params);
	findScalar("R_secondary", v, params);
	findScalar("L_secondary", v, params);
	findScalar("turns_ratio", v, params);
	findScalar("phase_shift", v, params);
	return {
		readScalarField(v, "R_primary", params),
		readScalarField(v, "L_primary", params),
		readScalarField(v, "R_secondary", params),
		readScalarField(v, "L_secondary", params),
		readScalarField(v, "turns_ratio", params),
		readScalarField(v, "phase_shift", params)
	};
}

Transformer_real* ComponentBuilder::buildTransformerReal(const JSON& comp, const JsonParameterTable& params) {
	return new Transformer_real(comp["id"], comp["location"], comp["pins"], readTransformerRealValues(comp, params));
}

Transformer_classic* ComponentBuilder::buildTransformerClassic(const JSON& comp, const JsonParameterTable& params) {
	return new Transformer_classic(comp["id"], comp["location"], comp["pins"], readTransformerClassicValues(comp, params));
}

TransformerYY* ComponentBuilder::buildTransformerYY(const JSON& comp, const JsonParameterTable& params) {
	return new TransformerYY(comp["id"], comp["location"], comp["pins"], readTransformerClassicValues(comp, params));
}

TransformerYY_real* ComponentBuilder::buildTransformerYYReal(const JSON& comp, const JsonParameterTable& params) {
	return new TransformerYY_real(comp["id"], comp["location"], comp["pins"], readTransformerRealValues(comp, params));
}

TransformerDeltaY* ComponentBuilder::buildTransformerDeltaY(const JSON& comp, const JsonParameterTable& params) {
	return new TransformerDeltaY(comp["id"], comp["location"], comp["pins"], readTransformerClassicValues(comp, params));
}

TransformerDeltaY_real* ComponentBuilder::buildTransformerDeltaYReal(const JSON& comp, const JsonParameterTable& params) {
	return new TransformerDeltaY_real(comp["id"], comp["location"], comp["pins"], readTransformerRealValues(comp, params));
}

TransformerYDelta* ComponentBuilder::buildTransformerYDelta(const JSON& comp, const JsonParameterTable& params) {
	return new TransformerYDelta(comp["id"], comp["location"], comp["pins"], readTransformerClassicValues(comp, params));
}

TransformerDeltaDelta* ComponentBuilder::buildTransformerDeltaDelta(const JSON& comp, const JsonParameterTable& params) {
	return new TransformerDeltaDelta(comp["id"], comp["location"], comp["pins"], readTransformerClassicValues(comp, params));
}

void ComponentBuilder::findNumber(const std::string& key, const JSON& j) {
	if (!j.contains(key) || !j[key].is_number()) {
		throw std::invalid_argument("ERROR: missing or invalid '" + key + "' in component.\n");
	}
}


void ComponentBuilder::findScalar(const std::string& key, const JSON& j, const JsonParameterTable& params) {
	if (!j.contains(key)) {
		throw std::invalid_argument("ERROR: missing '" + key + "' in component.\n");
	}
	const JSON& value = j.at(key);
	if (value.is_number()) {
		return;
	}
	if (JsonParameterTable::isReference(value) && params.contains(value.get<std::string>())) {
		return;
	}
	throw std::invalid_argument("ERROR: missing or invalid '" + key + "' in component.\n");
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


void ComponentBuilder::findNumericOrReferenceArray(
	const std::string& key,
	const JSON& j,
	const JsonParameterTable& params,
	int arraysize)
{
	if (!j.contains(key)) {
		throw std::invalid_argument("ERROR: missing '" + key + "' in component.\n");
	}
	params.validateNumericOrReferenceArray(j.at(key), ("'" + key + "'").c_str(), arraysize);
}
