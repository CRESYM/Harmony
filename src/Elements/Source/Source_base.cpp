#include "Source_base.h"

// Power flow computations for AC and DC networks
void Source_base::computePowerFlowAC(std::map<std::string, double>& gen,
    std::map<std::string, double>& globalParams) const {
    for (auto& [key, value] : element_OPF_info)
        gen[key] = value;

	gen["grid"] = (int)element_location[2] - '0'; // Example of setting grid based on element_location
	gen["area"] = (int)element_location[2] - '0'; // Example of setting area based on element_location
}