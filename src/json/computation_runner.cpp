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

bool jsonPlotRequested(const JSON& calc, const bool plottingEnabled) {
	return plottingEnabled && calc.value("plot", false);
}

bool jsonPlotResultRequested(const JSON& calc, const bool plottingEnabled) {
	return plottingEnabled && calc.value("plot_result", false);
}

std::string lowerCopy(std::string value) {
	std::transform(value.begin(), value.end(), value.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return value;
}

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

void runYMatrix(const JSON& calc, Network& network, const JSON& defaultRange, const bool plottingEnabled) {
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
		if (jsonPlotRequested(calc, plottingEnabled)) {
			elem->plotYParameters(range.start, range.end, range.points);
		}
	}
}

void runStabilityPlots(
	StabilityEstimate& stability,
	const JSON& calc,
	const FrequencyRange& range,
	const bool plottingEnabled)
{
	if (!jsonPlotRequested(calc, plottingEnabled)) {
		return;
	}
	if (!calc.contains("converter_id") || !calc.contains("location")) {
		throw std::invalid_argument(
			"stability_assessment 'plot' requires 'converter_id' and 'location'.\n");
	}

	const std::string converterId = calc.at("converter_id").get<std::string>();
	const std::string location = calc.at("location").get<std::string>();
	const std::string plotType = lowerCopy(calc.value("plot_type", std::string("bode")));

	if (plotType == "bode") {
		stability.bodeplotTF(converterId, location, range.start, range.end, range.points);
	}
	else if (plotType == "nyquist") {
		stability.nyquistplotTF(converterId, location, range.start, range.end, range.points);
	}
	else {
		throw std::invalid_argument(
			"stability_assessment 'plot_type' must be 'bode' or 'nyquist', got '" + plotType + "'.\n");
	}
}

void runOpf(const JSON& calc, Network& network, const JSON& simulationConfig, const bool plottingEnabled) {
	PowerFlow pf;
	if (calc.contains("case_name")) {
		const std::string acCase = calc.at("case_name").get<std::string>();
		const std::string dcCase = calc.value("dc_case_name", acCase);
		pf.solve_opf(
			dcCase,
			acCase,
			nullptr,
			calc.value("vsc_control", true),
			calc.value("write_txt", false),
			jsonPlotResultRequested(calc, plottingEnabled),
			calc.value("print_info", false));
		return;
	}

	std::map<std::string, double> globalParams;
	globalParams["baseMVA"] = simulationConfig.value("nominal_power", 100.0);
	globalParams["ACbaseKV"] = simulationConfig.value("nominal_voltage", 345.0);
	globalParams["DCbaseKV"] = simulationConfig.value("dc_nominal_voltage", 400.0);
	globalParams["omega"] = simulationConfig.value("omega", 2.0 * M_PI * 50.0);
	globalParams["ACZbase"] =
		globalParams["ACbaseKV"] * globalParams["ACbaseKV"] / globalParams["baseMVA"];
	globalParams["DCZbase"] =
		globalParams["DCbaseKV"] * globalParams["DCbaseKV"] / globalParams["baseMVA"];

	pf.make_OPF(&network, globalParams,
		calc.value("vsc_control", true),
		calc.value("write_txt", false),
		jsonPlotResultRequested(calc, plottingEnabled),
		calc.value("print_info", true));
}

void runDqsym(const JSON& calc, Network& network, const bool plottingEnabled) {
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

	dq.run(cfg);
	if (jsonPlotRequested(calc, plottingEnabled)) {
		dq.plot();
	}
}

} // namespace


ComputationRunner::ComputationRunner(const bool plottingEnabled)
	: plottingEnabled_(plottingEnabled) {
	registerBuiltins();
}


std::string ComputationRunner::normalizeType(const JSON& calc) {
	std::string type = calc.at("type").get<std::string>();
	std::transform(type.begin(), type.end(), type.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return type;
}


void ComputationRunner::registerBuiltins() {
	handlers_["y_matrix"] = [this](const JSON& calc, Network& net, const JSON& simCfg) {
		runYMatrix(calc, net, simCfg.value("frequency_range", JSON::object()), plottingEnabled_);
	};
	handlers_["y_matrx"] = handlers_["y_matrix"];

	handlers_["network_summary"] = [](const JSON&, Network& net, const JSON&) {
		net.print_summary();
	};

	handlers_["print_connections"] = [](const JSON&, Network& net, const JSON&) {
		net.printConnections();
	};

	handlers_["stability_assessment"] = [this](const JSON& calc, Network& net, const JSON& simCfg) {
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
			runStabilityPlots(stability, calc, range, plottingEnabled_);
		}
	};
	handlers_["stability_assesment"] = handlers_["stability_assessment"];

	handlers_["power_flow"] = [this](const JSON& calc, Network& net, const JSON& simCfg) {
		runOpf(calc, net, simCfg, plottingEnabled_);
	};
	handlers_["opf"] = handlers_["power_flow"];

	handlers_["dqsym"] = [this](const JSON& calc, Network& net, const JSON&) {
		runDqsym(calc, net, plottingEnabled_);
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
