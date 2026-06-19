/**
 * @file computation_runner.cpp
 * @brief Extensible JSON computation dispatch (OPF, DQsym, …).
 */
#include "computation_runner.h"

#include "../Solver/DQsym/DQsym.h"
#include "../Solver/Stability_Estimate/Stability_estimate.h"
#include "../Solver/OPF/Powerflow.h"

#include <cctype>
#include <filesystem>


namespace {

FrequencyRange parseFrequencyRangeLocal(const JSON& rangeJson) {
	FrequencyRange range;
	range.start = rangeJson.value("start", range.start);
	range.end = rangeJson.value("end", range.end);
	if (rangeJson.contains("step") && rangeJson.at("step").is_number()) {
		const double step = rangeJson.at("step").get<double>();
		if (step > 0.0 && range.end >= range.start) {
			range.points = static_cast<int>((range.end - range.start) / step) + 1;
		}
	}
	else if (rangeJson.contains("points") && rangeJson.at("points").is_number_integer()) {
		range.points = rangeJson.at("points").get<int>();
	}
	range.points = std::max(range.points, 2);
	return range;
}

void runYMatrix(const JSON& calc, Network& network, const JSON& defaultRange) {
	const JSON rangeJson = calc.value("frequency_range", defaultRange);
	const FrequencyRange range = parseFrequencyRangeLocal(rangeJson);
	for (const auto& [id, elem] : network.getElements()) {
		if (calc.contains("component_id") && calc.at("component_id") != id) {
			continue;
		}
		std::cout << "Computing Y-matrix for '" << id << "' ("
			<< range.start << "–" << range.end << " Hz, "
			<< range.points << " points)\n";
		elem->writeFile(range.start, range.end, range.points);
	}
}

void runOpf(const JSON& calc, Network& network, const JSON& simulationConfig) {
	if (!calc.contains("case_name")) {
		throw std::invalid_argument("'opf' / 'power_flow' requires 'case_name'.\n");
	}

	std::map<std::string, double> globalParams;
	globalParams["baseMVA"] = simulationConfig.value("nominal_power", 100.0);
	globalParams["ACbaseKV"] = simulationConfig.value("nominal_voltage", 345.0);
	globalParams["DCbaseKV"] = simulationConfig.value("dc_nominal_voltage", 400.0);

	PowerFlow pf;
	const std::string caseName = calc.at("case_name");
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

void runDqsym(const JSON& calc, Network& network) {
	DQsym dq;
	dq.initialize(&network);

	Config cfg;
	cfg.dt = calc.value("dt", 2e-5);
	cfg.t_start = calc.value("t_start", 0.0);
	cfg.t_end = calc.value("t_end", 0.1);
	cfg.f = calc.value("frequency", 50.0);
	cfg.omega = 2.0 * M_PI * cfg.f;
	cfg.nKeep = calc.value("n_keep", 5);

	const int swCount = calc.value("switch_count", 1);
	cfg.swOnRes = Eigen::VectorXd::Constant(swCount, 0.01);
	cfg.swOffRes = Eigen::VectorXd::Constant(swCount, 1e6);
	cfg.swType = Eigen::VectorXi::Zero(swCount);

	if (calc.contains("switch_on_resistance") && calc.at("switch_on_resistance").is_array()) {
		const auto& arr = calc.at("switch_on_resistance");
		cfg.swOnRes.resize(arr.size());
		for (size_t i = 0; i < arr.size(); ++i) {
			cfg.swOnRes(static_cast<Eigen::Index>(i)) = arr.at(i).get<double>();
		}
	}
	if (calc.contains("switch_off_resistance") && calc.at("switch_off_resistance").is_array()) {
		const auto& arr = calc.at("switch_off_resistance");
		cfg.swOffRes.resize(arr.size());
		for (size_t i = 0; i < arr.size(); ++i) {
			cfg.swOffRes(static_cast<Eigen::Index>(i)) = arr.at(i).get<double>();
		}
	}
	if (calc.contains("switch_types") && calc.at("switch_types").is_array()) {
		const auto& arr = calc.at("switch_types");
		cfg.swType.resize(arr.size());
		for (size_t i = 0; i < arr.size(); ++i) {
			cfg.swType(static_cast<Eigen::Index>(i)) = arr.at(i).get<int>();
		}
	}

	cfg.breakerFunction = nullptr;
	cfg.outputBuses.clear();
	if (calc.contains("output_bus_ids") && calc.at("output_bus_ids").is_array()) {
		for (const auto& busId : calc.at("output_bus_ids")) {
			const std::string id = busId.get<std::string>();
			auto it = network.getBuses().find(id);
			if (it == network.getBuses().end()) {
				throw std::invalid_argument("DQsym output bus '" + id + "' not found.\n");
			}
			cfg.outputBuses.push_back(it->second);
		}
	}
	else {
		for (const auto& [id, bus] : network.getBuses()) {
			if (id != "gnd") {
				cfg.outputBuses.push_back(bus);
				break;
			}
		}
	}

	const bool plot = calc.value("plot", false);
	(void)plot;
	dq.run(cfg);
}

} // namespace


ComputationRunner::ComputationRunner() {
	registerBuiltins();
}


std::string ComputationRunner::normalizeType(const JSON& calc) {
	std::string type = calc.at("type").get<std::string>();
	std::transform(type.begin(), type.end(), type.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return type;
}


void ComputationRunner::registerBuiltins() {
	handlers_["y_matrix"] = [](const JSON& calc, Network& net, const JSON& simCfg) {
		runYMatrix(calc, net, simCfg.value("frequency_range", JSON::object()));
	};
	handlers_["y_matrx"] = handlers_["y_matrix"];

	handlers_["network_summary"] = [](const JSON&, Network& net, const JSON&) {
		net.print_summary();
	};

	handlers_["print_connections"] = [](const JSON&, Network& net, const JSON&) {
		net.printConnections();
	};

	handlers_["stability_assessment"] = [](const JSON& calc, Network& net, const JSON& simCfg) {
		net.add_areas();
		StabilityEstimate stability;
		stability.add_areas(&net);
		stability.print_summary();
		if (calc.contains("converter_id") && calc.contains("location")) {
			const FrequencyRange range = parseFrequencyRangeLocal(
				calc.value("frequency_range", simCfg.value("frequency_range", JSON::object())));
			stability.writeFileTF(
				calc.at("converter_id").get<std::string>(),
				calc.at("location").get<std::string>(),
				range.start, range.end, range.points);
		}
	};
	handlers_["stability_assesment"] = handlers_["stability_assessment"];

	handlers_["power_flow"] = [](const JSON& calc, Network& net, const JSON& simCfg) {
		runOpf(calc, net, simCfg);
	};
	handlers_["opf"] = handlers_["power_flow"];

	handlers_["dqsym"] = [](const JSON& calc, Network& net, const JSON&) {
		runDqsym(calc, net);
	};
	handlers_["time_domain"] = handlers_["dqsym"];
}


void ComputationRunner::runAll(
	const JSON& sim,
	Network& network,
	const JSON& simulationConfig) const
{
	if (!sim.contains("computations") || !sim.at("computations").is_array()) {
		return;
	}

	const std::string outputDir = simulationConfig.value("output_directory", "./files");
	if (!outputDir.empty()) {
		std::filesystem::create_directories(outputDir);
	}

	for (const auto& calc : sim.at("computations")) {
		if (!calc.contains("type")) {
			std::cerr << "[WARN] Skipping computation without 'type'.\n";
			continue;
		}

		const std::string type = normalizeType(calc);
		const auto it = handlers_.find(type);
		if (it == handlers_.end()) {
			std::cerr << "[WARN] Unknown computation type '" << type
				<< "'. Register handlers in ComputationRunner.\n";
			continue;
		}

		try {
			it->second(calc, network, simulationConfig);
		}
		catch (const std::exception& e) {
			std::cerr << "[WARN] Computation '" << type << "' failed: " << e.what() << "\n";
		}
	}
}
