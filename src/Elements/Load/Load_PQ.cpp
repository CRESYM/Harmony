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


// Power flow computation for AC networks
void LoadPQ::computePowerFlowAC(std::map<std::string, double>& busAC,
    std::map<std::string, double>& global_params) const {

    double Pd = P / global_params["baseMVA"];
    double Qd = Q / global_params["baseMVA"];

    busAC["Pd"] += Pd;
    busAC["Qd"] += Qd;
}

