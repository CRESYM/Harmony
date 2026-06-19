/**
 * @file simulation_builder.cpp
 * @brief Implementation of JSON simulation orchestration (SimulationBuilder).
 */
#include "simulation_builder.h"

#include "../src/Solver/Stability_Estimate/Stability_estimate.h"
#include "../src/Solver/OPF/Powerflow.h"

#include <cctype>
#include <filesystem>
#include <set>


void SimulationBuilder::buildFromJSON(const JSON& sim, Network& network) {
	validateJSON(sim);
	simulationConfig_ = sim["simulation"];

	buildBusesFromJSON(sim, network);

	unsigned int i = 0;
	for (const auto& comp : sim["components"]) {
		if (comp.value("enabled", true) == false) {
			++i;
			continue;
		}

		Element* elem = ComponentBuilder::buildFromJSON(comp, i);
		network.addElement(comp["id"].get<std::string>(), elem);
		++i;
	}

	connectElementsToBuses(sim, network);
}


void SimulationBuilder::validateJSON(const JSON& j) const {
	if (!j.contains("simulation") || !j["simulation"].is_object()) {
		throw std::invalid_argument("ERROR: missing or invalid 'simulation' section in JSON file.\n");
	}
	if (!j.contains("buses") || !j["buses"].is_array()) {
		throw std::invalid_argument("ERROR: missing or invalid 'buses' section in JSON file.\n");
	}
	if (!j.contains("components") || !j["components"].is_array()) {
		throw std::invalid_argument("ERROR: missing or invalid 'components' section in JSON file.\n");
	}
	if (j["buses"].empty()) {
		throw std::invalid_argument("ERROR: 'buses' array is empty.\n");
	}
	if (j["components"].empty()) {
		throw std::invalid_argument("ERROR: 'components' array is empty.\n");
	}

	std::set<std::string> busIds;
	for (const auto& bus : j["buses"]) {
		ComponentBuilder::findNonEmptyString("id", bus);
		ComponentBuilder::findNumber("pins", bus);
		const std::string id = bus["id"];
		if (!busIds.insert(id).second) {
			throw std::invalid_argument("ERROR: duplicate bus id '" + id + "'.\n");
		}
	}

	std::set<std::string> componentIds;
	for (const auto& comp : j["components"]) {
		ComponentBuilder::findNonEmptyString("id", comp);
		const std::string id = comp["id"];
		if (!componentIds.insert(id).second) {
			throw std::invalid_argument("ERROR: duplicate component id '" + id + "'.\n");
		}
	}
}


void SimulationBuilder::buildBusesFromJSON(const JSON& sim, Network& network) const {
	for (const auto& busJson : sim["buses"]) {
		if (busJson.value("enabled", true) == false) {
			continue;
		}

		const std::string id = busJson["id"];
		const int pins = busJson["pins"];
		const std::string location = busJson.value("location", "AC1");

		auto* bus = new Bus(id, location, pins);
		network.addBus(id, bus);
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


void SimulationBuilder::runComputations(const JSON& sim, Network& network) const {
	if (!sim.contains("computations") || !sim["computations"].is_array()) {
		return;
	}

	const JSON defaultRange = simulationConfig_.value("frequency_range", JSON::object());
	const std::string outputDir = simulationConfig_.value("output_directory", "./files");

	if (!outputDir.empty()) {
		std::filesystem::create_directories(outputDir);
	}

	for (const auto& calc : sim["computations"]) {
		if (!calc.contains("type") || !calc["type"].is_string()) {
			std::cerr << "[WARN] Skipping computation without 'type'.\n";
			continue;
		}

		std::string type = calc["type"];
		std::transform(type.begin(), type.end(), type.begin(),
			[](unsigned char c) { return static_cast<char>(std::tolower(c)); });

		if (type == "y_matrix" || type == "y_matrx") {
			const JSON rangeJson = calc.value("frequency_range", defaultRange);
			const FrequencyRange range = parseFrequencyRange(rangeJson);

			for (const auto& [id, elem] : network.getElements()) {
				if (calc.contains("component_id") && calc["component_id"] != id) {
					continue;
				}
				std::cout << "Computing Y-matrix for '" << id << "' ("
					<< range.start << "–" << range.end << " Hz, "
					<< range.points << " points)\n";
				elem->writeFile(range.start, range.end, range.points);
			}
		}
		else if (type == "network_summary") {
			network.print_summary();
		}
		else if (type == "print_connections") {
			network.printConnections();
		}
		else if (type == "stability_assessment" || type == "stability_assesment") {
			network.add_areas();
			StabilityEstimate stability;
			stability.add_areas(&network);
			stability.print_summary();

			if (calc.contains("converter_id") && calc.contains("location")) {
				const std::string converterId = calc["converter_id"];
				const std::string location = calc["location"];
				const JSON rangeJson = calc.value("frequency_range", defaultRange);
				const FrequencyRange range = parseFrequencyRange(rangeJson);
				stability.writeFileTF(converterId, location, range.start, range.end, range.points);
			}
		}
		else if (type == "power_flow" || type == "opf") {
			if (!calc.contains("case_name")) {
				std::cerr << "[WARN] 'power_flow' requires 'case_name' (CSV case prefix under src/data/). "
					<< "Skipping.\n";
				continue;
			}
			try {
				std::map<std::string, double> globalParams;
				globalParams["baseMVA"] = simulationConfig_.value("nominal_power", 100.0);
				globalParams["ACbaseKV"] = simulationConfig_.value("nominal_voltage", 345.0);
				globalParams["DCbaseKV"] = simulationConfig_.value("dc_nominal_voltage", 400.0);

				PowerFlow pf;
				const std::string caseName = calc["case_name"];
				auto dataAc = pf.create_ac(caseName);
				auto dataDc = pf.create_dc(caseName);
				pf.load_params_ac("AC1", dataAc);
				pf.load_params_dc("DC1", dataDc);
				pf.make_OPF(&network, globalParams,
					calc.value("vsc_control", true),
					calc.value("write_txt", false),
					calc.value("plot_result", false),
					calc.value("print_info", true));
			}
			catch (const std::exception& e) {
				std::cerr << "[WARN] power_flow failed: " << e.what() << "\n";
			}
		}
		else if (type == "equivalent_impedance" || type == "equivalent_impedance_num") {
			std::cerr << "[WARN] Computation '" << type
				<< "' requires manual bus/element selection; use the C++ API or extend the JSON schema.\n";
		}
		else if (type == "time_domain" || type == "dqsym") {
			std::cerr << "[WARN] Computation '" << type
				<< "' is not yet exposed through the JSON runner; use the DQsym examples.\n";
		}
		else {
			std::cerr << "[WARN] Unknown computation type '" << type << "'.\n";
		}
	}
}
