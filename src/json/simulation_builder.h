#pragma once
#include "component_builder.h"

/**
 * @file simulation_builder.h
 * @brief Builds a @ref Network from a JSON simulation file and runs configured analyses.
 * @ingroup input
 */

/** @brief Frequency sweep specification parsed from JSON. */
struct FrequencyRange {
	double start = 1.0;  ///< Start frequency (Hz).
	double end = 1000.0; ///< End frequency (Hz).
	int points = 100;    ///< Number of logarithmic sweep points.
};

/**
 * @brief Orchestrates JSON validation, network assembly, and post-build computations.
 */
class SimulationBuilder {

public:

	SimulationBuilder() = default;
	~SimulationBuilder() = default;

	/**
	 * @brief Populate @p network from a full simulation JSON document.
	 * @param simJSON Root JSON object (`simulation`, `buses`, `components`, …).
	 * @param network Target network to populate.
	 */
	void buildFromJSON(const JSON& simJSON, Network& network);

	/**
	 * @brief Run analyses selected in the simulation JSON (OPF, stability, etc.).
	 * @param simJSON Root JSON object (must contain a `simulation` section).
	 * @param network Assembled network to analyse.
	 */
	void runComputations(const JSON& simJSON, Network& network) const;

	/** @brief Return the parsed `simulation` configuration object. */
	const JSON& getSimulationConfig() const { return simulationConfig_; }

private:

	/** @brief Validate required top-level JSON sections and unique IDs. @param simJSON Root JSON object. */
	void validateJSON(const JSON& simJSON) const;

	/**
	 * @brief Create buses from the `buses` array and add them to @p network.
	 * @param simJSON Root JSON object.
	 * @param network Target network.
	 */
	void buildBusesFromJSON(const JSON& simJSON, Network& network) const;

	/**
	 * @brief Wire components to buses using connection metadata in JSON.
	 * @param simJSON Root JSON object.
	 * @param network Network with elements already registered.
	 */
	void connectElementsToBuses(const JSON& simJSON, Network& network) const;

	/** @brief Parse a frequency-range object from JSON. @param rangeJson JSON sub-object. */
	static FrequencyRange parseFrequencyRange(const JSON& rangeJson);

	/**
	 * @brief Attach one element to the buses named in its JSON entry.
	 * @param comp Component JSON object.
	 * @param elem Built element pointer.
	 * @param network Target network.
	 */
	static void connectComponent(const JSON& comp, Element* elem, Network& network);

	JSON simulationConfig_;
};
