/**
 * @file simulation_builder.cpp
 * @brief Implementation of JSON simulation orchestration (SimulationBuilder).
 */
#include "simulation_builder.h"

#include "computation_runner.h"
#include "json_validator.h"
#include "json_parameters.h"

#include "../Bus.h"

#include <set>


void SimulationBuilder::buildFromJSON(const JSON& sim, Network& network) {
	JsonValidator::validateRoot(sim);
	simulationConfig_ = sim["simulation"];

	JsonParameterTable rootParams;
	if (sim.contains("parameters")) {
		rootParams.mergeFromObject(sim.at("parameters"), "parameters");
	}

	buildBusesFromJSON(sim, network);

	unsigned int i = 0;
	for (const auto& comp : sim["components"]) {
		if (comp.value("enabled", true) == false) {
			++i;
			continue;
		}

		JsonParameterTable compParams = rootParams;
		if (comp.contains("local_parameters")) {
			compParams.mergeFromObject(
				comp.at("local_parameters"),
				("local_parameters of component '" + comp.at("id").get<std::string>() + "'").c_str());
		}

		auto elem = ComponentBuilder::buildFromJSON(comp, i, compParams);
		network.addElement(comp["id"].get<std::string>(), std::move(elem));
		++i;
	}

	connectElementsToBuses(sim, network);
}


void SimulationBuilder::validateJSON(const JSON& j) const {
	JsonValidator::validateRoot(j);
}


void SimulationBuilder::buildBusesFromJSON(const JSON& sim, Network& network) const {
	for (const auto& busJson : sim["buses"]) {
		if (busJson.value("enabled", true) == false) {
			continue;
		}

		const std::string id = busJson["id"];
		const int pinCount = busJson["pins"];
		const std::string location = busJson.value("location", "AC1");

		network.addBus(id, std::make_unique<Bus>(id, location, pinCount));
	}
}


void SimulationBuilder::connectComponent(const JSON& comp, Element* elem, Network& network) {
	auto connectOne = [&](const JSON& conn) {
		if (!conn.contains("bus_id") || !conn["bus_id"].is_string()) {
			throw std::invalid_argument(
				"ERROR: connection for component '" + comp["id"].get<std::string>() + "' missing 'bus_id'.\n");
		}

		const std::string busId = conn["bus_id"];
		const int terminal = conn.value("terminal", 1);

		const auto& buses = network.getBuses();
		auto busIt = buses.find(busId);
		if (busIt == buses.end()) {
			throw std::invalid_argument("ERROR: bus '" + busId + "' not found for component '" +
				comp["id"].get<std::string>() + "'.\n");
		}

		network.connectElementToBus(elem, terminal, busIt->second);
	};

	if (comp.contains("connected_buses") && comp["connected_buses"].is_array()) {
		for (const auto& conn : comp["connected_buses"]) {
			connectOne(conn);
		}
	}
	else if (comp.contains("connected_bus") && comp["connected_bus"].is_object()) {
		connectOne(comp["connected_bus"]);
	}
}


void SimulationBuilder::connectElementsToBuses(const JSON& sim, Network& network) const {
	for (const auto& comp : sim["components"]) {
		if (comp.value("enabled", true) == false) {
			continue;
		}

		const std::string id = comp["id"];
		auto elemIt = network.getElements().find(id);
		if (elemIt == network.getElements().end()) {
			throw std::runtime_error("ERROR: component '" + id + "' was not added to the network.\n");
		}

		connectComponent(comp, elemIt->second, network);
	}
}


FrequencyRange SimulationBuilder::parseFrequencyRange(const JSON& rangeJson) {
	FrequencyRange range;
	range.start = rangeJson.value("start", range.start);
	range.end = rangeJson.value("end", range.end);

	if (rangeJson.contains("step") && rangeJson["step"].is_number()) {
		const double step = rangeJson["step"].get<double>();
		if (step > 0.0 && range.end >= range.start) {
			range.points = static_cast<int>((range.end - range.start) / step) + 1;
		}
	}
	else if (rangeJson.contains("points") && rangeJson["points"].is_number_integer()) {
		range.points = rangeJson["points"].get<int>();
	}

	range.points = std::max(range.points, 2);
	return range;
}


void SimulationBuilder::runComputations(
	const JSON& sim,
	Network& network,
	const bool plottingEnabled) const
{
	ComputationRunner runner(plottingEnabled);
	runner.runAll(sim, network, simulationConfig_);
}
