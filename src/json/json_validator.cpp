/**
 * @file json_validator.cpp
 * @brief Strict per-component JSON schema validation.
 */
#include "json_validator.h"

#include "component_builder.h"
#include "json_expression.h"
#include "json_parameters.h"

#include <cctype>


namespace {

std::string lowerType(const JSON& comp) {
	std::string type = comp.at("type").get<std::string>();
	std::transform(type.begin(), type.end(), type.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return type;
}

void requireKeys(const JSON& j, const std::set<std::string>& keys, const char* context) {
	for (const auto& key : keys) {
		if (!j.contains(key)) {
			throw std::invalid_argument(std::string("ERROR: missing '") + key + "' in " + context + ".\n");
		}
	}
}

/** Pin count is fixed inside the C++ model; JSON must not supply `pins`. */
bool componentUsesJsonPins(const std::string& type) {
	return type != "overhead_line"
		&& type != "mmc"
		&& type != "wt_type_3"
		&& type != "wt_type_4"
		&& type != "pv_plant"
		&& type != "wp_plant"
		&& type != "cable";
}

/** `location` on the component is optional (builder supplies a default). */
bool componentRequiresJsonLocation(const std::string& type) {
	return type != "overhead_line";
}

/** Keys allowed on any component in addition to type-specific fields. */
std::set<std::string> commonComponentKeys() {
	return {
		"id", "type", "location", "pins", "enabled",
		"connected_bus", "connected_buses", "local_parameters"
	};
}

JsonParameterTable mergedComponentParameters(
	const JsonParameterTable& rootParams,
	const JSON& comp,
	const char* componentId)
{
	JsonParameterTable params = rootParams;
	if (comp.contains("local_parameters")) {
		params.mergeFromObject(
			comp.at("local_parameters"),
			(std::string("local_parameters of component '") + componentId + "'").c_str());
	}
	return params;
}

void requireExclusiveValueSpec(
	const JSON& comp,
	const std::initializer_list<const char*> fields,
	const char* context)
{
	int count = 0;
	for (const char* field : fields) {
		if (comp.contains(field)) {
			++count;
		}
	}
	if (count == 0) {
		throw std::invalid_argument(
			std::string("ERROR: ") + context + " requires one of the value/expression fields.\n");
	}
	if (count > 1) {
		throw std::invalid_argument(
			std::string("ERROR: ") + context + " has multiple conflicting value/expression fields.\n");
	}
}

} // namespace


void JsonValidator::requireObject(const JSON& j, const char* context) {
	if (!j.is_object()) {
		throw std::invalid_argument(std::string("ERROR: expected object for ") + context + ".\n");
	}
}


void JsonValidator::rejectUnknownKeys(
	const JSON& j,
	const std::set<std::string>& allowed,
	const char* context)
{
	for (auto it = j.begin(); it != j.end(); ++it) {
		if (allowed.find(it.key()) == allowed.end()) {
			throw std::invalid_argument(
				std::string("ERROR: unknown key '") + it.key() + "' in " + context + ".\n");
		}
	}
}


void JsonValidator::validateRoot(const JSON& root) {
	requireObject(root, "root document");
	rejectUnknownKeys(root, {
		"simulation", "buses", "components", "computations", "parameters"
	}, "root document");

	JsonParameterTable rootParams;
	if (root.contains("parameters")) {
		JsonParameterTable::validateObject(root.at("parameters"), "parameters");
		rootParams.mergeFromObject(root.at("parameters"), "parameters");
	}

	validateSimulation(root.at("simulation"));
	if (!root.at("buses").is_array() || root.at("buses").empty()) {
		throw std::invalid_argument("ERROR: 'buses' must be a non-empty array.\n");
	}
	if (!root.at("components").is_array() || root.at("components").empty()) {
		throw std::invalid_argument("ERROR: 'components' must be a non-empty array.\n");
	}

	unsigned i = 0;
	for (const auto& bus : root.at("buses")) {
		validateBus(bus, i++);
	}

	i = 0;
	for (const auto& comp : root.at("components")) {
		validateComponent(comp, i++, rootParams);
	}

	if (root.contains("computations")) {
		if (!root.at("computations").is_array()) {
			throw std::invalid_argument("ERROR: 'computations' must be an array.\n");
		}
		i = 0;
		for (const auto& calc : root.at("computations")) {
			validateComputation(calc, i++);
		}
	}
}


void JsonValidator::validateSimulation(const JSON& sim) {
	requireObject(sim, "simulation");
	rejectUnknownKeys(sim, {
		"title", "description", "output_directory", "frequency_range",
		"nominal_power", "nominal_voltage", "dc_nominal_voltage", "omega"
	}, "simulation");
}


void JsonValidator::validateBus(const JSON& bus, const unsigned index) {
	requireObject(bus, ("bus[" + std::to_string(index) + "]").c_str());
	rejectUnknownKeys(bus, { "id", "location", "pins", "enabled" }, ("bus[" + std::to_string(index) + "]").c_str());
	ComponentBuilder::findNonEmptyString("id", bus);
	ComponentBuilder::findNumber("pins", bus);
}


void JsonValidator::validateByType(
	const JSON& comp,
	const std::string& type,
	const JsonParameterTable& params)
{
	const std::string ctx = "component '" + comp.at("id").get<std::string>() + "'";
	auto allow = [&](std::initializer_list<std::string> keys) {
		std::set<std::string> allowed = commonComponentKeys();
		for (const auto& key : keys) {
			allowed.insert(key);
		}
		return allowed;
	};

	if (type == "load" || type == "load_pq" || type == "capacitor" || type == "inductor"
		|| type == "resistor") {
		rejectUnknownKeys(comp, allow({ "values", "y_expr" }), ctx.c_str());
		requireExclusiveValueSpec(comp, { "values", "y_expr" }, ctx.c_str());
		if (comp.contains("values")) {
			params.validateNumericOrReferenceArray(comp.at("values"), "'values'");
		}
		if (comp.contains("y_expr")) {
			JsonExpression::validateExprField(comp.at("y_expr"), "'y_expr'");
			JsonExpression::parseAdmittance(comp.at("y_expr").get<std::string>(), params, "'y_expr'");
		}
		return;
	}

	if (type == "impedance") {
		rejectUnknownKeys(comp, allow({ "values", "complex", "z_expr" }), ctx.c_str());
		requireExclusiveValueSpec(comp, { "values", "complex", "z_expr" }, ctx.c_str());
		if (comp.contains("values")) {
			params.validateNumericOrReferenceArray(comp.at("values"), "'values'");
		}
		if (comp.contains("complex")) {
			params.validateNumericOrReferenceArray(comp.at("complex"), "'complex'", 2);
		}
		if (comp.contains("z_expr")) {
			JsonExpression::validateExprField(comp.at("z_expr"), "'z_expr'");
			JsonExpression::parseImpedance(comp.at("z_expr").get<std::string>(), params, "'z_expr'");
		}
		return;
	}

	if (type == "ac_source" || type == "generator") {
		rejectUnknownKeys(comp, allow({ "values", "voltage", "opf_info" }), ctx.c_str());
		params.validateNumericOrReferenceArray(comp.at("values"), "'values'");
		ComponentBuilder::findScalar("voltage", comp, params);
		return;
	}

	if (type == "dc_source") {
		rejectUnknownKeys(comp, allow({ "voltage", "resistance", "values" }), ctx.c_str());
		if (!comp.contains("voltage")) {
			throw std::invalid_argument("ERROR: dc_source requires 'voltage'.\n");
		}
		return;
	}

	if (type == "admittance") {
		rejectUnknownKeys(comp, allow({ "values", "y_expr", "y_exprs" }), ctx.c_str());
		requireExclusiveValueSpec(comp, { "values", "y_expr", "y_exprs" }, ctx.c_str());
		if (comp.contains("values")) {
			params.validateNumericOrReferenceArray(comp.at("values"), "'values'");
		}
		if (comp.contains("y_expr")) {
			JsonExpression::validateExprField(comp.at("y_expr"), "'y_expr'");
			JsonExpression::parseAdmittance(comp.at("y_expr").get<std::string>(), params, "'y_expr'");
		}
		if (comp.contains("y_exprs")) {
			JsonExpression::validateExprArray(comp.at("y_exprs"), params, "'y_exprs'");
		}
		return;
	}

	if (type == "switch") {
		rejectUnknownKeys(comp, allow({ "state", "closed" }), ctx.c_str());
		if (!comp.contains("state") && !comp.contains("closed")) {
			throw std::invalid_argument("ERROR: switch requires 'state' or 'closed'.\n");
		}
		return;
	}

	if (type == "transmission_line") {
		rejectUnknownKeys(comp, allow({ "values" }), ctx.c_str());
		params.validateNumericOrReferenceArray(comp.at("values"), "'values'", 5);
		return;
	}

	if (type.rfind("transformer_", 0) == 0) {
		rejectUnknownKeys(comp, allow({ "values" }), ctx.c_str());
		if (!comp.contains("values") || !comp.at("values").is_object()) {
			throw std::invalid_argument("ERROR: transformer requires object 'values'.\n");
		}
		return;
	}

	if (type == "cable") {
		rejectUnknownKeys(comp, allow({
			"cable_type", "length", "earth", "conductors", "insulators", "positions"
		}), ctx.c_str());
		requireKeys(comp, { "cable_type", "length", "earth", "conductors", "insulators", "positions" }, ctx.c_str());
		if (comp.contains("pins")) {
			ComponentBuilder::findScalar("pins", comp, params);
		}
		return;
	}

	if (type == "overhead_line") {
		rejectUnknownKeys(comp, allow({
			"length_km", "earth", "conductor", "groundwire"
		}), ctx.c_str());
		requireKeys(comp, { "length_km", "earth", "conductor", "groundwire" }, ctx.c_str());
		return;
	}

	if (type == "mmc") {
		rejectUnknownKeys(comp, allow({
			"converter_params", "controller_params", "filter_params"
		}), ctx.c_str());
		params.validateNumericOrReferenceArray(comp.at("converter_params"), "'converter_params'");
		if (comp.contains("controller_params")) {
			params.validateNumericOrReferenceArray(comp.at("controller_params"), "'controller_params'");
		}
		if (comp.contains("filter_params")) {
			params.validateNumericOrReferenceArray(comp.at("filter_params"), "'filter_params'");
		}
		return;
	}

	if (type == "wt_type_3" || type == "wt_type_4" || type == "pv_plant") {
		rejectUnknownKeys(comp, allow({ "parameters" }), ctx.c_str());
		params.validateNumericOrReferenceArray(comp.at("parameters"), "'parameters'");
		return;
	}

	if (type == "wp_plant") {
		rejectUnknownKeys(comp, allow({ "turbine_type", "number_wt", "parameters" }), ctx.c_str());
		ComponentBuilder::findScalar("turbine_type", comp, params);
		ComponentBuilder::findScalar("number_wt", comp, params);
		params.validateNumericOrReferenceArray(comp.at("parameters"), "'parameters'");
		return;
	}

	throw std::invalid_argument("ERROR: unknown component type '" + type + "'.\n");
}


void JsonValidator::validateComponent(
	const JSON& comp,
	const unsigned index,
	const JsonParameterTable& rootParams)
{
	requireObject(comp, ("component[" + std::to_string(index) + "]").c_str());
	ComponentBuilder::findNonEmptyString("type", comp);
	ComponentBuilder::findNonEmptyString("id", comp);

	const std::string type = lowerType(comp);
	const std::string id = comp.at("id").get<std::string>();
	const JsonParameterTable params = mergedComponentParameters(rootParams, comp, id.c_str());

	if (componentRequiresJsonLocation(type)) {
		ComponentBuilder::findNonEmptyString("location", comp);
	}
	if (componentUsesJsonPins(type)) {
		ComponentBuilder::findNumber("pins", comp);
	}

	validateByType(comp, type, params);
}


void JsonValidator::validateComputation(const JSON& calc, const unsigned index) {
	const std::string ctx = "computation[" + std::to_string(index) + "]";
	requireObject(calc, ctx.c_str());
	rejectUnknownKeys(calc, {
		"type", "case_name", "dc_case_name", "component_id", "converter_id", "location",
		"frequency_range", "vsc_control", "write_txt", "plot_result", "print_info",
		"dt", "t_start", "t_end", "frequency", "n_keep", "output_bus_ids",
		"switch_count", "switch_on_resistance", "switch_off_resistance", "switch_types",
		"plot", "plot_type"
	}, ctx.c_str());
	if (!calc.contains("type") || !calc.at("type").is_string()) {
		throw std::invalid_argument("ERROR: computation requires string 'type'.\n");
	}
	if (calc.contains("plot_type")) {
		if (!calc.at("plot_type").is_string()) {
			throw std::invalid_argument("ERROR: computation 'plot_type' must be a string.\n");
		}
		const std::string plotType = calc.at("plot_type").get<std::string>();
		std::string lower = plotType;
		std::transform(lower.begin(), lower.end(), lower.begin(),
			[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		if (lower != "bode" && lower != "nyquist") {
			throw std::invalid_argument(
				"ERROR: computation 'plot_type' must be 'bode' or 'nyquist'.\n");
		}
	}
}
