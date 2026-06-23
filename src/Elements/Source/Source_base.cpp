/**
 * @file Source_base.cpp
 * @brief Implementation of Base class for AC and DC voltage sources and generators.
 */
#include "Source_base.h"

// Power flow computations for AC and DC networks
void Source_base::computePowerFlow(std::map<std::string, double>& gen,
    std::map<std::string, double>& globalParams) const {

	string area = element_location.substr(0, 2); // Extract area code from element_location

	if ((area[0] == 'A' || area[0] == 'a') && (area[1] == 'c' || area[1] == 'C')) { // AC network
		for (auto& [key, value] : element_OPF_info)
			gen[key] = value;

		gen["grid"] = (int)element_location[2] - '0'; // Example of setting grid based on element_location
		gen["area"] = (int)element_location[2] - '0'; // Example of setting area based on element_location
		gen["Vg"] = V[0] * 1.0 / 1e3 / globalParams["ACbaseKV"]; // Convert to kV for OPF
		gen["Zsrc"] = Zsrc[0];
	}
	else 
		throw std::runtime_error("Power flow is defined only for AC networks.");
}