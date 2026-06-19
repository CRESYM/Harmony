/**
 * @file json_validator.cpp
 * @brief Strict per-component JSON schema validation.
 */
#include "json_validator.h"

#include "component_builder.h"

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
	rejectUnknownKeys(root, { "simulation", "buses", "components", "computations" }, "root document");
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
		validateComponent(comp, i++);
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
		"nominal_power", "nominal_voltage", "dc_nominal_voltage"
	}, "simulation");
}


void JsonValidator::validateBus(const JSON& bus, const unsigned index) {
	requireObject(bus, ("bus[" + std::to_string(index) + "]").c_str());
	rejectUnknownKeys(bus, { "id", "location", "pins", "enabled" }, ("bus[" + std::to_string(index) + "]").c_str());
	ComponentBuilder::findNonEmptyString("id", bus);
	ComponentBuilder::findNumber("pins", bus);
}


void JsonValidator::validateByType(const JSON& comp, const std::string& type) {
	const std::string ctx = "component '" + comp.at("id").get<std::string>() + "'";

	if (type == "load" || type == "load_pq" || type == "capacitor" || type == "inductor"
		|| type == "resistor" || type == "impedance") {
		rejectUnknownKeys(comp, {
			"id", "type", "location", "pins", "values", "enabled",
			"connected_bus", "connected_buses"
		}, ctx.c_str());
		ComponentBuilder::findNonEmptyNumericArray("values", comp);
		return;
	}

	if (type == "ac_source" || type == "generator") {
		rejectUnknownKeys(comp, {
			"id", "type", "location", "pins", "values", "voltage", "enabled",
			"connected_bus", "connected_buses"
		}, ctx.c_str());
		ComponentBuilder::findNonEmptyNumericArray("values", comp);
		ComponentBuilder::findNumber("voltage", comp);
		return;
	}

	if (type == "dc_source") {
		rejectUnknownKeys(comp, {
			"id", "type", "location", "pins", "voltage", "resistance", "values", "enabled",
			"connected_bus", "connected_buses"
		}, ctx.c_str());
		if (!comp.contains("voltage")) {
			throw std::invalid_argument("ERROR: dc_source requires 'voltage'.\n");
		}
		return;
	}

	if (type == "admittance") {
		rejectUnknownKeys(comp, {
			"id", "type", "location", "pins", "values", "enabled",
			"connected_bus", "connected_buses"
		}, ctx.c_str());
		ComponentBuilder::findNonEmptyNumericArray("values", comp);
		return;
	}

	if (type == "switch") {
		rejectUnknownKeys(comp, {
			"id", "type", "location", "pins", "state", "closed", "enabled",
			"connected_bus", "connected_buses"
		}, ctx.c_str());
		if (!comp.contains("state") && !comp.contains("closed")) {
			throw std::invalid_argument("ERROR: switch requires 'state' or 'closed'.\n");
		}
		return;
	}

	if (type == "transmission_line") {
		rejectUnknownKeys(comp, {
			"id", "type", "location", "pins", "values", "enabled",
			"connected_bus", "connected_buses"
		}, ctx.c_str());
		ComponentBuilder::findNonEmptyNumericArray("values", comp, 5);
		return;
	}

	if (type.rfind("transformer_", 0) == 0) {
		rejectUnknownKeys(comp, {
			"id", "type", "location", "pins", "values", "enabled",
			"connected_bus", "connected_buses"
		}, ctx.c_str());
		if (!comp.contains("values") || !comp.at("values").is_object()) {
			throw std::invalid_argument("ERROR: transformer requires object 'values'.\n");
		}
		return;
	}

	if (type == "cable") {
		rejectUnknownKeys(comp, {
			"id", "type", "location", "pins", "cable_type", "length", "earth",
			"conductors", "insulators", "positions", "enabled",
			"connected_bus", "connected_buses"
		}, ctx.c_str());
		requireKeys(comp, { "cable_type", "length", "earth", "conductors", "insulators", "positions" }, ctx.c_str());
		return;
	}

	if (type == "overhead_line") {
		rejectUnknownKeys(comp, {
			"id", "type", "location", "length_km", "earth", "conductor", "groundwire", "enabled",
			"connected_bus", "connected_buses"
		}, ctx.c_str());
		requireKeys(comp, { "length_km", "earth", "conductor", "groundwire" }, ctx.c_str());
		return;
	}

	if (type == "mmc") {
		rejectUnknownKeys(comp, {
			"id", "type", "location", "converter_params", "controller_params", "filter_params", "enabled",
			"connected_bus", "connected_buses"
		}, ctx.c_str());
		ComponentBuilder::findNonEmptyNumericArray("converter_params", comp);
		return;
	}

	if (type == "wt_type_3" || type == "wt_type_4" || type == "pv_plant") {
		rejectUnknownKeys(comp, {
			"id", "type", "location", "parameters", "enabled",
			"connected_bus", "connected_buses"
		}, ctx.c_str());
		ComponentBuilder::findNonEmptyNumericArray("parameters", comp);
		return;
	}

	if (type == "wp_plant") {
		rejectUnknownKeys(comp, {
			"id", "type", "location", "turbine_type", "number_wt", "parameters", "enabled",
			"connected_bus", "connected_buses"
		}, ctx.c_str());
		ComponentBuilder::findNumber("turbine_type", comp);
		ComponentBuilder::findNumber("number_wt", comp);
		ComponentBuilder::findNonEmptyNumericArray("parameters", comp);
		return;
	}

	throw std::invalid_argument("ERROR: unknown component type '" + type + "'.\n");
}


void JsonValidator::validateComponent(const JSON& comp, const unsigned index) {
	requireObject(comp, ("component[" + std::to_string(index) + "]").c_str());
	ComponentBuilder::findNonEmptyString("type", comp);
	ComponentBuilder::findNonEmptyString("id", comp);

	const std::string type = lowerType(comp);
	if (type != "overhead_line") {
		ComponentBuilder::findNonEmptyString("location", comp);
		ComponentBuilder::findNumber("pins", comp);
	}

	validateByType(comp, type);
}


void JsonValidator::validateComputation(const JSON& calc, const unsigned index) {
	const std::string ctx = "computation[" + std::to_string(index) + "]";
	requireObject(calc, ctx.c_str());
	rejectUnknownKeys(calc, {
		"type", "case_name", "component_id", "converter_id", "location",
		"frequency_range", "vsc_control", "write_txt", "plot_result", "print_info",
		"dt", "t_start", "t_end", "frequency", "n_keep", "output_bus_ids",
		"switch_on_resistance", "switch_off_resistance", "switch_types", "plot"
	}, ctx.c_str());
	if (!calc.contains("type") || !calc.at("type").is_string()) {
		throw std::invalid_argument("ERROR: computation requires string 'type'.\n");
	}
}
