#ifndef _STABILITY_ESTIMATE_H
#define _STABILITY_ESTIMATE_H

#include "../../Constants.h"
#include "../Helper_Functions/Symbolic_functions.h"
#include "../../SubNetwork.h"   // Include your SubNetwork class

// Declarations
class Bus;
class Element;
class Network;
class SubNetwork;

class StabilityEstimate {
public:
	StabilityEstimate() = default;

	~StabilityEstimate() = default;

	// Add AC and DC grids to the system
	void add_areas(Network* net);

	vector<string> get_ac_grid_names() { return ac_grid_names; };
	vector<string> get_dc_grid_names() { return dc_grid_names; };
	std::unordered_map<std::string, std::unique_ptr<SubNetwork>>& get_ac_grids() { return ac_grids; }
	std::unordered_map<std::string, std::unique_ptr<SubNetwork>>& get_dc_grids() { return dc_grids; }
	std::unordered_map<std::string, Element*>& get_converters() { return converters; }

	// Determine closing impedance of the part of the system
	void compute_equivalent_impedance(Network* net, std::vector<Bus*> start_buses, std::vector<Bus*> end_buses, std::vector<Element*> skip_elements);
	MatrixXcd compute_equivalent_impedance_num(Network* net, std::vector<Bus*> start_buses, std::vector<Bus*> end_buses, std::vector<Element*> skip_elements, double frequency);

	// Determine admittance of the part of the system
	MatrixXcd compute_equivalent_admittance_parameters_num(SubNetwork* subnet, double frequency);
	// Determine visible impedance from one of outputs of subnetwork when other outputs are closed
	MatrixXcd compute_closing_impedance(SubNetwork*, string&, vector<MatrixXcd>&);

	void compute_transfer_function(string converter_name, string location, double frequency);

	// Print summary of areas
	void print_summary() const;
private:
	// Names for identification
	std::vector<std::string> ac_grid_names;
	std::vector<std::string> dc_grid_names;

	// Core hierarchical system representation
	std::unordered_map<std::string, SubNetwork*> ac_grids;  // AC grids as subnetworks
	std::unordered_map<std::string, SubNetwork*> dc_grids;  // DC grids as subnetworks
	std::unordered_map<std::string, Element*> converters; // Converter subnetworks

	// Optional: internal mapping of converter interconnections between subnetworks
	std::unordered_map<std::string, std::pair<SubNetwork*, SubNetwork*>> converter_connections;
};


#endif