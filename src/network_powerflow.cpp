#include "network.h"
#include "Element.h"
#include "Bus.h"
#include "Constants.h"

#include <algorithm>



void Network::addBusAC(std::vector<std::vector<std::string>>& dict_ac,
    const std::vector<std::string>& bus_info)
{
    dict_ac.push_back(bus_info);
}

void Network::addBusDC(std::vector<std::vector<std::string>>& dict_dc,
    const std::vector<std::string>& bus_info)
{
    dict_dc.push_back(bus_info);
}

void Network::make_BranchAC(Element* element,
    std::map<std::string, std::map<std::string, std::map<std::string, double>>>& data,
    std::map<std::string, double>& global_params)
{
    element->computePowerFlowAC(data["branch"], global_params);
}

void Network::make_BranchDC(Element* element,
    std::map<std::string, std::map<std::string, std::map<std::string, double>>>& data,
    std::map<std::string, double>& global_params)
{
    element->computePowerFlowDC(data["branchdc"], global_params);
}

void Network::make_Converter(
    Element* element,
    std::vector<std::vector<std::string>>& dict_dc,
    std::vector<std::vector<std::string>>& dict_ac,
    std::vector<std::string> new_i,
    std::vector<std::string> new_o,
    std::map<std::string, std::map<std::string, std::map<std::string, double>>>& data,
    std::map<std::string, double>& global_params)
{
    int key_i, key_o;

    if (!new_i.empty()) {
        dict_dc.push_back(new_i);
        // Emulate add_bus_dc(data);
        key_i = static_cast<int>(data["busdc"].size()) + 1;
    }
    else {
        key_i = -1; 
    }

    if (!new_o.empty()) {
        dict_ac.push_back(new_o);
        // Emulate add_bus_ac(data);
        key_o = static_cast<int>(data["bus"].size()) + 1;
    }
    else {
        key_o = -1; 
    }

    int key = static_cast<int>(data["convdc"].size()) + 1;

    std::map<std::string, double> converter_data;
    converter_data["busdc_i"] = static_cast<double>(key_i);
    converter_data["busac_i"] = static_cast<double>(key_o);

    data["convdc"][std::to_string(key)] = converter_data;

    element->computePowerFlowDC(data["branchdc"], global_params);
    element->computePowerFlowAC(data["branch"], global_params);
}

void Network::make_Generator(Element* element,
    std::map<std::string, std::map<std::string, std::map<std::string, double>>>& data)
{
    std::map<std::string, double> dummy_globals; 
    element->computePowerFlowAC(data["gen"], dummy_globals);
}

std::map<std::string, double> Network::PowerFlow() {
    std::map<std::string, double> global_dict;

    double omega = 2 * M_PI * 50; 
    global_dict["omega"] = omega;

    // Define data structures
    std::vector<std::vector<std::string>> dict_ac;
    std::vector<std::vector<std::string>> dict_dc;

    // Define and initialize data map
    std::map<std::string, std::map<std::string, std::map<std::string, double>>> data;

    // Initialize specific elements of the data map
    data["source_type"]["matpower"]["0"] = 0;
    data["name"]["network"]["0"] = 0;
    data["source_version"]["0.0.0"]["0"] = 0;
    data["per_unit"]["true"]["0"] = 1;
    data["dcpol"]["2"]["0"] = 1;
    data["baseMVA"]["100"]["0"] = 100;

    // Add baseMVA to global_dict for solver use
    global_dict["baseMVA"] = 100;

    // Initialize empty elements of the data map
    std::vector<std::string> keys = {
        "bus", "busdc", "shunt", "dcline", "storage", "switch",
        "load", "branch", "branchdc", "gen", "convdc"
    };

    for (const auto& key : keys) {
        data[key] = {}; // Empty structure for each
    }
    // ...
    return global_dict;
}

