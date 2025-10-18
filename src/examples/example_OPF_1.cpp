#include "Examples.h"

#include "../network.h"
#include "../Bus.h"
#include "../Include_components.h"
#include "../Solver/OPF/powerflow.h"


void example_OPF_1() {

    ///* ---------- 0 Set Network Object ---------- */
    Network net9bus;

    ///* ---------- 1.1 Create AC Buses ---------- */
    Bus* bus1_ac = new Bus("ACBUS01", "AC1", 3);
    Bus* bus2_ac = new Bus("ACBUS02", "AC1", 3);
    Bus* bus3_ac = new Bus("ACBUS03", "AC1", 3);
    Bus* bus4_ac = new Bus("ACBUS04", "AC1", 3);
    Bus* bus5_ac = new Bus("ACBUS05", "AC1", 3);
    Bus* bus6_ac = new Bus("ACBUS06", "AC1", 3);
    Bus* bus7_ac = new Bus("ACBUS07", "AC1", 3);
    Bus* bus8_ac = new Bus("ACBUS08", "AC1", 3);
    Bus* bus9_ac = new Bus("ACBUS09", "AC1", 3);


    ///*  ---------- 1.2 Add AC Loads  ---------- */

    LoadPQ* load1 = new LoadPQ("LOAD01", "AC1", 3, { 0.0, 0.0 });
    net9bus.connectElementToBus(load1, 1, bus1_ac);

    //std::vector<double> load_params1 = { 0, 0 };
    //LoadPQ* load1 = new LoadPQ("LOAD01", "AC1", 3, load_params1);
    //net.connectElementToBus(load1, 1, bus1_ac);

    LoadPQ* load2 = new LoadPQ("LOAD02", "AC1", 3, { 0.0, 0.0 });
    net9bus.connectElementToBus(load2, 1, bus2_ac);

    LoadPQ* load3 = new LoadPQ("LOAD03", "AC1", 3, { 0.0, 0.0 });
    net9bus.connectElementToBus(load3, 1, bus3_ac);

    LoadPQ* load4 = new LoadPQ("LOAD04", "AC1", 3, { 0.0, 0.0 });
    net9bus.connectElementToBus(load4, 1, bus4_ac);

    // std::vector<double> load_params5 = {1322.7, 12.6, 0 };
    // Load* load5 = new Load("LOAD05", "AC1", 3, load_params5);
    // net.connectElementToBus(load5, 1, bus5_ac);

    LoadPQ* load5 = new LoadPQ("LOAD05", "AC1", 3, { 90.0, 30.0 });
    net9bus.connectElementToBus(load5, 1, bus5_ac);

    LoadPQ* load6 = new LoadPQ("LOAD06", "AC1", 3, { 0.0, 0.0 });
    net9bus.connectElementToBus(load6, 1, bus6_ac);

    LoadPQ* load7 = new LoadPQ("LOAD07", "AC1", 3, { 100.0, 35.0 });
    net9bus.connectElementToBus(load7, 1, bus7_ac);

    LoadPQ* load8 = new LoadPQ("LOAD08", "AC1", 3, { 0.0, 0.0 });
    net9bus.connectElementToBus(load8, 1, bus8_ac);

    LoadPQ* load9 = new LoadPQ("LOAD09", "AC1", 3, { 125.0, 50.0 });
    net9bus.connectElementToBus(load9, 1, bus9_ac);

    ///*  ---------- 1.3 Add AC Generators  ---------- */
    // Generator 1
    std::vector<double> gen1_params = { 0.02, 0.3, 0.05, 7.0 };
    Generator* gen1 = new Generator("GEN01", "AC1", 3, gen1_params);
    net9bus.connectElementToBus(gen1, 1, bus1_ac);
    map<string, double> gen_info1 = {
        {"Pmax", 250.0}, {"Pmin", 10.0},
        {"Qmax", 300.0}, {"Qmin", -300.0},
        {"c2", 0.11}, {"c1", 5.0},
        {"c0", 150}, {"Vg", 345.0 * 1.02}
    };
    gen1->setOPFInfo(gen_info1);

    // Generator 2
    std::vector<double> gen2_params = { 0.02, 0.3, 0.05, 7.0 };
    Generator* gen2 = new Generator("GEN02", "AC1", 3, gen2_params);
    net9bus.connectElementToBus(gen2, 1, bus2_ac);
    map<string, double> gen_info2 = {
        {"Pmax", 300.0}, {"Pmin", 10.0},
        {"Qmax", 300.0}, {"Qmin", -300.0},
        {"c2", 0.085}, {"c1", 1.2},
        {"c0", 600}, {"Vg", 345.0 * 1.04}
    };
    gen2->setOPFInfo(gen_info2);

    // Generator 3
    std::vector<double> gen3_params = { 0.02, 0.3, 0.05, 7.0 };
    Generator* gen3 = new Generator("GEN03", "AC1", 3, gen3_params);
    net9bus.connectElementToBus(gen3, 1, bus3_ac);
    map<string, double> gen_info3 = {
        {"Pmax", 270.0}, {"Pmin", 10.0},
        {"Qmax", 300.0}, {"Qmin", -300.0},
        {"c2", 0.1225}, {"c1", 1.0},
        {"c0", 335}
    };
    gen3->setOPFInfo(gen_info3);

    /*  ---------- 1.x Add RESs (debug)  ---------- */ 
        // Example usage of the PVplant class
    vector<double> pv_parameters = {
        2.8e6,		// P_pv: Rated power of the PV plant in watts
        6570,		// I_pv: Rated current of the PV plant in amperes
        2760,		// N_s: Number of series-connected modules
        720,		// N_p: Number of parallel-connected strings
        1.5,		// n: ideally factor of the diode
        2.5,		// I_sc: Short-circuit current of a single module at STC
        1e-10,		// I0: Reverse saturation current of the diode
        7.2e-3,     // C_pv: Capacitance of the PV array in farads
        900.0,		// V_dc: DC link voltage in volts
        16e-6,      // L_boost: Inductance of the boost converter in henries
        70e-3,      // C_dc: Capacitance of the DC link in farads
        4.9809e-06,	// kp_boost: Proportional gain for the boost converter voltage control loop
        4.9809e-09,	// ki_boost: Integral gain for the boost converter voltage control loop
        103e-6,		// L_1: Inductance of the filter in henries
        0,			// R_1: Resistance of the filter in ohms
        220e-6,		// C_f: Capacitance of the filter in farads
        0.1,		// R_c: Resistance of the filter in ohms
        125e-6,		// L_2: Grid-side inductance in henries
        690.0,		// V_g: Grid voltage in volts
        50.0,	    // f_g: Grid frequency in hertz
        1.0,		// K_p_dc: Proportional gain of the DC voltage controller
        500.0,		// K_i_dc: Integral gain of the DC voltage controller
        0.45,		// K_p_i: Proportional gain of the current controller
        69.7,		// K_i_i: Integral gain of the current controller
        0.5,		// K_p_pll: Proportional gain of the PLL
        1.0			// K_i_pll: Integral gain of the PLL
    };

    PVplant* pv1 = new PVplant("PV1", "AC1", pv_parameters);
    net9bus.connectElementToBus(pv1, 1, bus5_ac);

    ///*  ---------- 1.4 Add Branches  ---------- */
    double ACR1 = 0.0; double ACX1 = 0.0576;
    std::complex<double> ACZ1(ACR1, ACX1);
    Impedance* br1_ac = new Impedance("br1_ac", "AC1", 3, ACZ1);
    net9bus.connectElementToBus(br1_ac, /*terminal=*/1, bus1_ac);
    net9bus.connectElementToBus(br1_ac, /*terminal=*/2, bus4_ac);

    double ACR2 = 0.017; double ACX2 = 0.092;
    std::complex<double> ACZ2(ACR2, ACX2);
    Impedance* br2_ac = new Impedance("br2_ac", "AC1", 3, ACZ2);
    net9bus.connectElementToBus(br2_ac, /*terminal=*/1, bus4_ac);
    net9bus.connectElementToBus(br2_ac, /*terminal=*/2, bus5_ac);

    double ACR3 = 0.039; double ACX3 = 0.17;
    std::complex<double> ACZ3(ACR3, ACX3);
    Impedance* br3_ac = new Impedance("br3_ac", "AC1", 3, ACZ3);
    net9bus.connectElementToBus(br3_ac, /*terminal=*/1, bus5_ac);
    net9bus.connectElementToBus(br3_ac, /*terminal=*/2, bus6_ac);

    double ACR4 = 0.0; double ACX4 = 0.0586;
    std::complex<double> ACZ4(ACR4, ACX4);
    Impedance* br4_ac = new Impedance("br4_ac", "AC1", 3, ACZ4);
    net9bus.connectElementToBus(br4_ac, /*terminal=*/1, bus3_ac);
    net9bus.connectElementToBus(br4_ac, /*terminal=*/2, bus6_ac);

    double ACR5 = 0.0119; double ACX5 = 0.1008;
    std::complex<double> ACZ5(ACR5, ACX5);
    Impedance* br5_ac = new Impedance("br5_ac", "AC1", 3, ACZ5);
    net9bus.connectElementToBus(br5_ac, /*terminal=*/1, bus6_ac);
    net9bus.connectElementToBus(br5_ac, /*terminal=*/2, bus7_ac);

    double ACR6 = 0.0085; double ACX6 = 0.072;
    std::complex<double> ACZ6(ACR6, ACX6);
    Impedance* br6_ac = new Impedance("br6_ac", "AC1", 3, ACZ6);
    net9bus.connectElementToBus(br6_ac, /*terminal=*/1, bus7_ac);
    net9bus.connectElementToBus(br6_ac, /*terminal=*/2, bus8_ac);

    double ACR7 = 0.0; double ACX7 = 0.0625;
    std::complex<double> ACZ7(ACR7, ACX7);
    Impedance* br7_ac = new Impedance("br7_ac", "AC1", 3, ACZ7);
    net9bus.connectElementToBus(br7_ac, /*terminal=*/1, bus8_ac);
    net9bus.connectElementToBus(br7_ac, /*terminal=*/2, bus2_ac);

    double ACR8 = 0.032; double ACX8 = 0.161;
    std::complex<double> ACZ8(ACR8, ACX8);
    Impedance* br8_ac = new Impedance("br8_ac", "AC1", 3, ACZ8);
    net9bus.connectElementToBus(br8_ac, /*terminal=*/1, bus8_ac);
    net9bus.connectElementToBus(br8_ac, /*terminal=*/2, bus9_ac);

    double ACR9 = 0.01; double ACX9 = 0.085;
    std::complex<double> ACZ9(ACR9, ACX9);
    Impedance* br9_ac = new Impedance("br9_ac", "AC1", 3, ACZ9);
    net9bus.connectElementToBus(br9_ac, /*terminal=*/1, bus9_ac);
    net9bus.connectElementToBus(br9_ac, /*terminal=*/2, bus4_ac);

    ///*  ---------- 2.1 Create DC Buses  ---------- */
    Bus* bus1_dc = new Bus("DCBUS01", "DC1", 1);
    Bus* bus2_dc = new Bus("DCBUS02", "DC1", 1);
    Bus* bus3_dc = new Bus("DCBUS03", "DC1", 1);

    ///*  ---------- 2.2 Create DC Buses  ---------- */
    double DCR1 = 0.052;
    Impedance* br1_dc = new Impedance("br1_dc", "DC1", 1, DCR1);
    net9bus.connectElementToBus(br1_dc, /*terminal=*/1, bus1_dc);
    net9bus.connectElementToBus(br1_dc, /*terminal=*/2, bus2_dc);

    double DCR2 = 0.073;
    Impedance* br2_dc = new Impedance("br2_dc", "DC1", 1, DCR2);
    net9bus.connectElementToBus(br2_dc, /*terminal=*/1, bus1_dc);
    net9bus.connectElementToBus(br2_dc, /*terminal=*/2, bus3_dc);

    double DCR3 = 0.052;
    Impedance* br3_dc = new Impedance("br3_dc", "DC1", 1, DCR3);
    net9bus.connectElementToBus(br3_dc, /*terminal=*/1, bus2_dc);
    net9bus.connectElementToBus(br3_dc, /*terminal=*/2, bus3_dc);

    ///*  ---------- 2.3 Create Converters ---------- */
    MMC* mmc1 = new MMC(
        "MMC1",             // Symbol
        "AC1",              // Location
        1000.0,             // Omega (Nominal Frequency in rad/s)
        -60.0 * 1e6,          // Active Power (P) in W
        -40.0 * 1e6,        // Reactive Power (Q) in VA
        0.0,                // Theta (Voltage Angle in rad)
        345.0 * 1e3,        // AC Voltage (V_m) in V
        60 * 1e6,           // DC power (P_dc) in W
        500.0 * 1e3,        // DC Voltage (V_dc) in kV
        0.05,               // Arm Inductance (L_arm) in H
        1.07,               // Arm Resistance (R_arm) in Ω
        0.01,               // Capacitance per Submodule (C_arm) in F
        400,                // Number of Submodules (N)
        0.0005,             // Reactor Inductance (L_reactor) in H
        0.0001,             // Reactor Resistance (R_reactor) in Ω
        0.00015             // Time Delay (t_delay) in seconds
    );
    net9bus.connectElementToBus(mmc1, 1, bus2_ac);
    net9bus.connectElementToBus(mmc1, 2, bus1_dc);
    map<string, double> mmc1_info = {
        {"type_dc", 1},            // 2
        {"type_ac", 1},            // 3
    };
    mmc1->setOPFInfo(mmc1_info);

    MMC* mmc2 = new MMC(
        "MMC2",             // Symbol
        "AC1", 		        // Location
        1000.0,             // Omega (Nominal Frequency in rad/s)
        100.0 * 1e6,        // Active Power (P) in W
        50.0 * 1e6,         // Reactive Power (Q) in VA
        0.0,                // Theta (Voltage Angle in rad)
        345.0 * 1e3,        // AC Voltage (V_m) in V
        -50 * 1e6,          // DC power (P_dc) in W
        500.0 * 1e3,        // DC Voltage (V_dc) in V
        0.05,               // Arm Inductance (L_arm) in H
        1.07,               // Arm Resistance (R_arm) in Ω
        0.01,               // Capacitance per Submodule (C_arm) in F
        400,                // Number of Submodules (N)
        0.0005,             // Reactor Inductance (L_reactor) in H
        0.0001,             // Reactor Resistance (R_reactor) in Ω
        0.00015             // Time Delay (t_delay) in seconds
    );
    net9bus.connectElementToBus(mmc2, 1, bus3_ac);
    net9bus.connectElementToBus(mmc2, 2, bus2_dc);
    map<string, double> mmc2_info = {
        {"type_dc", 2},            // 2
        {"type_ac", 2},            // 3
    };
    mmc2->setOPFInfo(mmc2_info);

    MMC* mmc3 = new MMC(
        "MMC3",               // Symbol
        "AC1", 			      // Location
        1000.0,               // Omega (Nominal Frequency in rad/s)
        35.0 * 1e6,           // Active Power (P) in W
        5.0 * 1e6,            // Reactive Power (Q) in VA
        0.0,                  // Theta (Voltage Angle in rad)
        345.0 * 1e3,          // AC Voltage (V_m) in V
        -35 * 1e6,            // DC power (P_dc) in W
        500.0 * 1e3,          // DC Voltage (V_dc) in V
        0.05,                 // Arm Inductance (L_arm) in H
        1.07,                 // Arm Resistance (R_arm) in Ω
        0.01,                 // Capacitance per Submodule (C_arm) in F
        400,                  // Number of Submodules (N)
        0.0005,               // Reactor Inductance (L_reactor) in H
        0.0001,               // Reactor Resistance (R_reactor) in Ω
        0.00015               // Time Delay (t_delay) in seconds
    );
    net9bus.connectElementToBus(mmc3, 1, bus5_ac);
    net9bus.connectElementToBus(mmc3, 2, bus3_dc);
    map<string, double> mmc3_info = {
    {"type_dc", 1},            // 2
    {"type_ac", 1},            // 3
    };
    mmc3->setOPFInfo(mmc3_info);

    ///*----- 3 OPF Implementatiopn ----- */
    PowerFlow pf;

    //const auto& data = net.getNetData();
    std::map<std::string, double> global_dict;
    double omega = 2 * M_PI * 50;
    global_dict["omega"] = omega;
    global_dict["baseMVA"] = 100;
    global_dict["ACbaseKV"] = 345.0; // Base voltage in kV, can be adjusted as needed
    global_dict["DCbaseKV"] = 500.0; // Base voltage for DC, can be adjusted as needed
    global_dict["Z_base"] = 1.0; // Base impedance, can be adjusted as needed

    pf.make_OPF(&net9bus, global_dict, false, false, false, false);

}