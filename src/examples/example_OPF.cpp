/**
 * @file example_OPF.cpp
 * @brief Runnable example: Basic hybrid AC–DC optimal power flow case.
 */
#include "Examples.h"

#include "../network.h"
#include "../Bus.h"
#include "../Include_components.h"
#include "../Solver/OPF/Powerflow.h"


void example_OPF(bool plotting_enabled /*=true*/) {
    ///* ---------- 0 Set Network Object ---------- */
    Network net;
    ///* ---------- 1.1 Create AC Buses ---------- */
    Bus* bus1_ac = new Bus("ACBUS01", "AC1", 3);
    Bus* bus2_ac = new Bus("ACBUS02", "AC1", 3);
    Bus* bus3_ac = new Bus("ACBUS03", "AC1", 3);
    Bus* bus4_ac = new Bus("ACBUS04", "AC1", 3);
    Bus* bus5_ac = new Bus("ACBUS05", "AC1", 3);

    ///*  ---------- 1.2 Add AC Loads  ---------- */
    std::vector<double> load_params1 = { 1e12, 0., 0 };
    Load* load1 = new Load("LOAD01", "AC1", 3, load_params1);
    net.connectElementToBus(load1, 1, bus1_ac);

    std::vector<double> load_params2 = { 4761, 7.58, 0 };
    Load* load2 = new Load("LOAD02", "AC1", 3, load_params2);
    net.connectElementToBus(load2, 1, bus2_ac);

    std::vector<double> load_params3 = { 2380.5, 2.526, 0 };
    Load* load3 = new Load("LOAD03", "AC1", 3, load_params3);
    net.connectElementToBus(load3, 1, bus3_ac);

    std::vector<double> load_params4 = { 2929.85, 1.166, 0 };
    Load* load4 = new Load("LOAD04", "AC1", 3, load_params4);
    net.connectElementToBus(load4, 1, bus4_ac);

    std::vector<double> load_params5 = { 1930.14, 1.024, 0 };
    Load* load5 = new Load("LOAD05", "AC1", 3, load_params5);
    net.connectElementToBus(load5, 1, bus5_ac);


	///*  ---------- 1.3 Add AC Generators  ---------- */
    // Generator 1
    std::vector<double> gen1_params = { 0.02, 0.3, 0.05};
    Generator* gen1 = new Generator("GEN01", "AC1", 3, 345e3 * 1.06, gen1_params);
    net.connectElementToBus(gen1, 1, bus1_ac);
    map<string, double> gen_info1 = {
        {"Pmax", 300}, {"Pmin", 100},
        {"Qmax", 500.0}, {"Qmin", -500.0},
        {"c2", 0.11}, {"c1", 5.0},
        {"c0", 150}, {"Ref", 1}, {"Vg", 345 * 1.06}
    };
	gen1->setOPFInfo(gen_info1);

    std::vector<double> gen2_params = { 0.02, 0.3, 0.05};
    Generator* gen2 = new Generator("GEN02", "AC1", 3, 345e3, gen2_params);
    net.connectElementToBus(gen2, 1, bus2_ac);
    map<string, double> gen_info2 = {
        {"Pmax", 40.0}, {"Pmin", 40.0},
        {"Qmax", -32.0}, {"Qmin", -32.0},
        {"c2", 0.085}, {"c1", 1.2},
        {"c0", 600}
	};
	gen2->setOPFInfo(gen_info2);

    ///*  ---------- 1.4 Add Branches  ---------- */
    double ZbaseAC = 345.0 * 345.0 / 100.0;

    double ACR1 = 0.02 * ZbaseAC;
    double ACX1 = 0.06 * ZbaseAC;
    std::complex<double> ACZ1(ACR1,ACX1);
    Impedance* br1_ac = new Impedance("br1_ac", "AC1", 3, ACZ1);
    net.connectElementToBus(br1_ac, /*terminal=*/1, bus1_ac);
    net.connectElementToBus(br1_ac, /*terminal=*/2, bus2_ac);

    double ACR2 = 0.08 * ZbaseAC;
    double ACX2 = 0.24 * ZbaseAC;
    std::complex<double> ACZ2(ACR2, ACX2);
    Impedance* br2_ac = new Impedance("br2_ac", "AC1", 3, ACZ2);
    net.connectElementToBus(br2_ac, /*terminal=*/1, bus1_ac);
    net.connectElementToBus(br2_ac, /*terminal=*/2, bus3_ac);
     
    double ACR3 = 0.06 * ZbaseAC;
    double ACX3 = 0.18 * ZbaseAC;
    std::complex<double> ACZ3(ACR3, ACX3);
    Impedance* br3_ac = new Impedance("br3_ac", "AC1", 3, ACZ3);
    net.connectElementToBus(br3_ac, /*terminal=*/1, bus2_ac);
    net.connectElementToBus(br3_ac, /*terminal=*/2, bus3_ac);

    double ACR4 = 0.06 * ZbaseAC;
    double ACX4 = 0.18 * ZbaseAC;
    std::complex<double> ACZ4(ACR4, ACX4);
    Impedance* br4_ac = new Impedance("br4_ac", "AC1", 3, ACZ4);
    net.connectElementToBus(br4_ac, /*terminal=*/1, bus2_ac);
    net.connectElementToBus(br4_ac, /*terminal=*/2, bus4_ac);

    double ACR5 = 0.04 * ZbaseAC;
    double ACX5 = 0.12 * ZbaseAC;
    std::complex<double> ACZ5(ACR5, ACX5);
    Impedance* br5_ac = new Impedance("br5_ac", "AC1", 3, ACZ5);
    net.connectElementToBus(br5_ac, /*terminal=*/1, bus2_ac);
    net.connectElementToBus(br5_ac, /*terminal=*/2, bus5_ac);

    double ACR6 = 0.01 * ZbaseAC;
    double ACX6 = 0.03 * ZbaseAC;
    std::complex<double> ACZ6(ACR6, ACX6);
    Impedance* br6_ac = new Impedance("br6_ac", "AC1", 3, ACZ6);
    net.connectElementToBus(br6_ac, /*terminal=*/1, bus3_ac);
    net.connectElementToBus(br6_ac, /*terminal=*/2, bus4_ac);

    double ACR7 = 0.08 * ZbaseAC;
    double ACX7 = 0.24 * ZbaseAC;
    std::complex<double> ACZ7(ACR7, ACX7);
    Impedance* br7_ac = new Impedance("br7_ac", "AC1", 3, ACZ7);
    net.connectElementToBus(br7_ac, /*terminal=*/1, bus4_ac);
    net.connectElementToBus(br7_ac, /*terminal=*/2, bus5_ac); 

    ///*  ---------- 2.1 Create DC Buses  ---------- */
    Bus* bus1_dc = new Bus("DCBUS01", "DC1", 2);
    Bus* bus2_dc = new Bus("DCBUS02", "DC1", 2);
    Bus* bus3_dc = new Bus("DCBUS03", "DC1", 2);
    

    ///*  ---------- 2.2 Create DC Buses  ---------- */
    double ZbaseDC = 500.0 * 500.0 / 100.0;

    double DCR1 = 0.052 * ZbaseDC;
    Impedance* br1_dc = new Impedance("br1_dc", "DC1", 2, DCR1);
    net.connectElementToBus(br1_dc, /*terminal=*/1, bus1_dc);
    net.connectElementToBus(br1_dc, /*terminal=*/2, bus2_dc);

    double DCR2 = 0.073 * ZbaseDC;
    Impedance* br2_dc = new Impedance("br2_dc", "DC1", 2, DCR2);
    net.connectElementToBus(br2_dc, /*terminal=*/1, bus1_dc);
    net.connectElementToBus(br2_dc, /*terminal=*/2, bus3_dc);

    double DCR3 = 0.052 * ZbaseDC;
    Impedance* br3_dc = new Impedance("br3_dc", "DC1", 2, DCR3);
    net.connectElementToBus(br3_dc, /*terminal=*/1, bus2_dc);
    net.connectElementToBus(br3_dc, /*terminal=*/2, bus3_dc);

    ///*  ---------- 2.3 Create Converters ---------- */
    MMC* mmc1 = new MMC(
        "MMC1",             // Symbol
		"AC1",              // Location
        1000.0,             // Omega (Nominal Frequency in rad/s)
        -60.0*1e6,          // Active Power (P) in W
        -40.0 * 1e6,        // Reactive Power (Q) in VA
        0.0,                // Theta (Voltage Angle in rad)
        345 * 1e3,          // AC Voltage (V_m) in V
		60 * 1e6,           // DC power (P_dc) in W
        500 * 1e3,          // DC Voltage (V_dc) in kV
        0.05,               // Arm Inductance (L_arm) in H
        1.07,               // Arm Resistance (R_arm) in Ω
        0.01,               // Capacitance per Submodule (C_arm) in F
        400,                // Number of Submodules (N)
        0.0005,             // Reactor Inductance (L_reactor) in H
        0.0001,             // Reactor Resistance (R_reactor) in Ω
        0.00015             // Time Delay (t_delay) in seconds
    );
    net.connectElementToBus(mmc1, 1, bus2_ac);
    net.connectElementToBus(mmc1, 2, bus1_dc);
    map<string, double> mmc1_info = {
        {"type_dc", 1},            // 2
        {"type_ac", 1},            // 3
	};
    mmc1->setOPFInfo(mmc1_info);

    MMC* mmc2 = new MMC(
        "MMC2",             // Symbol
		"AC1", 		        // Location
        1000.0,             // Omega (Nominal Frequency in rad/s)
        0.0 * 1e6,        // Active Power (P) in W
        0.0 * 1e6,         // Reactive Power (Q) in VA
        0.0,                // Theta (Voltage Angle in rad)
        345.0 * 1e3,        // AC Voltage (V_m) in V
        0 * 1e6,          // DC power (P_dc) in W
        500.0 * 1e3,        // DC Voltage (V_dc) in V
        0.05,               // Arm Inductance (L_arm) in H
        1.07,               // Arm Resistance (R_arm) in Ω
        0.01,               // Capacitance per Submodule (C_arm) in F
        400,                // Number of Submodules (N)
        0.0005,             // Reactor Inductance (L_reactor) in H
        0.0001,             // Reactor Resistance (R_reactor) in Ω
        0.00015             // Time Delay (t_delay) in seconds
    );
    net.connectElementToBus(mmc2, 1, bus3_ac);
    net.connectElementToBus(mmc2, 2, bus2_dc);
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
    net.connectElementToBus(mmc3, 1, bus5_ac);
    net.connectElementToBus(mmc3, 2, bus3_dc);
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
    global_dict["ACZbase"] =
        global_dict["ACbaseKV"] * global_dict["ACbaseKV"] / global_dict["baseMVA"];
    global_dict["DCZbase"] =
        global_dict["DCbaseKV"] * global_dict["DCbaseKV"] / global_dict["baseMVA"];
    
	pf.make_OPF(&net, global_dict, true, false, plotting_enabled, true);

    cout << "Press Enter to continue...\n";
    cin.get();

}