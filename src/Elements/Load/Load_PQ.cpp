#include "Load_PQ.h"

LoadPQ::LoadPQ(const std::string& symbol, const std::string& location, int pins, vector<double> values) : Load_base(symbol, location, pins, pins) {
    if (pins <= 0) {
        throw std::invalid_argument("Invalid number of pins, must be greater than 0!");
    }
    if (values.size() != 2) {
        throw std::invalid_argument("LoadPQ requires exactly 2 values: P and Q.");
	}

    // Initialize active and reactive power
    P = values[0];
    Q = values[1];
}

void LoadPQ::computePowerFlow(std::map<std::string, double>& busAC,
    std::map<std::string, double>& global_params) const {
    string area = element_location.substr(0, 2); // Extract area code from element_location
    if ((area[0] == 'D' || area[0] == 'd') && (area[1] == 'C' || area[1] == 'c')) { // DC network
        throw std::runtime_error("LoadPQ is not applicable for DC networks.");
    }
    else if ((area[0] == 'A' || area[0] == 'a') && (area[1] == 'c' || area[1] == 'C')) { // AC network
        double Pd = P / global_params["baseMVA"];
        double Qd = Q / global_params["baseMVA"];
        busAC["Pd"] += Pd;
        busAC["Qd"] += Qd;
    }
    else {
        throw std::runtime_error("Invalid network type specified in global parameters.");
    }
}
