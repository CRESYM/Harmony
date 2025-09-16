#ifndef _STABILITY_ESTIMATE_H
#define _STABILITY_ESTIMATE_H

#include "../../Constants.h"
#include "../Helper_Functions/Symbolic_functions.h"

// Declarations
class Bus;
class Element;
class Network;

class StabilityEstimate {
public:
	StabilityEstimate() = default;

	~StabilityEstimate() = default;

	// Add AC and DC grids to the system
	void add_areas(Network* net);

	vector<string> get_ac_grids() { return ac_grids; };
	vector<string> get_dc_grids() { return dc_grids; };
	vector<Bus*> get_ac_buses(string area) { return ac_buses[area]; };
	vector<Bus*> get_dc_buses(string area) { return dc_buses[area]; };
	vector<Element*> get_ac_elements(string area) { return ac_elements[area]; };
	vector<Element*> get_dc_elements(string area) { return dc_elements[area]; };

	// Determine impedance of the part of the system
	void compute_equivalent_impedance(Network* net, std::vector<Bus*> start_buses, std::vector<Bus*> end_buses, std::vector<Element*> skip_elements);
	void compute_equivalent_impedance_num(Network* net, std::vector<Bus*> start_buses, std::vector<Bus*> end_buses, std::vector<Element*> skip_elements, double omega_num);
private:
	vector<string> ac_grids; // List of AC grids in the system
	vector<string> dc_grids; // List of DC grids in the system

	unordered_map<string, vector<Bus*>> ac_buses; // Map of AC grid names to their buses
	unordered_map<string, vector<Bus*>> dc_buses; // Map of DC grid names to their buses

	unordered_map<string, vector<Element*>> ac_elements; // Map of AC grid names to their elements
	unordered_map<string, vector<Element*>> dc_elements; // Map of DC grid names to their elements

	unordered_map<string, vector<Element*>> converters; // Map of AC grid names to their converter elements
	// add outputs to areas
};


#endif