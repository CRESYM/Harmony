#include "Examples.h"

#include "../network.h"
#include "../Bus.h"
#include "../Include_components.h"
#include "../Solver/OPF/Powerflow.h"
#include "../Solver/Stability_Estimate/Stability_estimate.h"

void example_OPF_PV(bool plotting_enabled /*=true*/)
{
    /* ---------- 0 Set Network Object ---------- */
    Network net;

    /* ---------- 1.1 Create AC Buses ---------- */
    Bus* bus1_ac = new Bus("ACBUS01", "AC1", 3);
    Bus* bus2_ac = new Bus("ACBUS02", "AC1", 3);
    Bus* bus3_ac = new Bus("ACBUS03", "AC2", 3);
    Bus* bus4_ac = new Bus("ACBUS04", "AC2", 3);

    /* ---------- 1.2 Add AC Loads ---------- */
    std::vector<double> load_params2 = { 119025, 0.01, 0 };
    Load* load2 = new Load("LOAD02", "AC2", 3, load_params2);
    net.connectElementToBus(load2, 1, bus4_ac);

 
    /* ---------- 1.3 Add PV Plant at Generator Bus ---------- */
    std::vector<double> pv_parameters = {
        5e6,        // P_pv
        6570,       // I_pv
        2760,       // N_s
        720,        // N_p
        1.5,        // n
        2.5,        // I_sc
        1e-10,      // I0
        7.2e-3,     // C_pv
        900.0,      // V_dc
        16e-6,      // L_boost
        70e-3,      // C_dc
        4.9809e-06, // kp_boost
        4.9809e-09, // ki_boost
        103e-6,     // L_1
        0,          // R_1
        220e-6,     // C_f
        0.1,        // R_c
        125e-6,     // L_2
        690.0,      // V_g
        50.0,       // f_g
        1.0,        // K_p_dc
        500.0,      // K_i_dc
        0.45,       // K_p_i
        69.7,       // K_i_i
        0.5,        // K_p_pll
        1.0         // K_i_pll
    };

    PVplant* pv1 = new PVplant("PV1", "AC1", pv_parameters);
    net.connectElementToBus(pv1, 1, bus1_ac);

    /* ---------- 1.4 Add AC Branches ---------- */
    double ACR1 = 5;
    double ACX1 = 140;
    std::complex<double> ACZ1(ACR1, ACX1);
    Impedance* br1_ac = new Impedance("br1_ac", "AC1", 3, ACZ1);
    net.connectElementToBus(br1_ac, 1, bus1_ac);
    net.connectElementToBus(br1_ac, 2, bus2_ac);

    double ACR2 = 5;
    double ACX2 = 140;
    std::complex<double> ACZ2(ACR2, ACX2);
    Impedance* br2_ac = new Impedance("br2_ac", "AC2", 3, ACZ2);
    net.connectElementToBus(br2_ac, 1, bus3_ac);
    net.connectElementToBus(br2_ac, 2, bus4_ac);

    /* ---------- 2.1 Create DC Buses ---------- */
    Bus* bus1_dc = new Bus("DCBUS01", "DC1", 2);
    Bus* bus2_dc = new Bus("DCBUS02", "DC1", 2);

    /* ---------- 2.2 Create DC Branch ---------- */
    double DCR1 = 20;
    Impedance* br1_dc = new Impedance("br1_dc", "DC1", 2, DCR1);
    net.connectElementToBus(br1_dc, 1, bus1_dc);
    net.connectElementToBus(br1_dc, 2, bus2_dc);

    /* ---------- 2.3 Create Converters ---------- */
    vector<double> converter_params1 = {
        2 * M_PI * 50,
        50.0 * 1e6,
        0 * 1e6,
        0.0,
        345.0 * 1e3,
        50 * 1e6,
        440.0 * 1e3,
        0.05,
        1.07,
        0.01,
        400,
        0.0005,
        0.0001,
        0.0
    };

    std::vector<double> controller_params1 = {
        1, 0, 0.001103374, 0.00073, 1, 0,
        0,
        1, 0, 6.6667e-07, 3.3333e-04, 1, 50e6,
        0,
        1, 0, 6.6667e-07, 3.3333e-04, 1, 0,
        1, 0, 120, 400, 1, 0,
        1, 0, 19.93, 4500, 1, 166.67,
        1, 0, 117.93, 8.5e4, 2, 666.67, 0,
        1, 0, 19.93, 4500, 2, 0, 0,
        0
    };

    MMC* mmc1 = new MMC("MMC1", "AC1_DC1", converter_params1, controller_params1);
    net.connectElementToBus(mmc1, 1, bus2_ac);
    net.connectElementToBus(mmc1, 2, bus1_dc);

    vector<double> converter_params2 = {
        2 * M_PI * 50,
        -50.0 * 1e6,
        -20e6,
        0.0,
        345.0 * 1e3,
        -50 * 1e6,
        440.0 * 1e3,
        0.05,
        1.07,
        0.01,
        400,
        0.0005,
        0.0001,
        0.0
    };

    std::vector<double> controller_params2 = {
        1, 0, 0.001103374, 0.00073, 1, 0,
        1, 0, 2, 82, 2, 0, 440e3,
        0,
        0,
        1, 0, 6.6667e-07, 3.3333e-04, 1, -20e6,
        1, 0, 120, 400, 1, 0,
        1, 0, 19.93, 4500, 1, -41.66,
        1, 0, 117.93, 8.5e4, 2, -89.71, 0,
        1, 0, 19.93, 4500, 2, 0, 0,
        0
    };

    MMC* mmc2 = new MMC("MMC2", "AC2_DC1", converter_params2, controller_params2);
    net.connectElementToBus(mmc2, 1, bus3_ac);
    net.connectElementToBus(mmc2, 2, bus2_dc);

    /* ---------- 3 OPF Implementation ---------- */
    PowerFlow pf;

    std::map<std::string, double> global_params;
    double omega = 2 * M_PI * 50;
    global_params["omega"] = omega;
    global_params["baseMVA"] = 100;
    global_params["ACbaseKV"] = 345.0;
    global_params["DCbaseKV"] = 400.0;
    global_params["Z_base"] =
        global_params["ACbaseKV"] * global_params["ACbaseKV"] / global_params["baseMVA"];

    pf.make_OPF(&net, global_params, false, false, plotting_enabled, true);

    std::cout << "Press Enter to continue...\n";
    std::cin.get();
}