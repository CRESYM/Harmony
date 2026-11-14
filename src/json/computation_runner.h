#ifndef HARMONY_JSON_COMPUTATION_RUNNER_H
#define HARMONY_JSON_COMPUTATION_RUNNER_H

#include "simulation_builder.h"

#include <functional>
#include <string>
#include <unordered_map>

/**
 * @file computation_runner.h
 * @brief Extensible registry for post-build JSON computations (OPF, DQsym, …).
 */

class ComputationRunner {
public:
	using Handler = std::function<void(const JSON&, Network&, const JSON& simulationConfig)>;

	explicit ComputationRunner(bool plottingEnabled = true);

	/** @return Number of computation steps that failed or used an unknown type. */
	int runAll(const JSON& sim, Network& network, const JSON& simulationConfig) const;

private:
	bool plottingEnabled_;
	std::unordered_map<std::string, Handler> handlers_;

	void registerBuiltins();
	static std::string normalizeType(const JSON& calc);
};

#endif // HARMONY_JSON_COMPUTATION_RUNNER_H