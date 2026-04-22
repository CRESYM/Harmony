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

	void add_areas(Network* net);

	// Determine admittance of the part of the system
	MatrixXcd compute_equivalent_admittance_parameters_num(SubNetwork* subnet, double frequency);
	// Determine visible impedance from one of outputs of subnetwork when other outputs are closed
	MatrixXcd compute_closing_impedance(SubNetwork*, string&, MatrixXcd&, MatrixXcd&);

	MatrixXcd compute_transfer_function(string converter_name, string location, double frequency);

	// Function to write the Y-parameter matrix to a file over a frequency range 
	void writeFileTF(string converter_name, string location, double start_frequency, double end_frequency, int number_of_points);

	// Function to plot the Y-parameter matrix entries
	void bodeplotTF(string converter_name, string location, double start_frequency, double end_frequency, int number_of_points);
	void nyquistplotTF(string converter_name, string location, double start_frequency, double end_frequency, int number_of_points);

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

	// Determine closing impedance of the part of the system, private version, not to be used outside
	void compute_equivalent_impedance(Network* net, std::vector<Bus*> start_buses, std::vector<Bus*> end_buses, std::vector<Element*> skip_elements);
	MatrixXcd compute_equivalent_impedance_num(Network* net, std::vector<Bus*> start_buses, std::vector<Bus*> end_buses, std::vector<Element*> skip_elements, double frequency);

};


#endif