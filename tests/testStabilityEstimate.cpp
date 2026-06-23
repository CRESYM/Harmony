#include <gtest/gtest.h>
#include "network.h"
#include "Bus.h"
#include "Include_components.h"
#include "Solver/Stability_Estimate/Stability_estimate.h"
#include "Solver/OPF/Powerflow.h"

class TestStabilityEstimate : public testing::Test {};

TEST_F(TestStabilityEstimate, TestOperatingPoint) {
    ///* ---------- 0 Set Network Object ---------- */
    Network net;

    ///* ---------- 1.1 Create AC Buses ---------- */
    Bus* bus1_ac = new Bus("ACBUS01", "AC1", 3);
    Bus* bus2_ac = new Bus("ACBUS02", "AC1", 3);
    Bus* bus3_ac = new Bus("ACBUS03", "AC2", 3);
    Bus* bus4_ac = new Bus("ACBUS04", "AC2", 3); // 

    ///*  ---------- 1.2 Add AC Loads  ---------- */

    std::vector<double> load_params2 = { 3859.46, 2.047, 0 };
    Load* load2 = new Load("LOAD02", "AC2", 3, load_params2);
    net.connectElementToBus(load2, 1, bus4_ac);

    ///*  ---------- 1.3 Add AC Generators  ---------- */

    /// Generator 1
    double Zsrc = 0.1;
    AC_source* src1 = new AC_source("SRC01", "AC1", 3, 345e3, Zsrc);
    net.connectElementToBus(src1, 1, bus1_ac);
    map<string, double> src_info1 = {
        {"Pmax", 250.0},
        {"Pmin", 0.0},
        {"Qmax", 10.0},
        {"Qmin", -10.0},
        {"c2", 0.11},
        {"c1", 5.0},
        {"c0", 150},
        {"Vmax", 1.06},
        {"Vmin", 1.06},
        {"Vg", 345.0 * 1.06},
        {"Ref", 1}
    };

    src1->setOPFInfo(src_info1);

    ///*  ---------- 1.4 Add Branches  ---------- */
    double ACR1 = 1.0;
    double ACX1 = 40.0;
    std::complex<double> ACZ1(ACR1, ACX1);
    Impedance* br1_ac = new Impedance("br1_ac", "AC1", 3, ACZ1);
    net.connectElementToBus(br1_ac, /*terminal=*/1, bus1_ac);
    net.connectElementToBus(br1_ac, /*terminal=*/2, bus2_ac);

    double ACR2 = 1.0;
    double ACX2 = 40.0;
    std::complex<double> ACZ2(ACR2, ACX2);
    Impedance* br2_ac = new Impedance("br2_ac", "AC2", 3, ACZ2);
    net.connectElementToBus(br2_ac, /*terminal=*/1, bus3_ac);
    net.connectElementToBus(br2_ac, /*terminal=*/2, bus4_ac);

    ///*  ---------- 2.1 Create DC Buses  ---------- */
    Bus* bus1_dc = new Bus("DCBUS01", "DC1", 2);
    Bus* bus2_dc = new Bus("DCBUS02", "DC1", 2);

    ///*  ---------- 2.2 Create DC Buses  ---------- */
    double DCR1 = 10.0;
    Impedance* br1_dc = new Impedance("br1_dc", "DC1", 2, DCR1);
    net.connectElementToBus(br1_dc, /*terminal=*/1, bus1_dc);
    net.connectElementToBus(br1_dc, /*terminal=*/2, bus2_dc);

    ///*  ---------- 2.3 Create Converters ---------- */
    vector<double> converter_params1 = {
        2 * M_PI * 50,  // Omega (Nominal Frequency in rad/s)
        50.0 * 1e6,     // Active Power (P) in W
        0 * 1e6,        // Reactive Power (Q) in VA
        0.0,            // Theta (Voltage Angle in rad)
        345.0 * 1e3,    // AC Voltage (V_m) in V
        50 * 1e6,       // DC power (P_dc) in W
        400.0 * 1e3,    // DC Voltage (V_dc) in kV
        0.05,           // Arm Inductance (L_arm) in H
        1.07,           // Arm Resistance (R_arm) in Ω
        0.01,           // Capacitance per Submodule (C_arm) in F
        400,            // Number of Submodules (N)
        0.0005,         // Reactor Inductance (L_reactor) in H
        0.0001,         // Reactor Resistance (R_reactor) in Ω
        0.0             // Time Delay (t_delay) in seconds
    };
    std::vector<double> controller_params1 = {
        1, 0, 0.001103374, 0.00073, 1, 0, // PLL controller parameters
        0, // DC voltage controller parameters
        1, 0, 6.6667e-07, 3.3333e-04, 1, 50e6, // active power
        0, // AC voltage
        1, 0, 6.6667e-07, 3.3333e-04, 1, 0, // reactive power
        1, 0, 120, 400, 1, 0, // energy controller parameters 
        1, 0, 19.93, 4500, 1, 166.67, // zcc controller parameters 
        1, 0, 117.93, 8.5e4, 2, 666.67, 0, // occ controller parameters
        1, 0, 19.93, 4500, 2, 0, 0, // ccc controller parameters
        0 // droop control
    };
    MMC* mmc1 = new MMC("MMC1", "AC1_DC1", converter_params1, controller_params1);
    net.connectElementToBus(mmc1, 1, bus2_ac);
    net.connectElementToBus(mmc1, 2, bus1_dc);

    vector<double> converter_params2 = {
        2 * M_PI * 50,  // Omega (Nominal Frequency in rad/s)
        -50.0 * 1e6,   // Active Power (P) in W
        -10e6,              // Reactive Power (Q) in VA
        0.0,            // Theta (Voltage Angle in rad)
        345.0 * 1e3,    // AC Voltage (V_m) in V
        -50 * 1e6,     // DC power (P_dc) in W
        400.0 * 1e3,    // DC Voltage (V_dc) in kV
        0.05,           // Arm Inductance (L_arm) in H
        1.07,           // Arm Resistance (R_arm) in Ω
        0.01,           // Capacitance per Submodule (C_arm) in F
        400,            // Number of Submodules (N)
        0.0005,         // Reactor Inductance (L_reactor) in H
        0.0001,         // Reactor Resistance (R_reactor) in Ω
        0.0             // Time Delay (t_delay) in seconds
    };
    std::vector<double> controller_params2 = {
        1, 0, 0.001103374, 0.00073, 1, 0, // PLL controller parameters
        1, 0, 2, 82, 2, 0, 400e3, // DC voltage controller parameters
        0, // active power
        0, // AC voltage
        1, 0, 6.6667e-07, 3.3333e-04, 1, -10e6, // reactive power
        1, 0, 120, 400, 1, 0, // energy controller parameters 
        1, 0, 19.93, 4500, 1, -41.66, // zcc controller parameters 
        1, 0, 117.93, 8.5e4, 2, -89.71, 0, // occ controller parameters
        1, 0, 19.93, 4500, 2, 0, 0, // ccc controller parameters
        0  // droop control
    };
    MMC* mmc2 = new MMC("MMC2", "AC2_DC1", converter_params2, controller_params2);
    net.connectElementToBus(mmc2, 1, bus3_ac);
    net.connectElementToBus(mmc2, 2, bus2_dc);

    ///*----- 3 OPF Implementatiopn ----- */
    PowerFlow pf;

    //const auto& data = net.getNetData();
    std::map<std::string, double> global_params;
    double omega = 2 * M_PI * 50;
    global_params["omega"] = omega;
    global_params["baseMVA"] = 100;
    global_params["ACbaseKV"] = 345.0; // Base voltage in kV, can be adjusted as needed
    global_params["DCbaseKV"] = 400.0; // Base voltage for DC, can be adjusted as needed
    global_params["ACZbase"] =
        global_params["ACbaseKV"] * global_params["ACbaseKV"]
        / global_params["baseMVA"];

    global_params["DCZbase"] =
        global_params["DCbaseKV"] * global_params["DCbaseKV"]
        / global_params["baseMVA"];
    pf.make_OPF(&net, global_params, false, false, false, false);

    // Making Stability Estimate Object
    StabilityEstimate* stability = new StabilityEstimate();
    stability->add_areas(&net);

    // Compute equivalent impedance between two AC buses, skipping the MMCs
    auto& dc_grids = stability->get_dc_grids();
    auto& ac_grids = stability->get_ac_grids();

    complex<double> y1 = 1.0 / ACZ1;
    complex<double> yeq1 = y1 * (1.0 / Zsrc) / (y1 + 1.0 / Zsrc);
    

    MatrixXcd Y_params = stability->compute_equivalent_admittance_parameters_num(dc_grids["DC1"], 1000);
	MatrixXcd Y_expected(2, 2);
    Y_expected << 1.0 / DCR1, -1.0 / DCR1,
		-1.0 / DCR1, 1.0 / DCR1;
	EXPECT_TRUE(Y_params.isApprox(Y_expected, 1e-3));

    MatrixXcd Y_params_ac1 = stability->compute_equivalent_admittance_parameters_num(ac_grids["AC1"], 1000);
	MatrixXcd Y_expected_ac1(2, 2);
    Y_expected_ac1 << yeq1, 0,
        0, yeq1;
    EXPECT_TRUE(Y_params_ac1.isApprox(Y_expected_ac1, 1e-3));

	MatrixXcd Y_params_ac2 = stability->compute_equivalent_admittance_parameters_num(ac_grids["AC2"], 1000);
	MatrixXcd Y_expected_ac2(2, 2);
    Y_expected_ac2 << complex<double>(0.00002141663268486, -0.00007124882626222), complex<double>(-0.00000296638331456, -0.00000195511729152),
        complex<double>(0.00000296638331456, 0.00000195511729152), complex<double>(0.00002141663268486, -0.00007124882626222);

	EXPECT_TRUE(Y_params_ac2.isApprox(Y_expected_ac2, 1e-4));

	MatrixXcd TF_mmc1 = stability->compute_transfer_function("MMC2", "AC", 1000);
	MatrixXcd TF_expected_mmc1(2, 2);
    TF_expected_mmc1 << complex<double>(0.55882973170418726, 0.12374482570387829), complex<double>(0.19708733457865962, 0.12172222983223854),
		complex<double>(0.11101225027226810, 0.15018134890425119), complex<double>(-2.14981022600911187, 3.51018238952165262);


    delete stability;
}