/**
 * @file Load_base.cpp
 * @brief Implementation of Base class for load elements connected to network buses.
 */
#include "Load_base.h"

void Load_base::computePowerFlow(std::map<std::string, double>& branchData,
	std::map<std::string, double>& globalParams) const {
	string area = element_location.substr(0, 2); // Extract area code from element_location
	if ((area[0] == 'D' || area[0] == 'd') && (area[1] == 'C' || area[1] == 'c')) { // DC network
		branchData["grid"] = (int)element_location[2] - '0'; // Example of setting grid based on element_location
		branchData["area"] = (int)element_location[2] - '0'; // Example of setting area based on element_location
		for (auto& [key, value] : element_OPF_info)
			branchData[key] = value;
	}
	else if ((area[0] == 'A' || area[0] == 'a') && (area[1] == 'c' || area[1] == 'C')) { // AC network
		branchData["grid"] = (int)element_location[2] - '0'; // Example of setting grid based on element_location
		branchData["area"] = (int)element_location[2] - '0'; // Example of setting area based on element_location
		for (auto& [key, value] : element_OPF_info)
			branchData[key] = value;
	}
	else {
		throw std::runtime_error("Invalid network type specified in global parameters.");
	}
}