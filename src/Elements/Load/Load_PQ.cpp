#include "Load_PQ.h"

LoadPQ::LoadPQ(const std::string& symbol, int pins) : Element(symbol, pins, pins) {
    if (pins <= 0) {
        throw std::invalid_argument("Invalid number of pins, must be greater than 0!");
    }

	// It has only OPF representation, so we do not need to compute Y-parameters
    element_OPF_info = { symbol, "1", "0", "0", "0"};
}


// Power flow computation for AC networks
void LoadPQ::computePowerFlowAC(std::map<std::string, std::map<std::string, double>>& branchData,
    std::map<std::string, double>& globalParams) const {
    int key = branchData.size();  // Unique branch identifier

    branchData[std::to_string(key)]["load"] = 1;
    branchData[std::to_string(key)]["br_r"] = 0;
    branchData[std::to_string(key)]["br_x"] = 0;
    branchData[std::to_string(key)]["g_fr"] = 0;
    branchData[std::to_string(key)]["b_fr"] = 0;
    branchData[std::to_string(key)]["g_to"] = 0;
    branchData[std::to_string(key)]["b_to"] = 0;
}


// Power flow computation for DC networks
void LoadPQ::computePowerFlowDC(std::map<std::string, std::map<std::string, double>>& branchDCData,
    std::map<std::string, double>& globalParams) const {
    int key = branchDCData.size();  // Unique DC branch identifier
    branchDCData[std::to_string(key)]["l"] = 0.0;
    branchDCData[std::to_string(key)]["c"] = 0.0;
    branchDCData[std::to_string(key)]["r"] = 0;
}

