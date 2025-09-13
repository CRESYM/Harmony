#include "Load_base.h"


// Power flow computation for DC networks
void Load_base::computePowerFlowDC(std::map<std::string, double>& branchDCData,
    std::map<std::string, double>& globalParams) const {
    branchDCData["grid"] = (int)element_location[2] - '0'; // Example of setting grid based on element_location
    branchDCData["area"] = (int)element_location[2] - '0'; // Example of setting area based on element_location
    for (auto& [key, value] : element_OPF_info)
        branchDCData[key] = value;
}

// Power flow computation for AC networks
void Load_base::computePowerFlowAC(std::map<std::string, double>& branchData,
    std::map<std::string, double>& globalParams) const {
    branchData["grid"] = (int)element_location[2] - '0'; // Example of setting grid based on element_location
	branchData["area"] = (int)element_location[2] - '0'; // Example of setting area based on element_location
    for (auto& [key, value] : element_OPF_info)
        branchData[key] = value;
}