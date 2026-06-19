#pragma once
#include "component_builder.h"

struct FrequencyRange {
	double start = 1.0;
	double end = 1000.0;
	int points = 100;
};

class SimulationBuilder {

public:

	SimulationBuilder() = default;
	~SimulationBuilder() = default;

	void buildFromJSON(const JSON& simJSON, Network& network);
	void runComputations(const JSON& simJSON, Network& network) const;

	const JSON& getSimulationConfig() const { return simulationConfig_; }

private:

	void validateJSON(const JSON& simJSON) const;
	void buildBusesFromJSON(const JSON& simJSON, Network& network) const;
	void connectElementsToBuses(const JSON& simJSON, Network& network) const;

	static FrequencyRange parseFrequencyRange(const JSON& rangeJson);
	static void connectComponent(const JSON& comp, Element* elem, Network& network);

	JSON simulationConfig_;
};
