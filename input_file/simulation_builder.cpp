#include "simulation_builder.h"

void SimulationBuilder::buildFromJSON(const JSON& sim, Network& network) {

	// Check all needed information is provided in the JSON file
    validateJSON(sim);
    
    try {
        // Build components
        int i = 0;
        for (const auto& comp : sim["components"]) {
            components.push_back(ComponentBuilder::buildFromJSON(comp, i));
            i++;
        }
    }
    catch (const std::invalid_argument& e) {
        std::cerr << e.what();
        throw std::runtime_error("ERROR: unable to initialize simulation.\n");
    }

}


void SimulationBuilder::validateJSON(const JSON& j) const {

    // Check for simulation, buses and components sections
    if (!j.contains("simulation") || !j["simulation"].is_object()) {
        throw invalid_argument("ERROR: missing or invalid 'simulation' section in JSON file.\n");
    }
    if (!j.contains("buses") || !j["buses"].is_array()) {
        throw invalid_argument("ERROR: missing or invalid 'buses' section in JSON file.\n");
    }
    if (!j.contains("components") || !j["components"].is_array()) {
        throw invalid_argument("ERROR: missing or invalid 'components' section in JSON file.\n");
    }

    // Check that "buses" has at least one item
    if (j["buses"].empty()) {
        throw invalid_argument("ERROR: 'buses' array is empty.\n");
    }
    
    // Check that "components" has at least one item
    if (j["components"].empty()) {
        throw invalid_argument("ERROR: 'components' array is empty.\n");
    }

}

void SimulationBuilder::buildBusFromJSON(const JSON& j) const {
    return;
}

void SimulationBuilder::connectElementsToBuses() {
    return;
}