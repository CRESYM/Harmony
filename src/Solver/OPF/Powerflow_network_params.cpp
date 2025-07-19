#include "powerflow.h"
#include "../Helper_Functions/Helper_Functions.h"

#include "../../network.h"
#include "../../Bus.h"
#include "../../Include_components.h"


void PowerFlow::addBusAC(std::vector<std::vector<std::string>>& dict_ac,
    Bus* bus, std::map<std::string, double>& global_params, bool print_info /* =false*/)
{
    int id = static_cast<int>(dict_ac.size()) + 1;

    std::string bus_id = std::to_string(id);
    std::string bus_name = bus->getBusName(); // bus_info[0];

    std::string row = std::to_string(id - 1);
    auto& busRow = data["busAC"][row];
    busRow["bus_i"] = id;
    busRow["type"] = 1.0;
    busRow["Pd"] = 0.0;
    busRow["Qd"] = 0.0;
    busRow["Gs"] = 0.0;
    busRow["Bs"] = 0.0;
	busRow["area"] = 1; // Default, but bus can overwrite it
	busRow["Vm"] = 1.0; // Default, but bus can overwrite it
    busRow["Va"] = 0.0;
    busRow["baseKV"] = global_params["ACbasekV"];
    busRow["zone"] = 1.0;
    busRow["Vmax"] = 1.1; // Default, but bus can overwrite it    
	busRow["Vmin"] = 0.9; // Default, but bus can overwrite it
    busRow["grid"] = 1; // Default, but bus can overwrite it

	bus->computePowerFlowAC(busRow, global_params);

	string area_id = std::to_string(busRow["area"]);
	double v_upper = busRow["Vmax"];
	double v_lower = busRow["Vmin"];
	double rated_kv = busRow["baseKV"];
	string rated_kv_str = std::to_string(rated_kv);
	double base_mw = global_params["baseMVA"];
	string base_mw_str = std::to_string(base_mw);

    bool is_pu = (v_upper <= 2.0 && v_lower <= 2.0);

    double v_upper_pu, v_lower_pu;

    if (is_pu) {
        v_upper_pu = v_upper;
        v_lower_pu = v_lower;
    }
    else {
        v_upper_pu = v_upper / rated_kv;
        v_lower_pu = v_lower / rated_kv;
    }

    auto exists = std::any_of(dict_ac.begin(), dict_ac.end(),
        [&](const auto& row) { return row[0] == bus_name; });
    if (exists)
        throw std::runtime_error("[Error: Duplicate entry for bus '" + bus_name + "'.");

    dict_ac.push_back({
       bus_id,
       bus_name,
       area_id,
       base_mw_str,
       rated_kv_str,
       std::to_string(v_upper_pu),
       std::to_string(v_lower_pu)
        });

    busName2Id_[bus_name] = id;

    if (print_info)
    {
        std::cout << "\n[data.busAC]  ("
            << data["busAC"].size() << " × " << 14
            << ")\n";

        const char* colOrder[14] = { "bus_i","type","Pd","Qd",
                                     "Gs","Bs","area","Vm","Va",
                                     "baseKV","zone","Vmax","Vmin","grid" };

        for (size_t r = 0; r < data["busAC"].size(); ++r)
        {
            const auto& row = data["busAC"][std::to_string(r)];
            for (int c = 0; c < 14; ++c)
                std::cout << std::setw(8) << row.at(colOrder[c]) << " ";
            std::cout << '\n';
        }
        std::cout << std::endl;
    }

    if (print_info) {
        std::cout << "Bus '" << bus_name << "' successfully added.\n";
    }
}

void PowerFlow::addBusDC(std::vector<std::vector<std::string>>& dict_dc,
    Bus* bus, std::map<std::string, double>& global_params,
    bool  print_info /* =false*/)
{
    int id = static_cast<int>(dict_dc.size()) + 1;

    std::string bus_id = std::to_string(id);
    std::string bus_name = bus->getBusName(); 

    std::string row = std::to_string(id - 1);
    auto& busDCRow = data["busDC"][row];
    busDCRow["bus_i"] = id;
    busDCRow["type"] = 1.0;
    busDCRow["Pd"] = 0.0;
    busDCRow["Qd"] = 0.0;
    busDCRow["Gs"] = 0.0;
    busDCRow["Bs"] = 0.0;
    busDCRow["area"] = 1.0;
    busDCRow["Vm"] = 1.0;
    busDCRow["Va"] = 0.0;
    busDCRow["baseKV"] = global_params["DCbasekV"];
    busDCRow["zone"] = 1.0;
    busDCRow["Vmax"] = 1.1;
    busDCRow["Vmin"] = 0.9;

    bus->computePowerFlowDC(busDCRow, global_params);

    string area_id = std::to_string(busDCRow["area"]);
    double v_upper = busDCRow["Vmax"];
    double v_lower = busDCRow["Vmin"];
    double rated_kv = busDCRow["baseKV"];
    string rated_kv_str = std::to_string(rated_kv);
    double base_mw = global_params["baseMVA"];
    string base_mw_str = std::to_string(base_mw);

    bool is_pu = (v_upper <= 2.0 && v_lower <= 2.0);

    double v_upper_pu, v_lower_pu;

    if (is_pu) {
        v_upper_pu = v_upper;
        v_lower_pu = v_lower;
    }
    else {
        v_upper_pu = v_upper / rated_kv;
        v_lower_pu = v_lower / rated_kv;
    }

    if (v_lower_pu > v_upper_pu) {
        std::cerr << "[addBusDC] Error: voltage_lower (" << v_lower_pu
            << " pu) is greater than voltage_upper (" << v_upper_pu << " pu) for bus "
            << bus_name << ".\n";
    }

    auto exists = std::any_of(dict_dc.begin(), dict_dc.end(),
        [&](const auto& row) { return row[0] == bus_name; });
    if (exists)
        throw std::runtime_error("[addBusDC] Error: Duplicate entry for bus '" + bus_name + "'.");

    dict_dc.push_back({
      bus_id,
      bus_name,
      rated_kv_str,
      std::to_string(v_upper_pu),
      std::to_string(v_lower_pu)
        });

    busName2Id_[bus_name] = id;


    if (print_info)
    {
        std::cout << "\n[data.busDC] ("
            << data["busDC"].size() << " × " << 13
            << ")\n";

        const char* colOrder[14] = { "bus_i","type","Pd","Qd",
                                     "Gs","Bs","area","Vm","Va",
                                     "baseKV","zone","Vmax","Vmin" };

        for (size_t r = 0; r < data["busDC"].size(); ++r)
        {
            const auto& row = data["busDC"][std::to_string(r)];
            for (int c = 0; c < 13; ++c)
                std::cout << std::setw(8) << row.at(colOrder[c]) << " ";
            std::cout << '\n';
        }
        std::cout << std::endl;
    }

}


void PowerFlow::make_BranchAC(Element* element, std::map<std::string, double>& global_params,
    bool print_info /* = false */)
{
    std::string row = std::to_string(data["branchAC"].size());
    data["branchAC"][row] = {}; // Ensure there's at least one row
    std::vector<std::string> keys = {
        "fbus","tbus","r","x","b","rateA","rateB","rateC",
            "ratio","angle","status","angmin","angmax","grid"
    };

    for (const auto& key : keys) {
        data["branchAC"][row][key] = 0; // Empty structure for each
    }

    std::vector<Bus*> ends = element->getBuses();

    if (ends.size() != 2)
        throw std::runtime_error("[make_BrancADC] Error: DC Branch should just connected with two DC Buses");

    std::string branch_name = element->getElementSymbol();
    const std::string& fBus_name = ends[0]->getBusName();
    const std::string& tBus_name = ends[1]->getBusName();

    int fId = busName2Id_[fBus_name];
    int tId = busName2Id_[tBus_name];

    /* ------- Write data["branchAC"] ------- */
    auto& brRow = data["branchAC"][row];

	std::string area_id = "1"; // Default area ID, can be changed based on your logic

    brRow["fbus"] = fId;
    brRow["tbus"] = tId;
    brRow["rateA"] = 100.0;
    brRow["rateB"] = 100.0;
    brRow["rateC"] = 100.0;
    brRow["ratio"] = 0.0;
    brRow["angle"] = 0.0;
    brRow["status"] = 1.0;
    brRow["angmin"] = -360.0;
    brRow["angmax"] = 360.0;
    brRow["grid"] = std::stod(area_id);

    element->computePowerFlowDC(data["branchAC"][row], global_params);

    if (print_info)
    {
        constexpr const char* cols[14] = {
            "fbus","tbus","r","x","b","rateA","rateB","rateC",
            "ratio","angle","status","angmin","angmax","grid"
        };
        std::cout << "\n[data.branchAC] (" << data["branchAC"].size() << " × 14)\n";
        for (size_t r = 0; r < data["branchAC"].size(); ++r)
        {
            const auto& rmap = data["branchAC"].at(std::to_string(r));
            for (int c = 0; c < 14; ++c)
                std::cout << std::setw(10) << rmap.at(cols[c]) << ' ';
            std::cout << '\n';
        }
        std::cout << std::endl;
    }

    if (print_info) {
        std::cout << "Branch '" << branch_name << "' successfully added.\n";
	}
}


void PowerFlow::make_BranchDC(Element* element, std::map<std::string, double>& global_params,
    bool print_info /* = false */)

{
    std::string row = std::to_string(data["branchDC"].size());
	data["branchDC"][row] = {}; // Ensure there's at least one row
    std::vector<std::string> keys = {
        "fbus", "tbus", "r", "x", "b", "rateA", "rateB",
            "rateC", "ratio", "angle", "status", "angmin", "angmax"
    };

    for (const auto& key : keys) {
        data["branchDC"][row][key] = 0; // Empty structure for each
    }

    std::vector<Bus*> ends = element->getBuses();

    if (ends.size() != 2)
        throw std::runtime_error("[make_BranchDC] Error: DC Branch should just connected with two DC Buses");

    std::string branch_name = element->getElementSymbol();
    const std::string& fBus_name = ends[0]->getBusName();
    const std::string& tBus_name = ends[1]->getBusName(); 

    int fId = busName2Id_[fBus_name];
    int tId = busName2Id_[tBus_name];

    /* ------- Write data["branchDC"] ------- */
    auto& brRow = data["branchDC"][row];

    brRow["fbus"] = fId;
    brRow["tbus"] = tId;
    brRow["rateA"] = 100.0;
    brRow["rateB"] = 100.0;
    brRow["rateC"] = 100.0;
    brRow["ratio"] = 0.0;
    brRow["angle"] = 0.0;
    brRow["status"] = 1.0;
    brRow["angmin"] = -0.0;
    brRow["angmax"] = 0.0;

    element->computePowerFlowDC(data["branchDC"][row], global_params);

    if (print_info)
    {
        constexpr const char* cols[13] = {
            "fbus","tbus","r","x","b","rateA","rateB","rateC",
            "ratio","angle","status","angmin","angmax"
        };
        std::cout << "\n[data.branchDC] (" << data["branchDC"].size() << " × 13)\n";
        for (size_t r = 0; r < data["branchDC"].size(); ++r)
        {
            const auto& rmap = data["branchDC"].at(std::to_string(r));
            for (int c = 0; c < 13; ++c)
                std::cout << std::setw(10) << rmap.at(cols[c]) << ' ';
            std::cout << '\n';
        }
        std::cout << std::endl;
    }

    if (print_info) {
        std::cout << "Branch '" << branch_name << "' successfully added.\n";
    }
}

void PowerFlow::make_Converter(Element* element, std::map<std::string, double>& global_params,
    bool print_info /*=false*/)
{
    std::vector<Bus*> ends = element->getBuses();

    if (ends.size() != 2)
        throw std::runtime_error("[make_Converter] Error: Converter must connect exactly 2 buses");

    Bus* acBus = nullptr;
    Bus* dcBus = nullptr;

    for (Bus* b : ends) {
        if (b->getPinNumber() == 3)
            acBus = b;
        else
            dcBus = b;
    }

    if (!acBus || !dcBus)
        throw std::runtime_error("[make_Converter] Error: Need one AC-bus and one DC-bus");

    auto acIt = busName2Id_.find(acBus->getBusName());
    auto dcIt = busName2Id_.find(dcBus->getBusName());

    if (acIt == busName2Id_.end() || dcIt == busName2Id_.end())
        throw std::runtime_error("[make_Converter] Error: Unknown bus name(s)");

    int busac_i = acIt->second;
    int busdc_i = dcIt->second;



    constexpr const char* keys[20] = {
         "gridac","type_dc","type_ac",
         "rtf","xtf","bf","rc","xc",
         "basekVac","Vmmax","Vmmin","Imax",
         "LossA","LossB","LossCrec","LossCinv",
         "droop","Pdcset","Vdcset","dvdcset"
    };

    std::string rowKey = std::to_string(data["conv"].size());
    auto& convRow = data["conv"][rowKey];

    //for (size_t i = 0; i < 20; ++i)
    //    convRow[keys[i]] = std::stod(conv_info[i + 1]);

    convRow["busdc_i"] = busdc_i;
    convRow["busac_i"] = busac_i;
    convRow["P_g"] = 0.0;
    convRow["Q_g"] = 0.0;
    convRow["Vtar"] = 1.0;
    convRow["status"] = 1.0;

    constexpr const char* cols[] = {
        "busdc_i","busac_i","gridac","type_dc","type_ac",
        "P_g","Q_g","Vtar",
        "rtf","xtf","bf","rc","xc",
        "basekVac","Vmmax","Vmmin","Imax","status",
        "LossA","LossB","LossCrec","LossCinv",
        "droop","Pdcset","Vdcset","dvdcset"
    };
    const int nCols = sizeof(cols) / sizeof(cols[0]);

    if (print_info) {
        std::cout << "\n[data.conv] (" << data["conv"].size()
            << " × " << nCols << ")\n";
        for (size_t r = 0; r < data["conv"].size(); ++r) {
            const auto& m = data["conv"].at(std::to_string(r));
            for (int c = 0; c < nCols; ++c)
                std::cout << std::setw(12) << m.at(cols[c]) << ' ';
            std::cout << '\n';
        }
    }

}


void PowerFlow::make_Generator(Element* element, std::map<std::string, double>& global_params,
    bool print_info /* = false */)
{
    vector<Bus*> buses = element->getBuses();
	Bus* bus = nullptr;

    for (Bus* b : buses) {
        if (b->getBusName() != "gnd")
			bus = b;  
    }

    // Generators will be made connected to ground on the other side
    if (!bus) throw std::runtime_error("[make_Generator] Error: No bus connected");

    const std::string& bus_name = bus->getBusName();
    auto idIt = busName2Id_.find(bus_name);
    if (idIt == busName2Id_.end())
        throw std::runtime_error("[make_Generator] Error: Bus name unknown");
    int bus_id = idIt->second;

    /* ---------- Write data["genAC"] ---------- */
    std::string rowGen = std::to_string(data["genAC"].size());
    auto& gRow = data["genAC"][rowGen];
    std::vector<std::string> keys = {
            "bus","Pg","Qg","Qmax","Qmin","Vg","mBase","status","Pmax","Pmin",
            "Pc1","Pc2","Qc1min","Qc1max","Qc2min","Qc2max",
            "ramp_agc","ramp_10","ramp_30","ramp_q","apf","grid"
    };
    for (const auto& key : keys) {
        gRow[key] = 0; // Empty structure for each
    }

	
   
    gRow["bus"] = bus_id;
    //gRow["Qmax"] = Qmax;
    //gRow["Qmin"] = Qmin;
    gRow["Vg"] = 1.0;
    gRow["mBase"] = 100.0;
    gRow["status"] = 1.0;
    //gRow["Pmax"] = Pmax;
    //gRow["Pmin"] = Pmin;

    element->computePowerFlowAC(gRow, global_params);

    const std::string& area_id = to_string(gRow["area"]);
    const std::string& pmax_s = gen_info[3];
    const std::string& pmin_s = gen_info[4];
    const std::string& qmax_s = gen_info[5];
    const std::string& qmin_s = gen_info[6];
    const std::string& a_s = gen_info[7];
    const std::string& b_s = gen_info[8];
    const std::string& c_s = gen_info[9];

    double Pmax = std::stod(pmax_s), Pmin = std::stod(pmin_s);
    double Qmax = std::stod(qmax_s), Qmin = std::stod(qmin_s); */

        /*if (Pmin > Pmax) {
            throw std::invalid_argument("[make_Generator] Error: P_min > P_max for generator " + gen_name);
        }

        if (Qmin > Qmax) {
            throw std::invalid_argument("[make_Generator] Error: Q_min > Q_max for generator " + gen_name);
        }
    //gRow["grid"] = std::stod(area_id);

    /* ---------- Write data["genCostAC"] ---------- */
    std::string rowCost = std::to_string(data["genCostAC"].size());
    auto& cRow = data["genCostAC"][rowCost];

    cRow["model"] = 2.0;
    cRow["startup"] = 1500.0;
    cRow["shutdown"] = 0.0;
    cRow["n"] = 3.0;
    /*cRow["c2"] = std::stod(a_s);
    cRow["c1"] = std::stod(b_s);
    cRow["c0"] = std::stod(c_s);
    cRow["grid"] = std::stod(area_id);*/

    if (print_info)
    {
        constexpr const char* gCols[22] = {
            "bus","Pg","Qg","Qmax","Qmin","Vg","mBase","status","Pmax","Pmin",
            "Pc1","Pc2","Qc1min","Qc1max","Qc2min","Qc2max",
            "ramp_agc","ramp_10","ramp_30","ramp_q","apf","grid"
        };
        std::cout << "\n[data.genAC] (" << data["genAC"].size() << " × 22)\n";
        for (size_t r = 0; r < data["genAC"].size(); ++r) {
            const auto& rmap = data["genAC"].at(std::to_string(r));
            for (int c = 0; c < 22; ++c) std::cout << std::setw(9) << rmap.at(gCols[c]) << " ";
            std::cout << '\n';
        }

        constexpr const char* cCols[8] =
        { "model","startup","shutdown","n","c2","c1","c0","grid" };
        std::cout << "\n[data.genCostAC] (" << data["genCostAC"].size() << " × 8)\n";
        for (size_t r = 0; r < data["genCostAC"].size(); ++r) {
            const auto& rmap = data["genCostAC"].at(std::to_string(r));
            for (int c = 0; c < 8; ++c) std::cout << std::setw(9) << rmap.at(cCols[c]) << " ";
            std::cout << '\n';
        }
        std::cout << std::endl;
    }
}

void PowerFlow::make_Load(Element* element, std::map<std::string, double>& global_params,
    bool print_info /* = false */)
{
    std::string row = std::to_string(data["busAC"].size());
    data["busAC"][row] = {}; // Ensure there's at least one row
    std::vector<std::string> keys = {
        "bus_i","type","Pd","Qd","Gs","Bs","area","Vm","Va",
            "baseKV","zone","Vmax","Vmin","grid"
    };

    for (const auto& key : keys) {
        data["busAC"][row][key] = 0; // Empty structure for each
    }

    std::string load_name = element->getElementSymbol();
    
	vector<Bus*> buses = element->getBuses(); // Get the buses connected to the load element
    Bus* attachedBus = nullptr;

    for (Bus* b : buses) {
        if (b->getBusName() != "gnd")
            attachedBus = b;
    }

    if (!attachedBus)
        throw std::runtime_error(
            "[make_Load] Error: load element is not connected to any bus");

    const std::string& bus_name = attachedBus->getBusName();    

    auto& busRow = data["busAC"][row];
	busRow["bus_i"] = busName2Id_[bus_name];
	busRow["area"] = 1; // Load type
	busRow["Vm"] = global_params["basekV"]; // Voltage magnitude in kV

    if (print_info)
    {
        constexpr const char* colOrder[14] = {
            "bus_i","type","Pd","Qd","Gs","Bs","area","Vm","Va",
            "baseKV","zone","Vmax","Vmin","grid"
        };

        std::cout << "\n[data.busAC]  (" << data["busAC"].size()
            << " × 14)\n";

        for (size_t r = 0; r < data["busAC"].size(); ++r)
        {
            const auto& row = data["busAC"].at(std::to_string(r));
            for (int c = 0; c < 14; ++c)
                std::cout << std::setw(9) << row.at(colOrder[c]) << " ";
            std::cout << '\n';
        }
        std::cout << std::endl;
    }
}


void PowerFlow::make_OPF(Network* net, std::map<std::string, double>& global_dict, bool vscControl,
    bool writeTxt, bool plotResult)
{
    //// Define and initialize data map
    //std::map<std::string, std::map<std::string, std::map<std::string, double>>> data;

    // Initialize specific elements of the data map
    data["source_type"]["matpower"]["0"] = 0;
    data["name"]["network"]["0"] = 0;
    data["source_version"]["0.0.0"]["0"] = 0;
    data["per_unit"]["true"]["0"] = 1;
    data["dcpol"]["2"]["0"] = 1;
    data["baseMVA"]["100"]["0"] = 100;

    // Initialize empty elements of the data map
    std::vector<std::string> keys = {
        "bus", "busdc", "shunt", "dcline", "storage", "switch",
        "load", "branch", "branchDC", "gen", "convdc"
    };

    for (const auto& key : keys) {
        data[key] = {}; // Empty structure for each
    }

    // Define and initialize data map

    cout << "\n[make_OPF] Start making OPF data...\n";

	// Process buses
    // Define data structures
    std::vector<std::vector<std::string>> dict_ac;
    std::vector<std::vector<std::string>> dict_dc;
    for (const auto& [bus_name, bus] : net->getBuses())
    {
		cout << "[make_OPF] Processing bus: " << bus_name << endl;
        if (bus->getPinNumber() == 3) {
            addBusAC(dict_ac, bus, global_dict, writeTxt);
        }
        else if (bus->getPinNumber() == 1) {
            addBusDC(dict_dc, bus, global_dict, writeTxt);
        }
        else {
            throw std::runtime_error("[make_OPF] Error: Unsupported bus type.");
        }
    }
	// Process elements: loads, generators, which contribute to the buses data    
    auto& elements = net->getElements();
    for (const auto& [element_name, element] : elements)
    {
		cout << "[make_OPF] Processing element: " << element_name << endl;
        if (dynamic_cast<Load*>(element)) {
            make_Load(element, global_dict, true); // writeTxt);
        }
        else if (dynamic_cast<Generator*>(element)) {
            make_Generator(element, global_dict, writeTxt);
        }
    }

	// Process branches: AC and DC branches, i.e., transmission lines, impedances, etc.
    for (const auto& [element_name, element] : elements)
    {
        if (dynamic_cast<Impedance*>(element)) {
            if (element->getInputPins() == 3) {
                make_BranchAC(element, global_dict, true); // writeTxt);
            }
            else if (element->getInputPins() == 1) {
                make_BranchDC(element, global_dict, true); // writeTxt);
            }
            else {
                throw std::runtime_error("[make_OPF] Error: Unsupported impedance pin number.");
            }

        }
        else if (dynamic_cast<MMC*>(element)) {
            make_Converter(element, global_dict, writeTxt);
        }
        else {
            throw std::runtime_error("Unsupported element type.");
        }
	}

	cout << "[make_OPF] Finished processing elements.\n";

    /* 2. Transform to Eigen::MatrixXd */
        // Initialize specific elements of the data map
    MatrixXd busDC = map2dense(data.at("busDC"),
        { "bus_i","type","Pd","Qd","Gs","Bs","area",
          "Vm","Va","baseKV","zone","Vmax","Vmin" });

    cout << "[make_OPF] Finished making OPF data.\n";

    MatrixXd branchDC = map2dense(data.at("branchDC"),
        { "fbus","tbus","r","x","b","rateA","rateB",
          "rateC","ratio","angle","status","angmin","angmax" });

    MatrixXd convDC = map2dense(data.at("conv"),
        { "busdc_i","busac_i","gridac","type_dc","type_ac",
          "P_g","Q_g","Vtar",
          "rtf","xtf","bf","rc","xc",
          "basekVac","Vmmax","Vmmin","Imax","status",
          "LossA","LossB","LossCrec","LossCinv",
          "droop","Pdcset","Vdcset","dvdcset" });

    MatrixXd busAC = map2dense(data.at("busAC"),
        { "bus_i","type","Pd","Qd",
          "Gs","Bs","area","Vm","Va",
          "baseKV","zone","Vmax","Vmin","grid" });

    MatrixXd branchAC = map2dense(data.at("branchAC"),
        { "fbus","tbus","r","x","b","rateAC","rateB","rateC",
          "ratio","angle","status","angmin","angmax","grid" });

    MatrixXd genAC = map2dense(data.at("genAC"),
        { "bus","Pg","Qg","Qmax","Qmin","Vg","mBase","status",
          "Pmax","Pmin","Pc1","Pc2","Qc1min","Qc1max",
          "Qc2min","Qc2max","ramp_agc","ramp_10","ramp_30",
          "ramp_q","apf","grid" });

    MatrixXd gencostAC = map2dense(data.at("genCostAC"),
        { "model","startup","shutdown","n","c2","c1","c0","grid" });

    /* 3. Put inside unordered_map<string, MatrixXd> */
    std::unordered_map<std::string, MatrixXd> dataOPF;
    dataOPF["busDC"] = std::move(busDC);
    dataOPF["branchDC"] = std::move(branchDC);
    dataOPF["converter"] = std::move(convDC);

    dataOPF["busAC"] = std::move(busAC);
    dataOPF["branchAC"] = std::move(branchAC);
    dataOPF["generator"] = std::move(genAC);
    dataOPF["gencost"] = std::move(gencostAC);

    // Below need to be revise later
    if (dataOPF["converter"].rows() >= 3) {
        dataOPF["converter"](0, 5) = -60;  // P_g
        dataOPF["converter"](0, 6) = -40;  // Q_g
        dataOPF["converter"](2, 5) = 35;
        dataOPF["converter"](2, 6) = 5;
    }


    //solveHmo_opf(dataOPF, vscControl, writeTxt, plotResult);
    solve_opf("", "", &dataOPF, vscControl, writeTxt, plotResult);
}


void PowerFlow::load_params_ac(const std::string& acgrid_name, const std::unordered_map<std::string, Eigen::MatrixXd>& dataOPF) {
    // Load AC data
    if (dataOPF.empty()) {
        // From file
        network_ac = create_ac(acgrid_name);
        baseMVA_ac = network_ac["baseMVA"](0, 0);
        bus_entire_ac = network_ac["bus"];
        branch_entire_ac = network_ac["branch"];
        gen_entire_ac = network_ac["generator"];
        gencost_entire_ac = network_ac["gencost"];
    }
    else {
        // From OPF input map
        baseMVA_ac = 100.0;  // or dataOPF["baseMVA"](0, 0) if available
        bus_entire_ac = dataOPF.at("busAC");
        branch_entire_ac = dataOPF.at("branchAC");
        gen_entire_ac = dataOPF.at("generator");
        gencost_entire_ac = dataOPF.at("gencost");
    }

    //Identify number of grids by unique area ID
    std::set<int> unique_areas;
    for (int i = 0; i < bus_entire_ac.rows(); ++i) {
        unique_areas.insert(static_cast<int>(bus_entire_ac(i, 13)));
    }
    ngrids = static_cast<int>(unique_areas.size());

    // Resize containers
    bus_ac.resize(ngrids);
    branch_ac.resize(ngrids);
    generator_ac.resize(ngrids);
    gencost_ac.resize(ngrids);
    pd_ac.resize(ngrids);
    qd_ac.resize(ngrids);
    nbuses_ac.resize(ngrids);
    nbranches_ac.resize(ngrids);
    ngens_ac.resize(ngrids);
    GG_ac.resize(ngrids);
    BB_ac.resize(ngrids);
    fbus_ac.resize(ngrids);
    tbus_ac.resize(ngrids);
    GG_ft_ac.resize(ngrids);
    BB_ft_ac.resize(ngrids);
    GG_tf_ac.resize(ngrids);
    BB_tf_ac.resize(ngrids);
    IDtoCountmap.resize(ngrids);
    refbuscount_ac.resize(ngrids, -1);
    recRef.resize(ngrids);

    // Partition per grid
    for (int ng = 0; ng < ngrids; ++ng) {
        std::vector<int> bus_rows, branch_rows, gen_rows, gencost_rows;

        // Filter buses
        for (int i = 0; i < bus_entire_ac.rows(); ++i) {
            if (static_cast<int>(bus_entire_ac(i, 13)) == ng + 1)
                bus_rows.push_back(i);
        }
        bus_ac[ng].resize(bus_rows.size(), bus_entire_ac.cols());
        for (size_t i = 0; i < bus_rows.size(); ++i)
            bus_ac[ng].row(i) = bus_entire_ac.row(bus_rows[i]);

        // Filter branches
        for (int i = 0; i < branch_entire_ac.rows(); ++i) {
            if (static_cast<int>(branch_entire_ac(i, 13)) == ng + 1)
                branch_rows.push_back(i);
        }
        branch_ac[ng].resize(branch_rows.size(), branch_entire_ac.cols());
        for (size_t i = 0; i < branch_rows.size(); ++i)
            branch_ac[ng].row(i) = branch_entire_ac.row(branch_rows[i]);

        // Filter generators
        for (int i = 0; i < gen_entire_ac.rows(); ++i) {
            if (static_cast<int>(gen_entire_ac(i, 21)) == ng + 1)
                gen_rows.push_back(i);
        }
        generator_ac[ng].resize(gen_rows.size(), gen_entire_ac.cols());
        for (size_t i = 0; i < gen_rows.size(); ++i)
            generator_ac[ng].row(i) = gen_entire_ac.row(gen_rows[i]);

        // Filter gencosts
        for (int i = 0; i < gencost_entire_ac.rows(); ++i) {
            if (static_cast<int>(gencost_entire_ac(i, 7)) == ng + 1)
                gencost_rows.push_back(i);
        }
        gencost_ac[ng].resize(gencost_rows.size(), gencost_entire_ac.cols());
        for (size_t i = 0; i < gencost_rows.size(); ++i)
            gencost_ac[ng].row(i) = gencost_entire_ac.row(gencost_rows[i]);

        //Local index and references
        nbuses_ac[ng] = static_cast<int>(bus_ac[ng].rows());
        nbranches_ac[ng] = static_cast<int>(branch_ac[ng].rows());
        ngens_ac[ng] = static_cast<int>(generator_ac[ng].rows());

        IDtoCountmap[ng] = Eigen::VectorXi::Zero(nbuses_ac[ng]);
        for (int i = 0; i < nbuses_ac[ng]; ++i) {
            int bus_id = static_cast<int>(bus_ac[ng](i, 0));
            IDtoCountmap[ng](bus_id - 1) = i;
            if (bus_ac[ng](i, 1) == 3) {
                refbuscount_ac[ng] = i;
            }
        }
        recRef[ng].push_back(refbuscount_ac[ng]);

        //Compute admittance matrix
        Eigen::SparseMatrix<std::complex<double>> YY_ac = makeYbus(baseMVA_ac, bus_ac[ng], branch_ac[ng]);
        GG_ac[ng] = YY_ac.real();
        BB_ac[ng] = YY_ac.imag();

        //from/to buses and loads
        fbus_ac[ng] = branch_ac[ng].col(0).cast<int>();
        tbus_ac[ng] = branch_ac[ng].col(1).cast<int>();
        pd_ac[ng] = bus_ac[ng].col(2) / baseMVA_ac;
        qd_ac[ng] = bus_ac[ng].col(3) / baseMVA_ac;
    }
}

void PowerFlow::load_params_dc(const std::string& dcgrid_name, const std::unordered_map<std::string, Eigen::MatrixXd>& dataOPF) {
    if (!dcgrid_name.empty()) {
        // Load DC grid from file
        network_dc = create_dc(dcgrid_name);
        baseMW_dc = network_dc["baseMW"](0, 0);
        pol_dc = network_dc["pol"](0, 0);
        bus_dc = network_dc["bus"];
        branch_dc = network_dc["branch"];
        conv_dc = network_dc["converter"];
    }
    else {
        // Load DC grid from OPF dictionary
        baseMW_dc = 100.0;  // Default base
        pol_dc = 2.0;
        bus_dc = dataOPF.at("busDC");
        branch_dc = dataOPF.at("branchDC");
        conv_dc = dataOPF.at("converter");
    }

    basekV_dc = conv_dc.col(13);

    // Sizes
    nbuses_dc = bus_dc.rows();
    nbranches_dc = branch_dc.rows();
    nconvs_dc = conv_dc.rows();

    fbus_dc = branch_dc.col(0).cast<int>();
    tbus_dc = branch_dc.col(1).cast<int>();

    Y_dc = makeYbus(baseMW_dc, bus_dc, branch_dc);
    y_dc = absoluteSparseMatrix(Y_dc);

    rtf_dc = conv_dc.col(8);
    xtf_dc = conv_dc.col(9);
    bf_dc = conv_dc.col(10);
    rc_dc = conv_dc.col(11);
    xc_dc = conv_dc.col(12);

    ztfc_dc = (rtf_dc + rc_dc).array().cast<std::complex<double>>()
        + std::complex<double>(0.0, 1.0) * (xtf_dc + xc_dc).array().cast<std::complex<double>>();

    gtfc_dc = ztfc_dc.array().inverse().real();
    btfc_dc = ztfc_dc.array().inverse().imag();

    closs_dc = Eigen::VectorXd::Zero(nconvs_dc);
    convState_dc = Eigen::VectorXi::Zero(nconvs_dc);

    for (int i = 0; i < nconvs_dc; ++i) {
        if (conv_dc(i, 5) >= 0) {
            closs_dc(i) = conv_dc(i, 21);
            convState_dc(i) = 1;
        }
        else {
            closs_dc(i) = conv_dc(i, 20);
            convState_dc(i) = 0;
        }
    }

    aloss_dc = conv_dc.col(18) / baseMW_dc;
    bloss_dc = conv_dc.col(19).array() / basekV_dc.array();
    closs_dc = closs_dc.array() / (basekV_dc.array().square() / baseMW_dc);
}
