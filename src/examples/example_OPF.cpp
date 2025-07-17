#include "Examples.h"

#include "../network.h"
#include "../Bus.h"
#include "../Include_components.h"
#include "../Solver/OPF/powerflow.h"


void example_OPF() {
    /* ---------- 0 Set Network Object ---------- */
    Network net;
    /* ---------- 1.1 Create AC Buses ---------- */
    net.addDefaultACBuses();
    /*  ---------- 1.2 Add AC Loads  ---------- */
    for (int i = 1; i <= 5; ++i) {
        std::string load_name = "LOAD0" + std::to_string(i);
        std::string bus_name = "ACBUS0" + std::to_string(i);

        Bus* bus = net.getBuses().at(bus_name);

        Load* load = new Load(load_name, 3);
        std::vector<std::string> element_info = load->getElementInfo();

        net.addElement("LOAD", load_name, 3, element_info, 0, bus, true);
    }

    /*  ---------- 1.3 Add AC Generators  ---------- */
    Bus* bus1 = net.getBuses().at("ACBUS01");
    Bus* bus2 = net.getBuses().at("ACBUS02");

    // Generator 1
    Generator* gen1 = new Generator("GEN01", 3);
    net.addElement("GENERATOR", "GEN01", 3, gen1->getElementInfo(), 0, bus1, false);

    // Generator 2
    Generator* gen2 = new Generator("GEN02", 3);
    net.addElement("GENERATOR", "GEN02", 3, gen2->getElementInfo(), 0, bus2, false);


    //Refactor until here
    Bus* bus1_ac = new Bus("ACBUS01", 3);
    Bus* bus2_ac = new Bus("ACBUS02", 3);
    Bus* bus3_ac = new Bus("ACBUS03", 3);
    Bus* bus4_ac = new Bus("ACBUS04", 3);
    Bus* bus5_ac = new Bus("ACBUS05", 3);
   

    ///*  ---------- 1.4 Add Branches  ---------- */
    // AC Branches
    std::map<std::string, double> globals = {
     {"omega", 2 * M_PI * 50},
     {"Z_base", 1.0}
    };
    std::vector<std::vector<std::string>> dict_br_ac;

    DenseMatrix ACZ1(1, 1);
    double ACR1 = 0.02;
    double ACX1 = 0.06;
    RCP<const Basic> ACZsym1 = real_double(ACR1);
    ACZ1.set(0, 0, ACZsym1);
    Impedance* br1_ac = new Impedance("br1_ac", 3, ACZ1);
    net.connectElementToBus(br1_ac, /*terminal=*/1, bus1_ac);
    net.connectElementToBus(br1_ac, /*terminal=*/2, bus2_ac);
    std::vector<std::string> info_br1_ac = { "ACBR1", "1", "0.06" };
    net.make_BranchAC(br1_ac, globals, info_br1_ac, false);

    DenseMatrix ACZ2(1, 1);
    double ACR2 = 0.08;
    double ACX2 = 0.24;
    RCP<const Basic> ACZsym2 = real_double(ACR2);
    ACZ2.set(0, 0, ACZsym2);
    Impedance* br2_ac = new Impedance("br2_ac", 3, ACZ2);
    net.connectElementToBus(br2_ac, /*terminal=*/1, bus1_ac);
    net.connectElementToBus(br2_ac, /*terminal=*/2, bus3_ac);
    std::vector<std::string> info_br2_ac = { "ACBR2", "1", "0.05" };
    net.make_BranchAC(br2_ac, globals, info_br2_ac, false);

    DenseMatrix ACZ3(1, 1);
    double ACR3 = 0.06;
    double ACX3 = 0.18;
    // RCP<const Basic> ACZsym3 = add(real_double(ACR3),
    //    mul(I, real_double(ACX3)));
    RCP<const Basic> ACZsym3 = real_double(ACR3);
    ACZ3.set(0, 0, ACZsym3);
    Impedance* br3_ac = new Impedance("br3_ac", 3, ACZ3);
    net.connectElementToBus(br3_ac, /*terminal=*/1, bus2_ac);
    net.connectElementToBus(br3_ac, /*terminal=*/2, bus3_ac);
    std::vector<std::string> info_br3_ac = { "ACBR3", "1", "0.04" };
    net.make_BranchAC(br3_ac, globals, info_br3_ac, false);

    DenseMatrix ACZ4(1, 1);
    double ACR4 = 0.06;
    double ACX4 = 0.18;
    // RCP<const Basic> ACZsym4 = add(real_double(ACR4),
    //    mul(I, real_double(ACX4)));
    RCP<const Basic> ACZsym4 = real_double(ACR4);
    ACZ4.set(0, 0, ACZsym4);
    Impedance* br4_ac = new Impedance("br4_ac", 3, ACZ4);
    net.connectElementToBus(br4_ac, /*terminal=*/1, bus2_ac);
    net.connectElementToBus(br4_ac, /*terminal=*/2, bus4_ac);
    std::vector<std::string> info_br4_ac = { "ACBR4", "1", "0.04" };
    net.make_BranchAC(br4_ac, globals, info_br4_ac, false);

    DenseMatrix ACZ5(1, 1);
    double ACR5 = 0.04;
    double ACX5 = 0.12;
    // RCP<const Basic> ACZsym5 = add(real_double(ACR5),
    //    mul(I, real_double(ACX5)));
    RCP<const Basic> ACZsym5 = real_double(ACR5);
    ACZ5.set(0, 0, ACZsym5);
    Impedance* br5_ac = new Impedance("br5_ac", 3, ACZ5);
    net.connectElementToBus(br5_ac, /*terminal=*/1, bus2_ac);
    net.connectElementToBus(br5_ac, /*terminal=*/2, bus5_ac);
    std::vector<std::string> info_br5_ac = { "ACBR5", "1", "0.03" };
    net.make_BranchAC(br5_ac, globals, info_br5_ac, false);

    DenseMatrix ACZ6(1, 1);
    double ACR6 = 0.01;
    double ACX6 = 0.03;
    // RCP<const Basic> ACZsym6 = add(real_double(ACR6),
    //    mul(I, real_double(ACX6)));
    RCP<const Basic> ACZsym6 = real_double(ACR6);
    ACZ6.set(0, 0, ACZsym6);
    Impedance* br6_ac = new Impedance("br6_ac", 3, ACZ6);
    net.connectElementToBus(br6_ac, /*terminal=*/1, bus3_ac);
    net.connectElementToBus(br6_ac, /*terminal=*/2, bus4_ac);
    std::vector<std::string> info_br6_ac = { "ACBR6", "1", "0.02" };
    net.make_BranchAC(br6_ac, globals, info_br6_ac, false);


    DenseMatrix ACZ7(1, 1);
    double ACR7 = 0.08;
    double ACX7 = 0.24;
    //RCP<const Basic> ACZsym7 = add(real_double(ACR7),
    //    mul(I, real_double(ACX7)));
    RCP<const Basic> ACZsym7 = real_double(ACR7);
    ACZ7.set(0, 0, ACZsym7);
    Impedance* br7_ac = new Impedance("br7_ac", 3, ACZ7);
    net.connectElementToBus(br7_ac, /*terminal=*/1, bus4_ac);
    net.connectElementToBus(br7_ac, /*terminal=*/2, bus5_ac);
    std::vector<std::string> info_br7_ac = { "ACBR7", "1", "0.05" };
    net.make_BranchAC(br7_ac, globals, info_br7_ac, false);

    /*  ---------- 2.1 Create DC Buses  ---------- */
    Bus* bus1_dc = new Bus("DCBUS01", 1);
    Bus* bus2_dc = new Bus("DCBUS02", 1);
    Bus* bus3_dc = new Bus("DCBUS03", 1);
    std::vector<std::vector<std::string>> dict_dc;
    net.addBusDC(dict_dc, { "DCBUS01", "330", "1.1", "0.9" }, false);
    net.addBusDC(dict_dc, { "DCBUS02", "330", "1.1", "0.9" }, false);
    net.addBusDC(dict_dc, { "DCBUS03", "330", "1.1", "0.9" }, false);

    ///*  ---------- 2.2 Create DC Buses  ---------- */

    DenseMatrix DCZ1(1, 1);
    double DCR1 = 0.052;
    RCP<const Basic> DCZsym1 = real_double(DCR1);
    DCZ1.set(0, 0, DCZsym1);
    Impedance* br1_dc = new Impedance("br1_dc", 1, DCZ1);
    net.connectElementToBus(br1_dc, /*terminal=*/1, bus1_dc);
    net.connectElementToBus(br1_dc, /*terminal=*/2, bus2_dc);
    std::vector<std::string> info_br1_dc = { "DCBR1" };
    net.make_BranchDC(br1_dc, globals, info_br1_dc, false);

    DenseMatrix DCZ2(1, 1);
    double DCR2 = 0.073;
    RCP<const Basic> DCZsym2 = real_double(DCR2);
    DCZ2.set(0, 0, DCZsym2);
    Impedance* br2_dc = new Impedance("br2_dc", 1, DCZ2);
    net.connectElementToBus(br2_dc, /*terminal=*/1, bus1_dc);
    net.connectElementToBus(br2_dc, /*terminal=*/2, bus3_dc);
    std::vector<std::string> info_br2_dc = { "DCBR2" };
    net.make_BranchDC(br2_dc, globals, info_br2_dc, false);

    DenseMatrix DCZ3(1, 1);
    double DCR3 = 0.052;
    RCP<const Basic> DCZsym3 = real_double(DCR3);
    DCZ3.set(0, 0, DCZsym3);
    Impedance* br3_dc = new Impedance("br3_dc", 1, DCZ3);
    net.connectElementToBus(br3_dc, /*terminal=*/1, bus2_dc);
    net.connectElementToBus(br3_dc, /*terminal=*/2, bus3_dc);
    std::vector<std::string> info_br3_dc = { "DCBR3" };
    net.make_BranchDC(br3_dc, globals, info_br3_dc, false);

    /*  ---------- 2.3 Create Converters ---------- */
    MMC* mmc1 = new MMC(
        "MMC1",         // Symbol
        1000.0,         // Omega (Nominal Frequency in rad/s)
        100.0,          // Active Power (P) in MW
        50.0,           // Reactive Power (Q) in MVA
        0.0,            // Theta (Voltage Angle in rad)
        330.0,          // AC Voltage (V_m) in kV
        640.0,          // DC Voltage (V_dc) in kV
        0.05,           // Arm Inductance (L_arm) in H
        1.07,           // Arm Resistance (R_arm) in Ω
        0.01,           // Capacitance per Submodule (C_arm) in F
        400,            // Number of Submodules (N)
        0.06,           // Reactor Inductance (L_reactor) in H
        0.535,          // Reactor Resistance (R_reactor) in Ω
        0.00015         // Time Delay (t_delay) in seconds
    );
    net.connectElementToBus(mmc1, 1, bus2_ac);
    net.connectElementToBus(mmc1, 2, bus1_dc);

    std::vector<std::string> info_conv1 = {
     "MMC1",          // 0  generator_name
     "1",             // 1  grid_area
     "1",             // 2  type_dc 
     "1",             // 3  type_ac
     "0.0015",        // 4  rftc
     "0.1121",        // 5  xtfc
     "0.0887",        // 6  bf
     "0.0001",        // 7  rc
     "0.16428",       // 8  xc
     "345",           // 9  basekVac
     "1.1",           // 10 Vmmax
     "0.9",           // 11 Vmmin
     "1.2",           // 12 Imax
     "1.103",         // 13 LossAC
     "0.887",         // 14 LossB
     "2.885",         // 15 LossCrec
     "4.371",         // 16 LossCinv
     "0",             // 17 droop
     "0",             // 18 Pdcset
     "0",             // 19 Vdcset
     "0",             // 20 Dvdsetc
    };
    net.make_Converter(mmc1, globals, info_conv1, false);


    MMC* mmc2 = new MMC(
        "MMC2",         // Symbol
        1000.0,         // Omega (Nominal Frequency in rad/s)
        100.0,          // Active Power (P) in MW
        50.0,           // Reactive Power (Q) in MVA
        0.0,            // Theta (Voltage Angle in rad)
        330.0,          // AC Voltage (V_m) in kV
        640.0,          // DC Voltage (V_dc) in kV
        0.05,           // Arm Inductance (L_arm) in H
        1.07,           // Arm Resistance (R_arm) in Ω
        0.01,           // Capacitance per Submodule (C_arm) in F
        400,            // Number of Submodules (N)
        0.06,           // Reactor Inductance (L_reactor) in H
        0.535,          // Reactor Resistance (R_reactor) in Ω
        0.00015         // Time Delay (t_delay) in seconds
    );
    net.connectElementToBus(mmc2, 1, bus3_ac);
    net.connectElementToBus(mmc2, 2, bus2_dc);

    std::vector<std::string> info_conv2 = {
     "MMC2",          // 0  generator_name
     "1",             // 1  grid_area
     "2",             // 2  type_dc 
     "2",             // 3  type_ac
     "0.0015",        // 4  rftc
     "0.1121",        // 5  xtfc
     "0.0887",        // 6  bf
     "0.0001",        // 7  rc
     "0.16428",       // 8  xc
     "345",           // 9  basekVac
     "1.1",           // 10 Vmmax
     "0.9",           // 11 Vmmin
     "1.2",           // 12 Imax
     "1.103",         // 13 LossAC
     "0.887",         // 14 LossB
     "2.885",         // 15 LossCrec
     "4.371",         // 16 LossCinv
     "0",             // 17 droop
     "0",             // 18 Pdcset
     "0",             // 19 Vdcset
     "0",             // 20 Dvdsetc
    };
    net.make_Converter(mmc2, globals, info_conv2, false);


    MMC* mmc3 = new MMC(
        "MMC3",         // Symbol
        1000.0,         // Omega (Nominal Frequency in rad/s)
        100.0,          // Active Power (P) in MW
        50.0,           // Reactive Power (Q) in MVA
        0.0,            // Theta (Voltage Angle in rad)
        330.0,          // AC Voltage (V_m) in kV
        640.0,          // DC Voltage (V_dc) in kV
        0.05,           // Arm Inductance (L_arm) in H
        1.07,           // Arm Resistance (R_arm) in Ω
        0.01,           // Capacitance per Submodule (C_arm) in F
        400,            // Number of Submodules (N)
        0.06,           // Reactor Inductance (L_reactor) in H
        0.535,          // Reactor Resistance (R_reactor) in Ω
        0.00015         // Time Delay (t_delay) in seconds
    );
    net.connectElementToBus(mmc3, 1, bus5_ac);
    net.connectElementToBus(mmc3, 2, bus3_dc);

    std::vector<std::string> info_conv3 = {
     "MMC3",          // 0  generator_name
     "1",             // 1  grid_area
     "1",             // 2  type_dc 
     "1",             // 3  type_ac
     "0.0015",        // 4  rftc
     "0.1121",        // 5  xtfc
     "0.0887",        // 6  bf
     "0.0001",        // 7  rc
     "0.16428",       // 8  xc
     "345",           // 9  basekVac
     "1.1",           // 10 Vmmax
     "0.9",           // 11 Vmmin
     "1.2",           // 12 Imax
     "1.103",         // 13 LossAC
     "0.887",         // 14 LossB
     "2.885",         // 15 LossCrec
     "4.371",         // 16 LossCinv
     "0",             // 17 droop
     "0",             // 18 Pdcset
     "0",             // 19 Vdcset
     "0",             // 20 Dvdsetc
    };
    net.make_Converter(mmc3, globals, info_conv3, false);

    /*----- 3 OPF Implementatiopn ----- */
    net.make_OPF(net,false, false, false);
}