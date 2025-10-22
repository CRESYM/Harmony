#include "Examples.h"

#include "../network.h"
#include "../Bus.h"
#include "../Include_components.h"
#include "../Solver/OPF/powerflow.h"
#include "../Solver/Stability_Estimate/Stability_estimate.h"


void example_point2point_case() {

    ///* ---------- 0 Set Network Object ---------- */
    Network net;

    ///* ---------- 1.1 Create AC Buses ---------- */
    Bus* bus1_ac = new Bus("ACBUS01", "AC1", 3);
    Bus* bus2_ac = new Bus("ACBUS02", "AC1", 3);
    Bus* bus3_ac = new Bus("ACBUS03", "AC2", 3);
    Bus* bus4_ac = new Bus("ACBUS04", "AC2", 3); // 

    ///*  ---------- 1.2 Add AC Loads  ---------- */

    std::vector<double> load_params2 = { 2380.5, 18.9, 0 };
    Load* load2 = new Load("LOAD02", "AC2", 3, load_params2);
    net.connectElementToBus(load2, 1, bus4_ac);

    ///*  ---------- 1.3 Add AC Generators  ---------- */

    /// Generator 1
    double Zsrc = 0.002; 
    AC_source* src1 = new AC_source("SRC01", "AC1", 3, Zsrc);
    net.connectElementToBus(src1, 1, bus1_ac);
    map<string, double> src_info1 = {
        {"Pmax", 250.0},   // Maximum active power output (MW)
        {"Pmin", 10.0},    // Minimum active power output (MW)

        {"Qmax", 10.0},    // Maximum reactive power output (MVar)
        {"Qmin", -10.0},   // Minimum reactive power output (MVar)

        {"c2", 0.11},      // Quadratic coefficient of the generation cost function (c2*P^2 + c1*P + c0)
        {"c1", 5.0},       // Linear coefficient of the generation cost function
        {"c0", 150},       // Constant term of the generation cost function (fixed operation cost)

        {"Vg", 345},       // Voltage magnitude setpoint of the source (kV)
        {"Zsrc", Zsrc},    // Internal source impedance (Ω), used in power flow and short-circuit analysis
        {"Ref", 1}         // Reference bus flag (1 = set as slack/reference bus)
    };
    src1->setOPFInfo(src_info1);

    ///*  ---------- 1.4 Add Branches  ---------- */
    double ACR1 = 1e-1; double ACX1 = 1;
    std::complex<double> ACZ1(ACR1, ACX1);
    Impedance* br1_ac = new Impedance("br1_ac", "AC1", 3, ACZ1);
    net.connectElementToBus(br1_ac, /*terminal=*/1, bus1_ac);
    net.connectElementToBus(br1_ac, /*terminal=*/2, bus2_ac);

    double ACR2 = 1e-1; double ACX2 = 1;
    std::complex<double> ACZ2(ACR2, ACX2);
    Impedance* br2_ac = new Impedance("br2_ac", "AC2", 3, ACZ2);
    net.connectElementToBus(br2_ac, /*terminal=*/1, bus3_ac);
    net.connectElementToBus(br2_ac, /*terminal=*/2, bus4_ac);

    ///*  ---------- 2.1 Create DC Buses  ---------- */
    Bus* bus1_dc = new Bus("DCBUS01", "DC1", 1);
    Bus* bus2_dc = new Bus("DCBUS02", "DC1", 1);

    ///*  ---------- 2.2 Create DC Buses  ---------- */
    double DCR1 = 0.5;
    Impedance* br1_dc = new Impedance("br1_dc", "DC1", 1, DCR1);
    net.connectElementToBus(br1_dc, /*terminal=*/1, bus1_dc);
    net.connectElementToBus(br1_dc, /*terminal=*/2, bus2_dc);

    ///*  ---------- 2.3 Create Converters ---------- */
    MMC* mmc1 = new MMC(
        "MMC1",             // Symbol
        "AC1_DC1",              // Location
        1000.0,             // Omega (Nominal Frequency in rad/s)
        -60.0 * 1e6,        // Active Power (P) in W
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
    net.connectElementToBus(mmc1, 1, bus2_ac);
    net.connectElementToBus(mmc1, 2, bus1_dc);
    map<string, double> mmc1_info = {
        {"type_dc", 1},            // 2
        {"type_ac", 1},            // 3
    };
    mmc1->setOPFInfo(mmc1_info);

    MMC* mmc2 = new MMC(
        "MMC2",             // Symbol
        "AC2_DC1", 		    // Location
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
    net.connectElementToBus(mmc2, 1, bus3_ac);
    net.connectElementToBus(mmc2, 2, bus2_dc);
    map<string, double> mmc2_info = {
        {"type_dc", 2},            // 2
        {"type_ac", 2},            // 3
    };
    mmc2->setOPFInfo(mmc2_info);

    ///*----- 3 OPF Implementatiopn ----- */
    PowerFlow pf;

    //const auto& data = net.getNetData();
    std::map<std::string, double> global_params;
    double omega = 2 * M_PI * 50;
    global_params["omega"] = omega;
    global_params["baseMVA"] = 100;
    global_params["ACbaseKV"] = 345.0; // Base voltage in kV, can be adjusted as needed
    global_params["DCbaseKV"] = 500.0; // Base voltage for DC, can be adjusted as needed
    global_params["Z_base"] = global_params["ACbaseKV"] * global_params["ACbaseKV"] / global_params["baseMVA"]; // Base impedance, can be adjusted as needed

    pf.make_OPF(&net, global_params, false, false, false, false);

    auto res01 = pf.getDCBusResult("DCBUS01", global_params);
    std::cout << res01.busName
        << " Vdc=" << res01.vn << " kV "
        << " Pdc=" << res01.pn << " MW "
        << " Pac (PCC-side)=" << res01.ps << " MW "
        << " Qac (PCC-side)=" << res01.qs << " Mvar "
        << " Vac (PCC-side)=" << res01.vs << " kV "
        << " theta ac (PCC-side)=" << res01.thetas << " rad "
        << " Pac (AC terminal)=" << res01.pc << " MW "
        << " Qac (AC terminal)=" << res01.qc << " Mvar "
        << " Vac (AC terminal)=" << res01.vc << " kV "
        << " theta ac (AC terminal)=" << res01.thetac << " rad "
        << "\n";

    auto res02 = pf.getDCBusResult("DCBUS02", global_params);
    std::cout << res02.busName
        << " Vdc=" << res02.vn << " kV "
        << " Pdc=" << res02.pn << " MW "
        << " Pac (PCC-side)=" << res02.ps << " MW "
        << " Qac (PCC-side)=" << res02.qs << " Mvar "
        << " Vac (PCC-side)=" << res02.vs << " kV "
        << " theta ac (PCC-side)=" << res02.thetas << " rad "
        << " Pac (AC terminal)=" << res02.pc << " MW "
        << " Qac (AC terminal)=" << res02.qc << " Mvar "
        << " Vac (AC terminal)=" << res02.vc << " kV "
        << " theta ac (AC terminal)=" << res02.thetac << " rad "
        << "\n";

    // Making Stability Estimate Object
	//bus1_ac->setBusName("gnd"); // Change bus name to reflect its area
    StabilityEstimate* stability = new StabilityEstimate();
    stability->add_areas(&net);
    //stability->print_summary();


    // Compute equivalent impedance between two AC buses, skipping the MMCs
    auto& dc_grids = stability->get_dc_grids();
	auto& ac_grids = stability->get_ac_grids();
    double omega_num = 2 * M_PI * 50.0; // Frequency in rad/s

    MatrixXcd Y_params = stability->compute_equivalent_admittance_parameters_num(dc_grids["DC1"], omega_num);
    cout << "Equivalent Admittance Matrix at DC side at " << omega_num << " rad/s:" << endl;
    cout << Y_params << endl;

	MatrixXcd Y_params_ac = stability->compute_equivalent_admittance_parameters_num(ac_grids["AC2"], omega_num);
	cout << "Equivalent Admittance Matrix of AC grid at " << omega_num << " rad/s:" << endl;
	cout << Y_params_ac << endl;
}