#pragma once
#include "component_builder.h"

// struct SimulationParameters

class SimulationBuilder {

public:

	SimulationBuilder() {};
	~SimulationBuilder() {};
	
	void buildFromJSON(const JSON& simJSON, Network& network);

	std::vector<ComponentType> getComponents() const {
		return components;
	}

	std::vector<Bus> getBuses() const {
		return buses;
	}

private:

	void validateJSON(const JSON& simJSON) const;

	void buildBusFromJSON(const JSON& simJSON) const;

	void connectElementsToBuses();

	std::vector<ComponentType> components;
	std::vector<Bus> buses;
	//std::vector<Computations> computations;

	//SimulationParameters parameters;
	

};