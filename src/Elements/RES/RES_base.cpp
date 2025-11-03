#include "RES_base.h"

void RES_base::computePowerFlow(std::map<std::string, double>& branchData,
    std::map<std::string, double>& globalParams) const
{
    std::string area = element_location.substr(0, 2);

    if ((area[0] == 'A' || area[0] == 'a') && (area[1] == 'C' || area[1] == 'c')) {
        branchData["grid"] = static_cast<int>(element_location[2] - '0');
        branchData["area"] = static_cast<int>(element_location[2] - '0');
    }
    else if ((area[0] == 'D' || area[0] == 'd') && (area[1] == 'C' || area[1] == 'c')) {
        branchData["grid"] = static_cast<int>(element_location[2] - '0');
        branchData["area"] = static_cast<int>(element_location[2] - '0');
    }
    else {
        throw std::runtime_error("[RES_base::computePowerFlow] Invalid network type.");
    }
}