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

	// Determine impedance of the part of the system

	void compute_equivalent_impedance(Network* net, std::vector<Bus*> start_buses, std::vector<Bus*> end_buses, std::vector<Element*> skip_elements);
	void compute_equivalent_impedance_num(Network* net, std::vector<Bus*> start_buses, std::vector<Bus*> end_buses, std::vector<Element*> skip_elements, double omega_num);
private:
	vector<string> ac_grids; // List of AC grids in the system
	vector<string> dc_grids; // List of DC grids in the system

	// It is probably better to pass the network as an argument to the functions
	// and to make maps with bases and elements in each grid
	// that would make a job easier
};


#endif