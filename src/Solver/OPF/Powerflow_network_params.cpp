#include "powerflow.h"
#include "../Helper_Functions/Helper_Functions.h"

#include "../../network.h"
#include "../../Bus.h"
#include "../../Include_components.h"
#include "../../Elements/Converter/MMC.h"

#include <map>
#include <unordered_map>
#include <vector>
#include <iostream>


static void
reNumberBusAC(Eigen::MatrixXd& busAC,
    Eigen::MatrixXd& branchAC,
    Eigen::MatrixXd& genAC,
    Eigen::MatrixXd& gencostAC,
    Eigen::MatrixXd& convDC);

static void extendBusAC(std::map<std::string,
    std::map<std::string, std::map<std::string, double>>>& data,
    Network* net,
    std::map<std::string, double>& global_params);

static void extendBranchAC(std::map<std::string,
    std::map<std::string, std::map<std::string, double>>>& data,
    Network* net,
    std::map<std::string, double>& global_params);

static void extendGenAC(
std::map<std::string,
    std::map<std::string, std::map<std::string, double>>>& data,
    Network* net,
    std::map<std::string, double>& global_params);

void PowerFlow::addBusAC(std::vector<std::vector<std::string>>& dict_ac,
    Bus* bus, std::map<std::string, double>& global_params, bool print_info /* =false*/)
{
    int id = static_cast<int>(dict_ac.size()) + 1;

    std::string bus_id = std::to_string(id);
    std::string bus_name = bus->getBusName(); 

    std::string row = std::to_string(id - 1);
    auto& busRow = data["busAC"][row];
    busRow["bus_i"] = id;
    busRow["type"] = 1.0;
    busRow["Pd"] = 0.0;
    busRow["Qd"] = 0.0;
    busRow["Gs"] = 0.0;
    busRow["Bs"] = 0.0;
	// busRow["area"] = 1; // Default, but bus can overwrite it
	busRow["Vm"] = 1.0; // Default, but bus can overwrite it
    busRow["Va"] = 0.0;
    busRow["baseKV"] = global_params["ACbaseKV"];
    busRow["zone"] = 1.0;
    busRow["Vmax"] = 1.1; // Default, but bus can overwrite it    
	busRow["Vmin"] = 0.9; // Default, but bus can overwrite it
    // busRow["grid"] = 1; // Default, but bus can overwrite it

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
        throw std::runtime_error("[addBusAC] Error: Duplicate entry for bus '" + bus_name + "'.");

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
    // busDCRow["area"] = 1.0;
    busDCRow["Vm"] = 1.0;
    busDCRow["Va"] = 0.0;
    busDCRow["baseKV"] = global_params["DCbaseKV"];
    busDCRow["zone"] = 1.0 ;
    busDCRow["Vmax"] = 1.1 * global_params["DCbaseKV"] / global_params["ACbaseKV"];
    busDCRow["Vmin"] = 0.9 * global_params["DCbaseKV"] / global_params["ACbaseKV"];

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
    // brRow["grid"] = std::stod(area_id);

    element->computePowerFlow(data["branchAC"][row], global_params);

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

    element->computePowerFlow(data["branchDC"][row], global_params);

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
    std::string rowKey = std::to_string(data["conv"].size());
    auto& convRow = data["conv"][rowKey];

	// Initialize the converter row with default values
    constexpr const char* keys[20] = {
     "gridac","type_dc","type_ac",
     "rtf","xtf","bf","rc","xc",
     "basekVac","Vmmax","Vmmin","Imax",
     "LossA","LossB","LossCrec","LossCinv",
     "droop","Pdcset","Vdcset","dvdcset"
    };
    for (const auto& key : keys) {
        convRow[key] = 0.0; // Initialize all keys to 0.0
	}

	// convRow["gridac"] = 1; // Default grid area for AC
	convRow["type_dc"] = 1; // Default type for DC
	convRow["type_ac"] = 1; // Default type for AC
	convRow["rtf"] = 0.0015; // Default rftc
	convRow["xtf"] = 0.1121; // Default xtfc
	convRow["bf"] = 0.0887; // Default bf
	convRow["rc"] = 0.0001; // Default rc
	convRow["xc"] = 0.16428; // Default xc
	convRow["basekVac"] = global_params["ACbaseKV"]; // Base voltage for AC
	convRow["Vmmax"] = 1.1; // Default maximum voltage
	convRow["Vmmin"] = 0.9; // Default minimum voltage
	convRow["Imax"] = 1.2; // Default maximum current
	convRow["LossA"] = 1.103; // Default LossAC
	convRow["LossB"] = 0.887; // Default LossB
	convRow["LossCrec"] = 2.885; // Default LossCrec
	convRow["LossCinv"] = 4.371; // Default LossCinv
	convRow["droop"] = 0.0; // Default droop
	convRow["Pdcset"] = 0.0; // Default Pdcset
	convRow["Vdcset"] = 0.0; // Default Vdcset
	convRow["dvdcset"] = 0.0; // Default Dvdsetc
	convRow["busdc_i"] = 0; // Placeholder for DC bus index
	convRow["busac_i"] = 0; // Placeholder for AC bus index
	convRow["P_g"] = 0.0; // Default active power generation
	convRow["Q_g"] = 0.0; // Default reactive power generation
	convRow["Vtar"] = 1.0; // Default target voltage
	convRow["status"] = 1.0; // Default status (1 = active, 0 = inactive)
	// Note: The keys in convRow should match the expected keys in the OPF data structure

	element->computePowerFlow(convRow, global_params);

    std::vector<Bus*> ends = element->getBuses();

    if (ends.size() != 2)
        throw std::runtime_error("[make_Converter] Error: Converter must connect exactly 2 buses");

    Bus* acBus = nullptr;
    Bus* dcBus = nullptr;

    for (Bus* b : element->getBuses())
        (b->getPinNumber() == 3 ? acBus : dcBus) = b;

    if (!acBus || !dcBus)
        throw std::runtime_error("[make_Converter] Error: Need one AC-bus and one DC-bus");

    int busac_i = busName2Id_.at(acBus->getBusName());
    int busdc_i = busName2Id_.at(dcBus->getBusName());

    convRow["busdc_i"] = busdc_i;
    convRow["busac_i"] = busac_i;

    conv_point.push_back(element);

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

    std::string rowCost = std::to_string(data["genCostAC"].size());
    auto& cRow = data["genCostAC"][rowCost];
    std::vector<std::string> cost_keys = {
        "model","startup","shutdown","n","c2","c1","c0","grid"
	};
    for (const auto& key : cost_keys) {
        cRow[key] = 0; // Empty structure for each
    }

	element->computePowerFlow(gRow, global_params); // Sets grid and area

    gRow["bus"] = bus_id;
    gRow["Vg"] = 1.0;
    gRow["mBase"] = 100.0;
    gRow["status"] = 1.0;
    // gRow["grid"] = 1; // Default area ID, can be changed based on your logic

    cRow["model"] = 2.0;
    cRow["startup"] = 1500.0;
    cRow["shutdown"] = 0.0;
    cRow["n"] = 3.0;   
	cRow["grid"] = gRow["grid"]; // Default, can be changed based on your logic

	// Here is easier to get the data from the element
	map<string, double> gen_info = element->getOPFInfo();

    //   for (const auto& [key, value] : gen_info) {
    //       if (gRow.find(key) == gRow.end()) {
	//		cRow[key] = value; // Fill in the cost data
    //       }
    //       else
	//	    gRow[key] = value; // Fill in the generator data
	//   }
	//  cRow["grid"] = gRow["grid"]; // Ensure grid is consistent

    for (const auto& [key, value] : gen_info) {
        if (key == "c0" || key == "c1" || key == "c2") {
            cRow[key] = value;  
        }
        else {
            gRow[key] = value;   
        }
    }

    // If gen_info contains "Vg", update corresponding bus Vmax/Vmin and fixed it to Vg
    auto it_vg = gen_info.find("Vg");
    if (it_vg != gen_info.end()) {
        double Vg_value = it_vg->second;  // e.g., 345 (in kV)
        double base_kv = global_params["ACbaseKV"];
        double Vg_pu = Vg_value / base_kv;

        std::string bus_row_key = std::to_string(bus_id - 1);
        if (data["busAC"].count(bus_row_key)) {
            auto& busRow = data["busAC"][bus_row_key];
            busRow["Vmax"] = Vg_pu;
            busRow["Vmin"] = Vg_pu;
            std::cout << "[make_Generator] Bus " << bus_name
                << " voltage limits updated by gen '" << element->getElementSymbol()
                << "': Vmax = Vmin = " << Vg_pu << " pu" << std::endl;
        }
    }

    // If gen_info contains "Ref", update corresponding bus type to "3"
    auto it_ref = gen_info.find("Ref");

    std::string bus_row_key = std::to_string(bus_id - 1);
    if (data["busAC"].count(bus_row_key)) {
        auto& busRow = data["busAC"][bus_row_key];

        if (it_ref != gen_info.end() && it_ref->second == 1.0) {
            busRow["type"] = 3.0;
            std::cout << "[make_Generator] Bus " << bus_name
                << " set as SLACK (type = 3) by generator '"
                << element->getElementSymbol() << "'\n";
        }
        else {
            busRow["type"] = 2.0;
            std::cout << "[make_Generator] Bus " << bus_name
                << " set as PV (type = 2) by generator '"
                << element->getElementSymbol() << "'\n";
        }
    }

	const std::string& gen_name = element->getElementSymbol();
    const std::string& area_id = to_string(gRow["area"]);
    const std::string& pmax_s = to_string(gRow["Pmax"]);
    const std::string& pmin_s = to_string(gRow["Pmin"]);
    const std::string& qmax_s = to_string(gRow["Qmax"]);
    const std::string& qmin_s = to_string(gRow["Qmax"]);

    double Pmax = std::stod(pmax_s), Pmin = std::stod(pmin_s);
    double Qmax = std::stod(qmax_s), Qmin = std::stod(qmin_s); 

    if (Pmin > Pmax) {
        throw std::invalid_argument("[make_Generator] Error: P_min > P_max for generator " + gen_name);
    }

    if (Qmin > Qmax) {
        throw std::invalid_argument("[make_Generator] Error: Q_min > Q_max for generator " + gen_name);
    }

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
    Bus* attachedBus = nullptr;

    for (Bus* b : element->getBuses()) {
        if (b->getBusName() != "gnd")
            attachedBus = b;
    }

    if (!attachedBus)
        throw std::runtime_error(
            "[make_Load] Error: load element is not connected to any bus");

    const std::string& bus_name = attachedBus->getBusName();  
    int bus_id = busName2Id_.at(bus_name);        
    std::string row = std::to_string(bus_id - 1);   

    //auto& busRow = data["busAC"][row];
	//busRow["bus_i"] = busName2Id_[bus_name];
	//busRow["type"] = 1; // Load type
	//busRow["Vm"] = global_params["basekV"]; // Voltage magnitude in kV
	//busRow["Va"] = 0.0; // Voltage angle in degrees
	//busRow["baseKV"] = global_params["ACbasekV"]; // Base voltage in kV
	//busRow["zone"] = 1.0; // Default zone, can be changed based on your logic
	//busRow["Vmax"] = 1.1; // Maximum voltage in pu
	//busRow["Vmin"] = 0.9; // Minimum voltage in pu

    if (!data["busAC"].count(row))
        throw std::runtime_error("[make_Load] AC bus not found)");

    auto& busRow = data["busAC"][row];

	element->computePowerFlow(busRow, global_params);

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


void PowerFlow::make_OPF(Network* net, std::map<std::string, double>& global_params, bool vscControl,
    bool writeTxt, bool plotResult, bool print_info)
{
    //// Define and initialize data map
    //std::map<std::string, std::map<std::string, std::map<std::string, double>>> data;

    // Initialize specific elements of the data map
   
    data["source_type"]["matpower"]["0"] = 0;
    data["name"]["network"]["0"] = 0;
    data["source_version"]["0.0.0"]["0"] = 0;
    data["per_unit"]["true"]["0"] = 1;
    data["dcpol"]["2"]["0"] = 2;
    data["baseMVA"]["100"]["0"] = 100;


    // Initialize empty elements of the data map
    std::vector<std::string> keys = {
        "bus", "busdc", "shunt", "dcline", "storage", "switch",
        "load", "branch", "branchDC", "gen", "convdc", "res"
    };

    for (const auto& key : keys) {
        data[key] = {}; // Empty structure for each
    }

    // Define and initialize data map

    cout << "\n[make_OPF] Start making OPF data...\n";

	// Process buses
    // Define data structures
    //  std::vector<std::vector<std::string>> dict_ac;
    //  std::vector<std::vector<std::string>> dict_dc;
    //  for (const auto& [bus_name, bus] : net->getBuses())
    //  {
    //    cout << "[make_OPF] Processing bus: " << bus_name << endl;
    //      if (bus->getPinNumber() == 3) {
    //          addBusAC(dict_ac, bus, global_dict, writeTxt);
    //      }
    //      else if (bus->getPinNumber() == 1) {
    //          addBusDC(dict_dc, bus, global_dict, writeTxt);
    //      }
    //      else {
    //          throw std::runtime_error("[make_OPF] Error: Unsupported bus type.");
    //      }
    //  }

    std::vector<std::vector<std::string>> dict_ac;
    std::vector<std::vector<std::string>> dict_dc;

    std::vector<Bus*> acBuses, dcBuses;
    for (auto& kv : net->getBuses()) {
        Bus* b = kv.second;
        (b->getPinNumber() == 3 ? acBuses : dcBuses).push_back(b);
    }

    auto byName = [](Bus* a, Bus* b) { return a->getBusName() < b->getBusName(); };
    std::sort(acBuses.begin(), acBuses.end(), byName);
    std::sort(dcBuses.begin(), dcBuses.end(), byName);

    for (Bus* b : acBuses)
        addBusAC(dict_ac, b, global_params, print_info);

    for (Bus* b : dcBuses)
        addBusDC(dict_dc, b, global_params, print_info);

	// Process elements: loads, generators, which contribute to the buses data    
    auto& elements = net->getElements();
    for (const auto& [element_name, element] : elements)
    {
        if (dynamic_cast<Load*>(element) || dynamic_cast<LoadPQ*>(element)) {
            cout << "[make_OPF] Processing element: " << element_name << endl;
            make_Load(element, global_params, print_info);
        }
        else if (dynamic_cast<Source_base*>(element)) {
            cout << "[make_OPF] Processing element: " << element_name << endl;
            make_Generator(element, global_params, print_info);
        }
    }

	// Process branches: AC and DC branches, i.e., transmission lines, impedances, etc.
    for (const auto& [element_name, element] : elements)
    {
        if (dynamic_cast<Impedance*>(element)) {
            if (element->getInputPins() == 3) {
                make_BranchAC(element, global_params, print_info);
            }
            else if (element->getInputPins() == 1) {
                make_BranchDC(element, global_params, print_info);
            }
            else {
                throw std::runtime_error("[make_OPF] Error: Unsupported impedance pin number.");
            }

        }
        else if (dynamic_cast<MMC*>(element)) {
            make_Converter(element, global_params, print_info);
        }
        else {
        }
	}

    extendBusAC(data, net, global_params);
    extendBranchAC(data, net, global_params);
    extendGenAC(data, net, global_params);

	cout << "[make_OPF] Finished processing elements.\n";

    /* 2. Transform to Eigen::MatrixXd */
        // Initialize specific elements of the data map
    MatrixXd busDC = map2dense(data.at("busDC"),
        { "bus_i","type","Pd","Qd","Gs","Bs","area",
          "Vm","Va","baseKV","zone","Vmax","Vmin" });


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

    reNumberBusAC(busAC, branchAC, genAC, gencostAC, convDC);

    // for debug
    Eigen::MatrixXd resAC;

    if (data.find("resAC") != data.end()) {
        resAC = map2dense(data.at("resAC"),
            { "bus","Presmax","Sresmax","model","startup","shutdown","n","c2","c1","c0","grid" });
    }
    else {
        // fallback to default
        //resAC = Eigen::MatrixXd::Zero(1, 12);
        //resAC(0, 11) = 1;
        //resAC(0, 0) = 1;
    }

    cout << "[make_OPF] Finished making OPF data.\n";

    /* 3. Put inside unordered_map<string, MatrixXd> */
    std::unordered_map<std::string, MatrixXd> dataOPF;
    dataOPF["busDC"] = std::move(busDC);
    dataOPF["branchDC"] = std::move(branchDC);
    dataOPF["converter"] = std::move(convDC);

    dataOPF["busAC"] = std::move(busAC);
    dataOPF["branchAC"] = std::move(branchAC);
    dataOPF["generator"] = std::move(genAC);
    dataOPF["gencost"] = std::move(gencostAC);
    dataOPF["res"] = std::move(resAC);

	cout << "[make_OPF] Finished transforming data to Eigen::MatrixXd.\n";

    //solveHmo_opf(dataOPF, vscControl, writeTxt, plotResult);
    solve_opf("", "", &dataOPF, vscControl, writeTxt, plotResult, print_info);

    
    // Update each MMC element with OPF results
    if (!conv_point.empty()) {
        std::cout << "\n=== Updating " << conv_point.size()
            << " MMC elements with OPF results ===" << std::endl;

        for (size_t i = 0; i < conv_point.size(); ++i) {
            Element* elem = conv_point[i];
            if (!elem) continue;

            auto* mmc = dynamic_cast<MMC*>(elem);
            if (!mmc) continue;

            // Retrieve OPF results
            double Vm_kV = std::sqrt(v2s_dc_k(i)) * global_params["ACbaseKV"];
            double theta_deg = theta_s_k(i)/ M_PI * 180;
            double Pac_MW = ps_dc_k(i) * global_params["baseMVA"];
            double Qac_MVar = qs_dc_k(i) * global_params["baseMVA"];
            double Vdc_kV = std::sqrt(vn2_dc_k(i)) * global_params["ACbaseKV"];
            double Pdc_MW = pn_dc_k(i) * global_params["baseMVA"];

            // Convert units
            double Vm_V = Vm_kV * 1e3;
            double theta_rad = theta_deg * M_PI / 180.0;
            double Pac_W = Pac_MW * 1e6;
            double Qac_Var = Qac_MVar * 1e6;
            double Vdc_V = Vdc_kV * 1e3;
            double Pdc_W = Pdc_MW * 1e6;

            // Update the MMC
            mmc->update_MMC(Vm_V, theta_rad, Pac_W, Qac_Var, Vdc_V, Pdc_W);

                std::cout << "[Updated MMC] " << elem->getElementSymbol()
                    << " | Vm=" << Vm_kV << " kV, θ=" << theta_deg
                    << "°, Pac=" << Pac_MW << " MW, Qac=" << Qac_MVar
                    << " MVar, Vdc=" << Vdc_kV << " kV, Pdc=" << Pdc_MW
                    << " MW" << std::endl;
        }
    }

}

/// ========================================== debug start
static void
reNumberBusAC(
    Eigen::MatrixXd& busAC,
    Eigen::MatrixXd& branchAC,
    Eigen::MatrixXd& genAC,
    Eigen::MatrixXd& gencostAC,
    Eigen::MatrixXd& convDC)  
{
    std::unordered_map<int, std::unordered_map<int, int>> newIds; 
    std::unordered_map<int, int> counters; 

    for (int i = 0; i < busAC.rows(); ++i) {
        int old_id = static_cast<int>(busAC(i, 0));
        int region = static_cast<int>(busAC(i, busAC.cols() - 1));
        int new_id = ++counters[region];
        newIds[region][old_id] = new_id;
    }

    for (int i = 0; i < busAC.rows(); ++i) {
        int old_id = static_cast<int>(busAC(i, 0));
        int region = static_cast<int>(busAC(i, busAC.cols() - 1));
        busAC(i, 0) = newIds[region][old_id];
    }

    for (int i = 0; i < branchAC.rows(); ++i) {
        int fbus = static_cast<int>(branchAC(i, 0));
        int tbus = static_cast<int>(branchAC(i, 1));
        int region = static_cast<int>(branchAC(i, branchAC.cols() - 1));
        if (newIds.count(region) && newIds[region].count(fbus))
            branchAC(i, 0) = newIds[region][fbus];
        if (newIds.count(region) && newIds[region].count(tbus))
            branchAC(i, 1) = newIds[region][tbus];
    }

    for (int i = 0; i < genAC.rows(); ++i) {
        int bus = static_cast<int>(genAC(i, 0));
        int region = static_cast<int>(genAC(i, genAC.cols() - 1));
        if (newIds.count(region) && newIds[region].count(bus))
            genAC(i, 0) = newIds[region][bus];
    }

    for (int i = 0; i < convDC.rows(); ++i) {
        int oldBusAC = static_cast<int>(convDC(i, 1));  
        int region = static_cast<int>(convDC(i, 2));  
        if (newIds.count(region) && newIds[region].count(oldBusAC)) {
            convDC(i, 1) = newIds[region][oldBusAC];
        }

    }

}

static void extendBusAC(
    std::map<std::string,
    std::map<std::string, std::map<std::string, double>>>& data,
    Network* net,
    std::map<std::string, double>& global_params)
{
    // get the maximum bus_i according to grid area
    std::unordered_map<int, int> max_bus_by_grid;
    for (auto& [rowKey, row] : data["busAC"]) {
        int g = static_cast<int>(row["grid"]);
        int b = static_cast<int>(row["bus_i"]);
        max_bus_by_grid[g] = std::max(max_bus_by_grid[g], b);
    }

    // When AC_source is connected, add a new bus
    for (auto& kv : net->getElements()) {
        Element* elem = kv.second;
        auto* src = dynamic_cast<AC_source*>(elem);
        if (!src) continue;

        // find the connected bus with AC_source（terminal==1）
        Bus* bus1 = nullptr;
        for (auto& cx : elem->getConnections()) {
            if (cx.second == 1) { bus1 = cx.first; break; }
        }
        if (!bus1) continue;

        // get order form bus name
        std::string bus_name = bus1->getBusName();
        int bus1_id = -1;
        {
            std::smatch m;
            if (std::regex_search(bus_name, m, std::regex(R"((\d+)$)")))
                bus1_id = std::stoi(m[1].str());
        }
        if (bus1_id < 0) continue;

        // find grid that the bus belong
        int grid = -1;
        for (auto& [k, row] : data["busAC"]) {
            if (static_cast<int>(row["bus_i"]) == bus1_id) {
                grid = static_cast<int>(row["grid"]);
                break;
            }
        }
        if (grid < 0) continue;

        // create new bus_id
        int new_bus_id = ++max_bus_by_grid[grid];

        std::string new_row_key = std::to_string(static_cast<int>(data["busAC"].size()));
        auto& newRow = data["busAC"][new_row_key];
        newRow["bus_i"] = new_bus_id;
        newRow["type"] = 1.0;
        newRow["Pd"] = 0.0;
        newRow["Qd"] = 0.0;
        newRow["Gs"] = 0.0;
        newRow["Bs"] = 0.0;
        newRow["area"] = grid;
        newRow["Vm"] = 1.0;
        newRow["Va"] = 0.0;
        newRow["baseKV"] = global_params["ACbaseKV"];
        newRow["zone"] = 1.0;
        newRow["Vmax"] = 1.1;
        newRow["Vmin"] = 0.9;
        newRow["grid"] = grid;

        double zsrc = 0.0;
        auto info = src->getOPFInfo();
        if (info.count("Zsrc")) zsrc = info["Zsrc"];

        std::string metaKey = std::to_string(static_cast<int>(data["acsrcMeta"].size()));
        auto& meta = data["acsrcMeta"][metaKey];
        meta["bus1_id"] = bus1_id;
        meta["new_bus_id"] = new_bus_id;
        meta["grid"] = grid;
        meta["Zsrc"] = zsrc;
    }
}

static void extendBranchAC(
    std::map<std::string,
    std::map<std::string, std::map<std::string, double>>>& data,
    Network* net,
    std::map<std::string, double>& global_params)
{
    // If no AC_source connected, return
    if (data.find("acsrcMeta") == data.end()) return;

    for (auto& [k, meta] : data["acsrcMeta"]) {
        int fbus = static_cast<int>(meta["bus1_id"]);
        int tbus = static_cast<int>(meta["new_bus_id"]);
        int grid = static_cast<int>(meta["grid"]);
        double zsrc = meta["Zsrc"];

        std::string brKey = std::to_string(static_cast<int>(data["branchAC"].size()));
        auto& br = data["branchAC"][brKey];

        br["fbus"] = fbus;
        br["tbus"] = tbus;
        br["r"] = 0.0;
        br["x"] = zsrc/global_params["Z_base"];
        br["b"] = 0.0;
        br["rateAC"] = 100.0;
        br["rateB"] = 100.0;
        br["rateC"] = 100.0;
        br["ratio"] = 0.0;
        br["angle"] = 0.0;
        br["status"] = 1.0;
        br["angmin"] = -360.0;
        br["angmax"] = 360.0;
        br["grid"] = grid;
    }

}

static void extendGenAC(
    std::map<std::string,
    std::map<std::string, std::map<std::string, double>>>& data,
    Network* net,
    std::map<std::string, double>& global_params)
{
    // Iterate over all AC_source
    for (auto& elem_pair : net->getElements()) {
        Element* elem = elem_pair.second;
        AC_source* src = dynamic_cast<AC_source*>(elem);
        if (!src) continue;

        // find the bus that AC_source connected with
        Bus* connectedBus = nullptr;
        for (auto& kv : elem->getConnections()) {
            if (kv.second == 1) { connectedBus = kv.first; break; }
        }
        if (!connectedBus) continue;

        std::string bus_name = connectedBus->getBusName();
        int old_bus_id = -1;
        int grid = -1;

        for (auto& [key, busRow] : data["busAC"]) {
            int id = static_cast<int>(busRow.at("bus_i"));
            if (std::abs(id - std::stoi(bus_name.substr(bus_name.size() - 1))) < 1e-6) {
                old_bus_id = id;
                grid = static_cast<int>(busRow.at("grid"));
                break;
            }
        }
        if (grid == -1) continue;

        // find the new added bus order in the grid
        int max_bus_in_grid = 0;
        for (auto& [key, busRow] : data["busAC"]) {
            if (static_cast<int>(busRow.at("grid")) == grid) {
                int id = static_cast<int>(busRow.at("bus_i"));
                if (id > max_bus_in_grid) max_bus_in_grid = id;
            }
        }

        // Mapping oldBus -> newBus
        int new_bus_id = max_bus_in_grid;

        for (auto& [key, genRow] : data["genAC"]) {
            int gen_bus = static_cast<int>(genRow.at("bus"));
            int gen_grid = static_cast<int>(genRow.at("grid"));
            if (gen_bus == old_bus_id && gen_grid == grid) {
                genRow["bus"] = new_bus_id;
            }
        }
    }
}


/// ========================================== debug end

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
        res_entire_ac = network_ac["res"];
    }
    else {
        // From OPF input map
        baseMVA_ac = 100.0;  // or dataOPF["baseMVA"](0, 0) if available
        bus_entire_ac = dataOPF.at("busAC");
        branch_entire_ac = dataOPF.at("branchAC");
        gen_entire_ac = dataOPF.at("generator");
        gencost_entire_ac = dataOPF.at("gencost");
        res_entire_ac = dataOPF.at("res");
    }

    /// debug
    Eigen::IOFormat fmt(Eigen::StreamPrecision, 0, ", ", "\n", "[", "]");

    std::cout << "\n=== baseMVA_ac ===\n";
    std::cout << baseMVA_ac << "\n";

    std::cout << "\n=== bus_entire_ac (" << bus_entire_ac.rows()
        << " x " << bus_entire_ac.cols() << ") ===\n";
    std::cout << bus_entire_ac.format(fmt) << "\n";

    std::cout << "\n=== branch_entire_ac (" << branch_entire_ac.rows()
        << " x " << branch_entire_ac.cols() << ") ===\n";
    std::cout << branch_entire_ac.format(fmt) << "\n";

    std::cout << "\n=== gen_entire_ac (" << gen_entire_ac.rows()
        << " x " << gen_entire_ac.cols() << ") ===\n";
    std::cout << gen_entire_ac.format(fmt) << "\n";

    std::cout << "\n=== gencost_entire_ac (" << gencost_entire_ac.rows()
        << " x " << gencost_entire_ac.cols() << ") ===\n";
    std::cout << gencost_entire_ac.format(fmt) << "\n";

    std::cout << "\n=== res_entire_ac (" << res_entire_ac.rows()
        << " x " << res_entire_ac.cols() << ") ===\n";
    std::cout << res_entire_ac.format(fmt) << "\n";

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
    res_ac.resize(ngrids);
    pd_ac.resize(ngrids);
    qd_ac.resize(ngrids);
    sres_ac.resize(ngrids);
    nbuses_ac.resize(ngrids);
    nbranches_ac.resize(ngrids);
    ngens_ac.resize(ngrids);
    nress_ac.resize(ngrids);
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
        std::vector<int> bus_rows, branch_rows, gen_rows, gencost_rows, res_rows;

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

        // Filter RES data
        for (int i = 0; i < res_entire_ac.rows(); ++i) {
            if (static_cast<int>(res_entire_ac(i, 11)) == ng + 1) {
                res_rows.push_back(i);
            }
        }
        res_ac[ng] = Eigen::MatrixXd(res_rows.size(), res_entire_ac.cols());
        for (size_t i = 0; i < res_rows.size(); ++i) {
            res_ac[ng].row(i) = res_entire_ac.row(res_rows[i]);
        }

        //Local index and references
        nbuses_ac[ng] = static_cast<int>(bus_ac[ng].rows());
        nbranches_ac[ng] = static_cast<int>(branch_ac[ng].rows());
        ngens_ac[ng] = static_cast<int>(generator_ac[ng].rows());
        nress_ac[ng] = static_cast<int>(res_ac[ng].rows());

        IDtoCountmap[ng] = Eigen::VectorXi::Zero(nbuses_ac[ng]);
        for (int i = 0; i < nbuses_ac[ng]; ++i) {
            int bus_id = static_cast<int>(bus_ac[ng](i, 0));
            IDtoCountmap[ng](bus_id - 1) = i;
            if (bus_ac[ng](i, 1) == 3) {
                refbuscount_ac[ng] = i+1;
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

        // Normalize RES capacity
        sres_ac[ng] = res_ac[ng].col(2) / baseMVA_ac;

        //// mapping from "busname" to "id"

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

    /// debug
    Eigen::IOFormat fmt(Eigen::StreamPrecision, 0, ", ", "\n", "[", "]");

    std::cout << "\n=== baseMW_dc ===\n";
    std::cout << baseMW_dc << "\n";

    std::cout << "\n=== pol_dc ===\n";
    std::cout << pol_dc << "\n";

    std::cout << "\n=== bus_dc (" << bus_dc.rows()
        << " x " << bus_dc.cols() << ") ===\n";
    std::cout << bus_dc.format(fmt) << "\n";

    std::cout << "\n=== branch_dc (" << branch_dc.rows()
        << " x " << branch_dc.cols() << ") ===\n";
    std::cout << branch_dc.format(fmt) << "\n";

    std::cout << "\n=== conv_dc (" << conv_dc.rows()
        << " x " << conv_dc.cols() << ") ===\n";
    std::cout << conv_dc.format(fmt) << "\n";

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
