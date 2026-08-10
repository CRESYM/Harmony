/**
 * @file example_OPF_single_area.cpp
 * @brief Runnable example: Basic hybrid AC–DC optimal power flow case.
 */
#include "Examples.h"

#include "../network.h"
#include "../Bus.h"
#include "../Include_components.h"
#include "../Solver/OPF/Powerflow.h"


void example_OPF_single_area(bool plotting_enabled /*=true*/) {
    ///* ---------- 0 Set Network Object ---------- */
    Network net;

    // Common base for branches and OPF (raise Sbase for GW-scale flows / loss limit)
    const double base_mva = 2500.0;
    const double zbase_ac = 345.0 * 345.0 / base_mva;
    const double zbase_dc = 500.0 * 500.0 / base_mva;

    ///* ---------- 1.1 Create AC Buses ---------- */
    Bus* bus1_ac = new Bus("ACBUS01", "AC1", 3);
    Bus* bus2_ac = new Bus("ACBUS02", "AC1", 3);
    Bus* bus3_ac = new Bus("ACBUS03", "AC1", 3);
    Bus* bus4_ac = new Bus("ACBUS04", "AC1", 3);
    Bus* bus5_ac = new Bus("ACBUS05", "AC1", 3);

    ///*  ---------- 1.2 Add AC Loads (~2 GW total, R-only equivalent at 345 kV) ---------- */
    const double v_phase = 345e3 / std::sqrt(3.0);
    auto load_r = [&](double p_mw) {
        return 3.0 * v_phase * v_phase / (p_mw * 1e6);
    };

    std::vector<double> load_params1 = { 1e12, 0., 0 }; // slack bus — negligible load
    Load* load1 = new Load("LOAD01", "AC1", 3, load_params1);
    net.connectElementToBus(load1, 1, bus1_ac);

    std::vector<double> load_params2 = { load_r(300.0), 0., 0 };
    Load* load2 = new Load("LOAD02", "AC1", 3, load_params2);
    net.connectElementToBus(load2, 1, bus2_ac);

    std::vector<double> load_params3 = { load_r(400.0), 0., 0 };
    Load* load3 = new Load("LOAD03", "AC1", 3, load_params3);
    net.connectElementToBus(load3, 1, bus3_ac);

    std::vector<double> load_params4 = { load_r(400.0), 0., 0 };
    Load* load4 = new Load("LOAD04", "AC1", 3, load_params4);
    net.connectElementToBus(load4, 1, bus4_ac);

    std::vector<double> load_params5 = { load_r(900.0), 0., 0 };
    Load* load5 = new Load("LOAD05", "AC1", 3, load_params5);
    net.connectElementToBus(load5, 1, bus5_ac);


	///*  ---------- 1.3 Add AC Generators  ---------- */
    // Generator 1
    std::vector<double> gen1_params = { 0.02, 0.3, 0.05};
    Generator* gen1 = new Generator("GEN01", "AC1", 3, 345e3 * 1.06, gen1_params);
    net.connectElementToBus(gen1, 1, bus1_ac);
    map<string, double> gen_info1 = {
        {"Pmax", 8000}, {"Pmin", 0},
        {"Qmax", 10000.0}, {"Qmin", -10000.0},
        {"c2", 0.11}, {"c1", 5.0},
        {"c0", 150}, {"Ref", 1}, {"Vg", 345 * 1.06}
    };
	gen1->setOPFInfo(gen_info1);

    std::vector<double> gen2_params = { 0.02, 0.3, 0.05};
    Generator* gen2 = new Generator("GEN02", "AC1", 3, 345e3, gen2_params);
    net.connectElementToBus(gen2, 1, bus2_ac);
    map<string, double> gen_info2 = {
        {"Pmax", 8000.0}, {"Pmin", 500.0},
        {"Qmax", 4000.0}, {"Qmin", -4000.0},
        {"c2", 0.085}, {"c1", 1.2},
        {"c0", 600}
	};
	gen2->setOPFInfo(gen_info2);

    ///*  ---------- 1.4 Add Branches  ---------- */
    double ACR1 = 0.02 * zbase_ac;
    double ACX1 = 0.06 * zbase_ac;
    std::complex<double> ACZ1(ACR1,ACX1);
    Impedance* br1_ac = new Impedance("br1_ac", "AC1", 3, ACZ1);
    net.connectElementToBus(br1_ac, /*terminal=*/1, bus1_ac);
    net.connectElementToBus(br1_ac, /*terminal=*/2, bus2_ac);

    double ACR2 = 0.08 * zbase_ac;
    double ACX2 = 0.24 * zbase_ac;
    std::complex<double> ACZ2(ACR2, ACX2);
    Impedance* br2_ac = new Impedance("br2_ac", "AC1", 3, ACZ2);
    net.connectElementToBus(br2_ac, /*terminal=*/1, bus1_ac);
    net.connectElementToBus(br2_ac, /*terminal=*/2, bus3_ac);
     
    double ACR3 = 0.06 * zbase_ac;
    double ACX3 = 0.18 * zbase_ac;
    std::complex<double> ACZ3(ACR3, ACX3);
    Impedance* br3_ac = new Impedance("br3_ac", "AC1", 3, ACZ3);
    net.connectElementToBus(br3_ac, /*terminal=*/1, bus2_ac);
    net.connectElementToBus(br3_ac, /*terminal=*/2, bus3_ac);

    double ACR4 = 0.06 * zbase_ac;
    double ACX4 = 0.18 * zbase_ac;
    std::complex<double> ACZ4(ACR4, ACX4);
    Impedance* br4_ac = new Impedance("br4_ac", "AC1", 3, ACZ4);
    net.connectElementToBus(br4_ac, /*terminal=*/1, bus2_ac);
    net.connectElementToBus(br4_ac, /*terminal=*/2, bus4_ac);

    double ACR5 = 0.04 * zbase_ac;
    double ACX5 = 0.12 * zbase_ac;
    std::complex<double> ACZ5(ACR5, ACX5);
    Impedance* br5_ac = new Impedance("br5_ac", "AC1", 3, ACZ5);
    net.connectElementToBus(br5_ac, /*terminal=*/1, bus2_ac);
    net.connectElementToBus(br5_ac, /*terminal=*/2, bus5_ac);

    double ACR6 = 0.01 * zbase_ac;
    double ACX6 = 0.03 * zbase_ac;
    std::complex<double> ACZ6(ACR6, ACX6);
    Impedance* br6_ac = new Impedance("br6_ac", "AC1", 3, ACZ6);
    net.connectElementToBus(br6_ac, /*terminal=*/1, bus3_ac);
    net.connectElementToBus(br6_ac, /*terminal=*/2, bus4_ac);

    double ACR7 = 0.08 * zbase_ac;
    double ACX7 = 0.24 * zbase_ac;
    std::complex<double> ACZ7(ACR7, ACX7);
    Impedance* br7_ac = new Impedance("br7_ac", "AC1", 3, ACZ7);
    net.connectElementToBus(br7_ac, /*terminal=*/1, bus4_ac);
    net.connectElementToBus(br7_ac, /*terminal=*/2, bus5_ac); 

    ///*  ---------- 2.1 Create DC Buses  ---------- */
    Bus* bus1_dc = new Bus("DCBUS01", "DC1", 2);
    Bus* bus2_dc = new Bus("DCBUS02", "DC1", 2);
    Bus* bus3_dc = new Bus("DCBUS03", "DC1", 2);
    

    ///*  ---------- 2.2 Create DC Branches  ---------- */
    double DCR1 = 0.052 * zbase_dc;
    Impedance* br1_dc = new Impedance("br1_dc", "DC1", 2, DCR1);
    net.connectElementToBus(br1_dc, /*terminal=*/1, bus1_dc);
    net.connectElementToBus(br1_dc, /*terminal=*/2, bus2_dc);

    double DCR2 = 0.073 * zbase_dc;
    Impedance* br2_dc = new Impedance("br2_dc", "DC1", 2, DCR2);
    net.connectElementToBus(br2_dc, /*terminal=*/1, bus1_dc);
    net.connectElementToBus(br2_dc, /*terminal=*/2, bus3_dc);

    double DCR3 = 0.052 * zbase_dc;
    Impedance* br3_dc = new Impedance("br3_dc", "DC1", 2, DCR3);
    net.connectElementToBus(br3_dc, /*terminal=*/1, bus2_dc);
    net.connectElementToBus(br3_dc, /*terminal=*/2, bus3_dc);

    ///*  ---------- 2.3 Create Converters (2 GW hybrid link) ---------- */
    const double omega_nom = 2 * M_PI * 50;
    const double v_ac = 345.0 * 1e3;
    const double v_dc = 500.0 * 1e3;
    const double p_rect = 1200.0 * 1e6;
    const double q_rect = 600.0 * 1e6;
    const double p_inv = 800.0 * 1e6;
    const double q_inv = 100.0 * 1e6;

    // MMC1 — rectifier: DC P control + AC Q control
    std::vector<double> converter_params1 = {
        omega_nom, -p_rect, -q_rect, 0.0, v_ac, p_rect, v_dc,
        0.05, 1.07, 0.01, 400, 0.0005, 0.0001, 0.0
    };
    std::vector<double> controller_params1 = {
        1, 0, 0.001103374, 0.00073, 1, 0, // PLL
        0, // dc_voltage
        1, 0, 6.6667e-07, 3.3333e-04, 1, p_rect, // active_power (DC P)
        0, // ac_voltage
        1, 0, 6.6667e-07, 3.3333e-04, 1, -q_rect, // reactive_power (AC Q)
        1, 0, 120, 400, 1, 0, // energy
        1, 0, 19.93, 4500, 1, 4000.0, // zcc
        1, 0, 117.93, 8.5e4, 2, 16000.0, 0, // occ
        1, 0, 19.93, 4500, 2, 0, 0, // ccc
        0 // droop
    };
    MMC* mmc1 = new MMC("MMC1", "AC1", converter_params1, controller_params1);
    net.connectElementToBus(mmc1, 1, bus2_ac);
    net.connectElementToBus(mmc1, 2, bus1_dc);
    map<string, double> mmc1_info = {
        {"type_dc", 1},
        {"type_ac", 1},
	};
    mmc1->setOPFInfo(mmc1_info);

    // MMC2 — DC slack: Vdc control + AC voltage control
    std::vector<double> converter_params2 = {
        omega_nom, 0.0, 0.0, 0.0, v_ac, 0.0, v_dc,
        0.05, 1.07, 0.01, 400, 0.0005, 0.0001, 0.0
    };
    std::vector<double> controller_params2 = {
        1, 0, 0.001103374, 0.00073, 1, 0, // PLL
        1, 0, 2, 82, 2, 0, v_dc, // dc_voltage
        0, // active_power
        0, // ac_voltage
        1, 0, 6.6667e-07, 3.3333e-04, 1, 0.0, // reactive_power (OPF AC-V via type_ac=2)
        1, 0, 120, 400, 1, 0, // energy
        1, 0, 19.93, 4500, 1, 0.0, // zcc
        1, 0, 117.93, 8.5e4, 2, 0.0, 0, // occ
        1, 0, 19.93, 4500, 2, 0, 0, // ccc
        0 // droop
    };
    MMC* mmc2 = new MMC("MMC2", "AC1", converter_params2, controller_params2);
    net.connectElementToBus(mmc2, 1, bus3_ac);
    net.connectElementToBus(mmc2, 2, bus2_dc);
    map<string, double> mmc2_info = {
        {"type_dc", 2},
        {"type_ac", 2},
    };
	mmc2->setOPFInfo(mmc2_info);

    // MMC3 — inverter: DC P control + AC Q control
    std::vector<double> converter_params3 = {
        omega_nom, p_inv, q_inv, 0.0, v_ac, -p_inv, v_dc,
        0.05, 1.07, 0.01, 400, 0.0005, 0.0001, 0.0
    };
    std::vector<double> controller_params3 = {
        1, 0, 0.001103374, 0.00073, 1, 0, // PLL
        0, // dc_voltage
        1, 0, 6.6667e-07, 3.3333e-04, 1, p_inv, // active_power (DC P)
        0, // ac_voltage
        1, 0, 6.6667e-07, 3.3333e-04, 1, q_inv, // reactive_power (AC Q)
        1, 0, 120, 400, 1, 0, // energy
        1, 0, 19.93, 4500, 1, -10666.7, // zcc
        1, 0, 117.93, 8.5e4, 2, -1435.36, 0, // occ
        1, 0, 19.93, 4500, 2, 0, 0, // ccc
        0 // droop
    };
    MMC* mmc3 = new MMC("MMC3", "AC1", converter_params3, controller_params3);
    net.connectElementToBus(mmc3, 1, bus5_ac);
    net.connectElementToBus(mmc3, 2, bus3_dc);
    map<string, double> mmc3_info = {
        {"type_dc", 1},
        {"type_ac", 1},
    };
    mmc3->setOPFInfo(mmc3_info);


	///*----- 3 OPF Implementatiopn ----- */
	PowerFlow pf;

	//const auto& data = net.getNetData();
	std::map<std::string, double> global_dict;
	double omega = 2 * M_PI * 50;
	global_dict["omega"] = omega;
	global_dict["baseMVA"] = base_mva;
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
