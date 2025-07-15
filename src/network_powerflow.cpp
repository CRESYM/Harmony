#include "network.h"
#include "Bus.h"
#include "Include_components.h"
#include "solveHmo_opf.h"



template<typename Table>
Eigen::MatrixXd map2dense(const Table& tbl,
    const std::vector<std::string>& colNames)
{
    const int nRow = static_cast<int>(tbl.size());
    const int nCol = static_cast<int>(colNames.size());
    Eigen::MatrixXd M(nRow, nCol);

    for (const auto& [rowKey, colMap] : tbl)
    {
        int r = std::stoi(rowKey);
        for (int c = 0; c < nCol; ++c)
        {
            auto it = colMap.find(colNames[c]);
            M(r, c) = (it != colMap.end()) ? it->second : 0.0;
        }
    }
    return M;
}


void Network::addBusAC(std::vector<std::vector<std::string>>& dict_ac,
    const std::vector<std::string>& bus_info,
    bool print_info /* =false*/)
{
    int id = static_cast<int>(dict_ac.size()) + 1;

    std::string bus_id = std::to_string(id);
    std::string bus_name = bus_info[0];
    std::string area_id = bus_info[1];
    std::string base_mw_str = bus_info[2];
    std::string rated_kv_str = bus_info[3];
    std::string upper_str = bus_info[4];
    std::string lower_str = bus_info[5];

    double rated_kv = std::stod(rated_kv_str);
    double v_upper = std::stod(upper_str);
    double v_lower = std::stod(lower_str);

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
        std::cerr << "[addBusAC] Error: voltage_lower (" << v_lower_pu
            << " pu) is greater than voltage_upper (" << v_upper_pu << " pu) for bus "
            << bus_name << ".\n";
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

    std::string row = std::to_string(id - 1);
    auto& busRow = data["busAC"][row];
    busRow["bus_i"] = id;
    busRow["type"] = 1.0;
    busRow["Pd"] = 0.0;
    busRow["Qd"] = 0.0;
    busRow["Gs"] = 0.0;
    busRow["Bs"] = 0.0;
    busRow["area"] = 1.0;
    busRow["Vm"] = 1.0;
    busRow["Va"] = 0.0;
    busRow["baseKV"] = rated_kv;
    busRow["zone"] = 1.0;
    busRow["Vmax"] = v_upper_pu;
    busRow["Vmin"] = v_lower_pu;
    busRow["grid"] = std::stod(area_id);


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


void Network::addBusDC(std::vector<std::vector<std::string>>& dict_dc,
    const std::vector<std::string>& bus_info,
    bool  print_info /* =false*/)
{
    int id = static_cast<int>(dict_dc.size()) + 1;

    if (bus_info.size() != 4) {

        std::cerr << "[addBusAC] Error: Expected 4 fields:\n "
            << " [0] bus_name\n "
            << " [1] rated_voltage_kv [kV]\n "
            << " [2] voltage_upper [pu or kV]\n "
            << " [3] voltage_lower [pu or kV]\n "
            << " but received " << bus_info.size() << ".\n";

        std::cerr << "  Received: { ";
        for (const auto& s : bus_info) std::cerr << '"' << s << "\" ";
        std::cerr << "}\n";
    }

    std::string bus_id = std::to_string(id);
    std::string bus_name = bus_info[0];
    std::string rated_kv_str = bus_info[1];
    std::string upper_str = bus_info[2];
    std::string lower_str = bus_info[3];

    double rated_kv = std::stod(rated_kv_str);
    double v_upper = std::stod(upper_str);
    double v_lower = std::stod(lower_str);

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
    busDCRow["baseKV"] = rated_kv;
    busDCRow["zone"] = 1.0;
    busDCRow["Vmax"] = v_upper_pu;
    busDCRow["Vmin"] = v_lower_pu;

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


void Network::make_BranchAC(Element* element,
    std::map<std::string, double>& global_params,
    const std::vector<std::string>& br_info,
    bool print_info /* = false */)
{
    std::vector<Bus*> ends;
    for (auto& kv : connections)
        if (std::find(kv.second.begin(), kv.second.end(), element) != kv.second.end())
            ends.push_back(kv.first);

    if (ends.size() != 2)
        throw std::runtime_error("[make_BranchAC] Error: AC Branch should just connected with two AC Buses");

    element->computePowerFlowAC(data["branch"], global_params);
    if (data["branch"].empty()) {
        std::cerr << "[make_BranchAC] Error: [branch] table is empty.\n";
        return;
    }

    const std::string& fBus_name = ends[0]->getBusName();
    const std::string& tBus_name = ends[1]->getBusName();

    std::string row = std::to_string(data["branch"].size() - 1);
    double br_r_pu = data["branch"][row]["br_r"];
    double br_x_pu = data["branch"][row]["br_x"];

    if (br_info.size() != 3) {
        std::cerr << "[make_BranchAC] Error: Expected 3 fields:\n"
            << " [0] branch_name \n "
            << " [1] grid_area \n "
            << " [2] shunt [p.u.]\n"
            << " but received " << br_info.size() << ".\n";

        std::cerr << "  Received: { ";
        for (const auto& s : br_info) std::cerr << '"' << s << "\" ";
        std::cerr << "}\n";
    }

    std::string branch_name = br_info[0];
    std::string area_id = br_info[1];
    std::string br_b_pu = br_info[2];

    auto fIt = busName2Id_.find(fBus_name);
    auto tIt = busName2Id_.find(tBus_name);
    if (fIt == busName2Id_.end() || tIt == busName2Id_.end())
        throw std::runtime_error("[make_BranchAC] Error: One of the bus name unknown");

    int fId = fIt->second;
    int tId = tIt->second;

    /* ------- Write data["branchAC"] ------- */
    std::string rowKey = std::to_string(data["branchAC"].size());
    auto& brRow = data["branchAC"][rowKey];

    brRow["fbus"] = fId;
    brRow["tbus"] = tId;
    brRow["r"] = br_r_pu;
    brRow["x"] = br_x_pu;
    brRow["b"] = std::stod(br_b_pu);
    brRow["rateA"] = 100.0;
    brRow["rateB"] = 100.0;
    brRow["rateC"] = 100.0;
    brRow["ratio"] = 0.0;
    brRow["angle"] = 0.0;
    brRow["status"] = 1.0;
    brRow["angmin"] = -360.0;
    brRow["angmax"] = 360.0;
    brRow["grid"] = std::stod(area_id);

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

}


void Network::make_BranchDC(Element* element,
    std::map<std::string, double>& global_params,
    const std::vector<std::string>& br_info,
    bool print_info /* = false */)

{
    std::vector<Bus*> ends;
    for (auto& kv : connections)
        if (std::find(kv.second.begin(), kv.second.end(), element) != kv.second.end())
            ends.push_back(kv.first);

    if (ends.size() != 2)
        throw std::runtime_error("[make_BranchDC] Error: DC Branch should just connected with two DC Buses");


    element->computePowerFlowDC(data["branchdc"], global_params);
    if (data["branchdc"].empty()) {
        std::cerr << "[make_BranchDC] Error: [branchdc] table is empty.\n";
        return;
    }


    const std::string& fBus_name = ends[0]->getBusName();
    const std::string& tBus_name = ends[1]->getBusName();

    std::string row = std::to_string(data["branchdc"].size() - 1);
    double br_r_pu = data["branchdc"][row]["br_r"];
    double br_x_pu = data["branchdc"][row]["br_x"];


    if (br_info.size() != 1) {
        std::cerr << "[make_BranchDC] Error: Expected 1 fields:\n"
            << " [0] branch_name \n "
            << " but received " << br_info.size() << ".\n";

        std::cerr << "  Received: { ";
        for (const auto& s : br_info) std::cerr << '"' << s << "\" ";
        std::cerr << "}\n";
    }

    std::string branch_name = br_info[0];

    auto fIt = busName2Id_.find(fBus_name);
    auto tIt = busName2Id_.find(tBus_name);
    if (fIt == busName2Id_.end() || tIt == busName2Id_.end())
        throw std::runtime_error("[make_BranchAC] Error: One of the bus name unknown");

    int fId = fIt->second;
    int tId = tIt->second;

    /* ------- Write data["branchDC"] ------- */
    std::string rowKey = std::to_string(data["branchDC"].size());
    auto& brRow = data["branchDC"][rowKey];

    brRow["fbus"] = fId;
    brRow["tbus"] = tId;
    brRow["r"] = 0.052; // will revise later
    brRow["x"] = 0.0;
    brRow["b"] = 0.0;
    brRow["rateA"] = 100.0;
    brRow["rateB"] = 100.0;
    brRow["rateC"] = 100.0;
    brRow["ratio"] = 0.0;
    brRow["angle"] = 0.0;
    brRow["status"] = 1.0;
    brRow["angmin"] = -0.0;
    brRow["angmax"] = 0.0;

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

}

void Network::make_Converter(
    Element* element,
    std::map<std::string, double>& global_params,
    const std::vector<std::string>& conv_info,
    bool print_info /*=false*/)
{

    if (conv_info.size() != 21) {
        std::cerr << "[make_Converter] Error: Expected 21 fields:\n"
            << " [0] converter_name \n "
            << " [1] grid_area \n "
            << " [2] type_dc\n "
            << " [3] type_ac\n "
            << " [4] rftc\n "
            << " [5] xtfc\n "
            << " [6] bf \n "
            << " [7] rc \n "
            << " [8] xc \n "
            << " [9] basekVac \n "
            << " [10] Vmmax \n "
            << " [11] Vmmin \n "
            << " [12] Imax \n "
            << " [13] LossAC \n "
            << " [14] LossB \n "
            << " [15] LossCrec \n "
            << " [16] LossCinv \n "
            << " [17] droop \n "
            << " [18] Pdcset \n "
            << " [19] Vdcset \n "
            << " [20] Dvdcset \n"
            << " but received " << conv_info.size() << ".\n";

        std::cerr << "  Received: { ";
        for (const auto& s : conv_info) std::cerr << '"' << s << "\" ";
        std::cerr << "}\n";
    }

    std::vector<Bus*> ends;
    for (auto& kv : connections)
        if (std::find(kv.second.begin(), kv.second.end(), element) != kv.second.end())
            ends.push_back(kv.first);

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

    for (size_t i = 0; i < 20; ++i)
        convRow[keys[i]] = std::stod(conv_info[i + 1]);

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


void Network::make_Generator(
    Element* element,
    const std::vector<std::string>& gen_info,
    bool print_info /* = false */)
{

    if (gen_info.size() != 10)
        throw std::runtime_error("[make_Generator] Expect 10 fields, got "
            + std::to_string(gen_info.size()));

    const std::string& gen_name = gen_info[0];
    const std::string& area_id = gen_info[1];
    const std::string& pmax_s = gen_info[3];
    const std::string& pmin_s = gen_info[4];
    const std::string& qmax_s = gen_info[5];
    const std::string& qmin_s = gen_info[6];
    const std::string& a_s = gen_info[7];
    const std::string& b_s = gen_info[8];
    const std::string& c_s = gen_info[9];

    double Pmax = std::stod(pmax_s), Pmin = std::stod(pmin_s);
    double Qmax = std::stod(qmax_s), Qmin = std::stod(qmin_s);

    if (Pmin > Pmax) {
        throw std::invalid_argument("[make_Generator] Error: P_min > P_max for generator " + gen_name);
    }

    if (Qmin > Qmax) {
        throw std::invalid_argument("[make_Generator] Error: Q_min > Q_max for generator " + gen_name);
    }

    std::map<std::string, double> dummy;
    element->computePowerFlowAC(data["gen"], dummy);

    Bus* bus = nullptr;
    for (auto& kv : connections)
        if (std::find(kv.second.begin(), kv.second.end(), element) != kv.second.end())
        {
            if (bus) throw std::runtime_error("[make_Generator] Error: Multiple buses connected");
            bus = kv.first;
        }

    if (!bus) throw std::runtime_error("[make_Generator] Error: No bus connected");

    const std::string& bus_name = bus->getBusName();
    auto idIt = busName2Id_.find(bus_name);
    if (idIt == busName2Id_.end())
        throw std::runtime_error("[make_Generator] Error: Bus name unknown");
    int bus_id = idIt->second;

    /* ---------- Write data["genAC"] ---------- */
    std::string rowGen = std::to_string(data["genAC"].size());
    auto& gRow = data["genAC"][rowGen];

    gRow["bus"] = bus_id;
    gRow["Pg"] = 0.0;
    gRow["Qg"] = 0.0;
    gRow["Qmax"] = Qmax;
    gRow["Qmin"] = Qmin;
    gRow["Vg"] = 1.0;
    gRow["mBase"] = 100.0;
    gRow["status"] = 1.0;
    gRow["Pmax"] = Pmax;
    gRow["Pmin"] = Pmin;
    gRow["Pc1"] = 0.0;
    gRow["Pc2"] = 0.0;
    gRow["Qc1min"] = 0.0;
    gRow["Qc1max"] = 0.0;
    gRow["Qc2min"] = 0.0;
    gRow["Qc2max"] = 0.0;
    gRow["ramp_agc"] = 0.0;
    gRow["ramp_10"] = 0.0;
    gRow["ramp_30"] = 0.0;
    gRow["ramp_q"] = 0.0;
    gRow["apf"] = 0.0;
    gRow["grid"] = std::stod(area_id);

    /* ---------- Write data["genCostAC"] ---------- */
    std::string rowCost = std::to_string(data["genCostAC"].size());
    auto& cRow = data["genCostAC"][rowCost];

    cRow["model"] = 2.0;
    cRow["startup"] = 1500.0;
    cRow["shutdown"] = 0.0;
    cRow["n"] = 3.0;
    cRow["c2"] = std::stod(a_s);
    cRow["c1"] = std::stod(b_s);
    cRow["c0"] = std::stod(c_s);
    cRow["grid"] = std::stod(area_id);

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

void Network::make_Load(Element* element,
    const std::vector<std::string>& load_info,
    bool print_info /* = false */)
{
    std::map<std::string, double> dummy;
    element->computePowerFlowAC(data["load"], dummy);

    Bus* attachedBus = nullptr;
    for (const auto& kv : connections)
    {
        Bus* pBus = kv.first;
        const auto& vec = kv.second;
        if (std::find(vec.begin(), vec.end(), element) != vec.end())
        {
            if (attachedBus)
                throw std::runtime_error(
                    "[make_Load] Error: Load element connected to multiple buses");
            attachedBus = pBus;
        }
    }
    if (!attachedBus)
        throw std::runtime_error(
            "[make_Load] Error: load element is not connected to any bus");

    const std::string& bus_name = attachedBus->getBusName();

    if (load_info.size() != 6) {
        std::cerr << "[make_Load] Error: Expected 10 fields:\n"
            << " [0] load_name\n "
            << " [1] grid_area\n "
            << " [2] rated_voltage_kv [kV]\n "
            << " [3] R [Ω]\n "
            << " [4] L [H]\n "
            << " [5] C [F]\n "
            << " but received " << load_info.size() << ".\n ";

        std::cerr << "  Received: { ";
        for (const auto& s : load_info) std::cerr << '"' << s << "\" ";
        std::cerr << "}\n";

    }

    std::string r = std::to_string(data["load"].size() - 1);
    double br_r_pu = data["load"][r]["br_r"];
    double br_x_pu = data["load"][r]["br_x"];

    std::string load_name = load_info[0];
    std::string area_str = load_info[1];
    std::string v_kv_str = load_info[2];
    std::string R_str = load_info[3];
    std::string L_str = load_info[4];
    std::string C_str = load_info[5];

    double V_LL = std::stod(v_kv_str);
    double R = std::stod(R_str);
    double L = std::stod(L_str);
    double C = std::stod(C_str);

    if (R < 0 || L < 0 || C < 0)
        throw std::invalid_argument("[make_Error] Error: R, L, C must be non-negative");

    /* ---------- Calculate Pd + Qd ---------- */
    const double omega = 2 * M_PI * 50.0;
    double V_phase = V_LL * 1e3 / std::sqrt(3.0);

    double G = (R == 0.0) ? 0.0 : 1.0 / R;
    double B_L = (L == 0.0) ? 0.0 : -1.0 / (omega * L);
    double B_C = (C == 0.0) ? 0.0 : omega * C;
    double B = B_L + B_C;

    double Pd_MW = 3.0 * V_phase * V_phase * G / 1e6;
    double Qd_MVAr = -3.0 * V_phase * V_phase * B / 1e6;

    std::string row = std::to_string(data["load"].size());
    data["load"][row]["Pd"] = Pd_MW;
    data["load"][row]["Qd"] = Qd_MVAr;

    auto itBusId = busName2Id_.find(bus_name);
    if (itBusId == busName2Id_.end())
        throw std::runtime_error("[make_Load] Error: Bus name that load connected is not found ");

    std::string busRowKey = std::to_string(itBusId->second - 1);
    auto& busRow = data["busAC"][busRowKey];
    busRow["Pd"] += Pd_MW;
    busRow["Qd"] += Qd_MVAr;

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

void Network::make_OPF(const Network& net,
    bool vscControl,
    bool writeTxt,
    bool plotResult)
{
    const auto& data = net.getNetData();

    /* 2. Transform to Eigen::MatrixXd */
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

    solveHmo_opf(dataOPF, vscControl, writeTxt, plotResult);
}
