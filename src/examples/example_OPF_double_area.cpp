/**
 * @file example_OPF_double_area.cpp
 * @brief Runnable example: Extended hybrid OPF demonstration.
 */
#include "Examples.h"

#include "../network.h"
#include "../Bus.h"
#include "../Include_components.h"
#include "../Solver/OPF/Powerflow.h"


void example_OPF_double_area(bool plotting_enabled /*=true*/) {

    ///* ---------- 0 Set Network Object ---------- */
    Network net;

    ///* ------- 1.1 Create AC Buses (Area 1) ----- */
    Bus* bus1_ac1 = new Bus("ACBUS01", "AC1", 3);
    Bus* bus2_ac1 = new Bus("ACBUS02", "AC1", 3);
    Bus* bus3_ac1 = new Bus("ACBUS03", "AC1", 3);
    Bus* bus4_ac1 = new Bus("ACBUS04", "AC1", 3);
    Bus* bus5_ac1 = new Bus("ACBUS05", "AC1", 3);
    Bus* bus6_ac1 = new Bus("ACBUS06", "AC1", 3);
    Bus* bus7_ac1 = new Bus("ACBUS07", "AC1", 3);
    Bus* bus8_ac1 = new Bus("ACBUS08", "AC1", 3);
    Bus* bus9_ac1 = new Bus("ACBUS09", "AC1", 3);

    ///* ------- 1.2 Create AC Buses (Area 2) ----- */
    Bus* bus1_ac2 = new Bus("AC2BUS01", "AC2", 3);
    Bus* bus2_ac2 = new Bus("AC2BUS02", "AC2", 3);
    Bus* bus3_ac2 = new Bus("AC2BUS03", "AC2", 3);
    Bus* bus4_ac2 = new Bus("AC2BUS04", "AC2", 3);
    Bus* bus5_ac2 = new Bus("AC2BUS05", "AC2", 3);
    Bus* bus6_ac2 = new Bus("AC2BUS06", "AC2", 3);
    Bus* bus7_ac2 = new Bus("AC2BUS07", "AC2", 3);
    Bus* bus8_ac2 = new Bus("AC2BUS08", "AC2", 3);
    Bus* bus9_ac2 = new Bus("AC2BUS09", "AC2", 3);
    Bus* bus10_ac2 = new Bus("AC2BUS10", "AC2", 3);
    Bus* bus11_ac2 = new Bus("AC2BUS11", "AC2", 3);
    Bus* bus12_ac2 = new Bus("AC2BUS12", "AC2", 3);
    Bus* bus13_ac2 = new Bus("AC2BUS13", "AC2", 3);
    Bus* bus14_ac2 = new Bus("AC2BUS14", "AC2", 3);

    ///* ------- 1.3 Add AC Loads (Area 1) ----- */

    // Bus 1
    std::vector<double> load_params1_ac1 = { 1e12, 0.0, 0.0 };
    Load* load1_ac1 = new Load("ACLOAD01", "AC1", 3, load_params1_ac1);
    net.connectElementToBus(load1_ac1, 1, bus1_ac1);

    // Bus 2
    std::vector<double> load_params2_ac1 = { 1e12, 0.0, 0.0 };
    Load* load2_ac1 = new Load("ACLOAD02", "AC1", 3, load_params2_ac1);
    net.connectElementToBus(load2_ac1, 1, bus2_ac1);

    // Bus 3
    std::vector<double> load_params3_ac1 = { 1e12, 0.0, 0.0 };
    Load* load3_ac1 = new Load("ACLOAD03", "AC1", 3, load_params3_ac1);
    net.connectElementToBus(load3_ac1, 1, bus3_ac1);

    // Bus 4
    std::vector<double> load_params4_ac1 = { 1e12, 0.0, 0.0 };
    Load* load4_ac1 = new Load("ACLOAD04", "AC1", 3, load_params4_ac1);
    net.connectElementToBus(load4_ac1, 1, bus4_ac1);

    // Bus 5: 90 MW + j30 MVAr
    std::vector<double> load_params5_ac1 = { 1190.25, 1.262894, 0.0 };
    Load* load5_ac1 = new Load("ACLOAD05", "AC1", 3, load_params5_ac1);
    net.connectElementToBus(load5_ac1, 1, bus5_ac1);

    // Bus 6
    std::vector<double> load_params6_ac1 = { 1e12, 0.0, 0.0 };
    Load* load6_ac1 = new Load("ACLOAD06", "AC1", 3, load_params6_ac1);
    net.connectElementToBus(load6_ac1, 1, bus6_ac1);

    // Bus 7: 100 MW + j35 MVAr
    std::vector<double> load_params7_ac1 = { 1060.356347, 1.181327, 0.0 };
    Load* load7_ac1 = new Load("ACLOAD07", "AC1", 3, load_params7_ac1);
    net.connectElementToBus(load7_ac1, 1, bus7_ac1);

    // Bus 8
    std::vector<double> load_params8_ac1 = { 1e12, 0.0, 0.0 };
    Load* load8_ac1 = new Load("ACLOAD08", "AC1", 3, load_params8_ac1);
    net.connectElementToBus(load8_ac1, 1, bus8_ac1);

    // Bus 9: 125 MW + j50 MVAr
    std::vector<double> load_params9_ac1 = { 820.862069, 1.045154, 0.0 };
    Load* load9_ac1 = new Load("ACLOAD09", "AC1", 3, load_params9_ac1);
    net.connectElementToBus(load9_ac1, 1, bus9_ac1);


    ///* ------- 1.4 Add AC Loads (Area 2) ----- */

    // Bus 1
    std::vector<double> load_params1_ac2 = { 1e12, 0.0, 0.0 };
    Load* load1_ac2 = new Load("AC2LOAD01", "AC2", 3, load_params1_ac2);
    net.connectElementToBus(load1_ac2, 1, bus1_ac2);

    // Bus 2: 21.7 MW + j12.7 MVAr
    std::vector<double> load_params2_ac2 = { 4085.612484, 7.611168, 0.0 };
    Load* load2_ac2 = new Load("AC2LOAD02", "AC2", 3, load_params2_ac2);
    net.connectElementToBus(load2_ac2, 1, bus2_ac2);

    // Bus 3: 94.2 MW + j19 MVAr
    std::vector<double> load_params3_ac2 = { 1214.140995, 0.779510, 0.0 };
    Load* load3_ac2 = new Load("AC2LOAD03", "AC2", 3, load_params3_ac2);
    net.connectElementToBus(load3_ac2, 1, bus3_ac2);

    // Bus 4: 47.8 MW - j3.9 MVAr
    std::vector<double> load_params4_ac2 = { 2473.596226, 0.0, 1.577192e-05 };
    Load* load4_ac2 = new Load("AC2LOAD04", "AC2", 3, load_params4_ac2);
    net.connectElementToBus(load4_ac2, 1, bus4_ac2);

    // Bus 5: 7.6 MW + j1.6 MVAr
    std::vector<double> load_params5_ac2 = { 14996.518568, 10.049558, 0.0 };
    Load* load5_ac2 = new Load("AC2LOAD05", "AC2", 3, load_params5_ac2);
    net.connectElementToBus(load5_ac2, 1, bus5_ac2);

    // Bus 6: 11.2 MW + j7.5 MVAr
    std::vector<double> load_params6_ac2 = { 7337.112664, 15.639345, 0.0 };
    Load* load6_ac2 = new Load("AC2LOAD06", "AC2", 3, load_params6_ac2);
    net.connectElementToBus(load6_ac2, 1, bus6_ac2);

    // Bus 7
    std::vector<double> load_params7_ac2 = { 1e12, 0.0, 0.0 };
    Load* load7_ac2 = new Load("AC2LOAD07", "AC2", 3, load_params7_ac2);
    net.connectElementToBus(load7_ac2, 1, bus7_ac2);

    // Bus 8
    std::vector<double> load_params8_ac2 = { 1e12, 0.0, 0.0 };
    Load* load8_ac2 = new Load("AC2LOAD08", "AC2", 3, load_params8_ac2);
    net.connectElementToBus(load8_ac2, 1, bus8_ac2);

    // Bus 9: 29.5 MW + j16.6 MVAr
    std::vector<double> load_params9_ac2 = { 3064.415130, 5.488881, 0.0 };
    Load* load9_ac2 = new Load("AC2LOAD09", "AC2", 3, load_params9_ac2);
    net.connectElementToBus(load9_ac2, 1, bus9_ac2);

    // Bus 10: 9 MW + j5.8 MVAr
    std::vector<double> load_params10_ac2 = { 9344.251570, 19.168147, 0.0 };
    Load* load10_ac2 = new Load("AC2LOAD10", "AC2", 3, load_params10_ac2);
    net.connectElementToBus(load10_ac2, 1, bus10_ac2);

    // Bus 11: 3.5 MW + j1.8 MVAr
    std::vector<double> load_params11_ac2 = { 26893.963848, 44.026018, 0.0 };
    Load* load11_ac2 = new Load("AC2LOAD11", "AC2", 3, load_params11_ac2);
    net.connectElementToBus(load11_ac2, 1, bus11_ac2);

    // Bus 12: 6.1 MW + j1.6 MVAr
    std::vector<double> load_params12_ac2 = { 18256.286145, 15.242377, 0.0 };
    Load* load12_ac2 = new Load("AC2LOAD12", "AC2", 3, load_params12_ac2);
    net.connectElementToBus(load12_ac2, 1, bus12_ac2);

    // Bus 13: 13.5 MW + j5.8 MVAr
    std::vector<double> load_params13_ac2 = { 7442.852842, 10.178500, 0.0 };
    Load* load13_ac2 = new Load("AC2LOAD13", "AC2", 3, load_params13_ac2);
    net.connectElementToBus(load13_ac2, 1, bus13_ac2);

    // Bus 14: 14.9 MW + j5 MVAr
    std::vector<double> load_params14_ac2 = { 7179.759929, 7.669089, 0.0 };
    Load* load14_ac2 = new Load("AC2LOAD14", "AC2", 3, load_params14_ac2);
    net.connectElementToBus(load14_ac2, 1, bus14_ac2);

    ///* ------- 1.5 Add AC Generators (Area 1) ----- */

    // Generator 1: AC1 bus 1
    std::vector<double> gen1_ac1_params = { 0.02, 0.3, 0.05 };
    Generator* gen1_ac1 = new Generator("AC1GEN01", "AC1", 3, 345.0e3 * 1.000, gen1_ac1_params);
    net.connectElementToBus(gen1_ac1, 1, bus1_ac1);
    map<string, double> gen1_ac1_info = {
        {"Pmax", 500.0}, {"Pmin", 0.0},
        {"Qmax", 500.0}, {"Qmin", -500.0},
        {"c2", 0.110000}, {"c1", 5.0},
        {"c0", 150.0}, {"Ref", 1}, {"Vg", 345.0 * 1.000}
    };
    gen1_ac1->setOPFInfo(gen1_ac1_info);

    // Generator 2: AC1 bus 2
    std::vector<double> gen2_ac1_params = { 0.02, 0.3, 0.05 };
    Generator* gen2_ac1 = new Generator("AC1GEN02", "AC1", 3, 345.0e3 * 1.000, gen2_ac1_params);
    net.connectElementToBus(gen2_ac1, 1, bus2_ac1);
    map<string, double> gen2_ac1_info = {
        {"Pmax", 500.0}, {"Pmin", 0.0},
        {"Qmax", 500.0}, {"Qmin", -500.0},
        {"c2", 0.085000}, {"c1", 1.2},
        {"c0", 600.0}, {"Vg", 345.0 * 1.000}
    };
    gen2_ac1->setOPFInfo(gen2_ac1_info);

    // Generator 3: AC1 bus 3
    std::vector<double> gen3_ac1_params = { 0.02, 0.3, 0.05 };
    Generator* gen3_ac1 = new Generator("AC1GEN03", "AC1", 3, 345.0e3 * 1.000, gen3_ac1_params);
    net.connectElementToBus(gen3_ac1, 1, bus3_ac1);
    map<string, double> gen3_ac1_info = {
        {"Pmax", 500.0}, {"Pmin", 0.0},
        {"Qmax", 500.0}, {"Qmin", -500.0},
        {"c2", 0.122500}, {"c1", 1.0},
        {"c0", 335.0}, {"Vg", 345.0 * 1.000}
    };
    gen3_ac1->setOPFInfo(gen3_ac1_info);


    ///* ------- 1.6 Add AC Generators (Area 2) ----- */

    // Generator 1: AC2 bus 1
    std::vector<double> gen1_ac2_params = { 0.02, 0.3, 0.05 };
    Generator* gen1_ac2 = new Generator("AC2GEN01", "AC2", 3, 345.0e3 * 1.060, gen1_ac2_params);
    net.connectElementToBus(gen1_ac2, 1, bus1_ac2);
    map<string, double> gen1_ac2_info = {
        {"Pmax", 332.4}, {"Pmin", 0.0},
        {"Qmax", 300.0}, {"Qmin", -300.0},
        {"c2", 0.043029}, {"c1", 20.0},
        {"c0", 0.0}, {"Ref", 1}, {"Vg", 345.0 * 1.060}
    };
    gen1_ac2->setOPFInfo(gen1_ac2_info);

    // Generator 2: AC2 bus 2
    std::vector<double> gen2_ac2_params = { 0.02, 0.3, 0.05 };
    Generator* gen2_ac2 = new Generator("AC2GEN02", "AC2", 3, 345.0e3 * 1.045, gen2_ac2_params);
    net.connectElementToBus(gen2_ac2, 1, bus2_ac2);
    map<string, double> gen2_ac2_info = {
        {"Pmax", 300.0}, {"Pmin", 0.0},
        {"Qmax", 300.0}, {"Qmin", -300.0},
        {"c2", 0.250000}, {"c1", 20.0},
        {"c0", 0.0}, {"Vg", 345.0 * 1.045}
    };
    gen2_ac2->setOPFInfo(gen2_ac2_info);

    // Generator 3: AC2 bus 3
    std::vector<double> gen3_ac2_params = { 0.02, 0.3, 0.05 };
    Generator* gen3_ac2 = new Generator("AC2GEN03", "AC2", 3, 345.0e3 * 1.010, gen3_ac2_params);
    net.connectElementToBus(gen3_ac2, 1, bus3_ac2);
    map<string, double> gen3_ac2_info = {
        {"Pmax", 300.0}, {"Pmin", 0.0},
        {"Qmax", 300.0}, {"Qmin", -300.0},
        {"c2", 0.010000}, {"c1", 40.0},
        {"c0", 0.0}, {"Vg", 345.0 * 1.010}
    };
    gen3_ac2->setOPFInfo(gen3_ac2_info);

    // Generator 4: AC2 bus 6
    std::vector<double> gen4_ac2_params = { 0.02, 0.3, 0.05 };
    Generator* gen4_ac2 = new Generator("AC2GEN04", "AC2", 3, 345.0e3 * 1.070, gen4_ac2_params);
    net.connectElementToBus(gen4_ac2, 1, bus6_ac2);
    map<string, double> gen4_ac2_info = {
        {"Pmax", 300.0}, {"Pmin", 0.0},
        {"Qmax", 300.0}, {"Qmin", -300.0},
        {"c2", 0.010000}, {"c1", 40.0},
        {"c0", 0.0}, {"Vg", 345.0 * 1.070}
    };
    gen4_ac2->setOPFInfo(gen4_ac2_info);

    // Generator 5: AC2 bus 8
    std::vector<double> gen5_ac2_params = { 0.02, 0.3, 0.05 };
    Generator* gen5_ac2 = new Generator("AC2GEN05", "AC2", 3, 345.0e3 * 1.090, gen5_ac2_params);
    net.connectElementToBus(gen5_ac2, 1, bus8_ac2);
    map<string, double> gen5_ac2_info = {
        {"Pmax", 300.0}, {"Pmin", 0.0},
        {"Qmax", 300.0}, {"Qmin", -300.0},
        {"c2", 0.010000}, {"c1", 40.0},
        {"c0", 0.0}, {"Vg", 345.0 * 1.090}
    };
    gen5_ac2->setOPFInfo(gen5_ac2_info);
   
    ///* ------- 1.7 Add AC Branches (Area 1) ----- */

    double ZbaseAC = 345.0 * 345.0 / 100.0;

    // Branch 1: bus 1 - bus 4
    double AC1R1 = 0.00000 * ZbaseAC;
    double AC1X1 = 0.05760 * ZbaseAC;
    std::complex<double> AC1Z1(AC1R1, AC1X1);
    Impedance* br1_ac1 = new Impedance("br1_ac1", "AC1", 3, AC1Z1);
    net.connectElementToBus(br1_ac1, 1, bus1_ac1);
    net.connectElementToBus(br1_ac1, 2, bus4_ac1);

    // Branch 2: bus 4 - bus 5
    double AC1R2 = 0.01700 * ZbaseAC;
    double AC1X2 = 0.09200 * ZbaseAC;
    std::complex<double> AC1Z2(AC1R2, AC1X2);
    Impedance* br2_ac1 = new Impedance("br2_ac1", "AC1", 3, AC1Z2);
    net.connectElementToBus(br2_ac1, 1, bus4_ac1);
    net.connectElementToBus(br2_ac1, 2, bus5_ac1);

    // Branch 3: bus 5 - bus 6
    double AC1R3 = 0.03900 * ZbaseAC;
    double AC1X3 = 0.17000 * ZbaseAC;
    std::complex<double> AC1Z3(AC1R3, AC1X3);
    Impedance* br3_ac1 = new Impedance("br3_ac1", "AC1", 3, AC1Z3);
    net.connectElementToBus(br3_ac1, 1, bus5_ac1);
    net.connectElementToBus(br3_ac1, 2, bus6_ac1);

    // Branch 4: bus 3 - bus 6
    double AC1R4 = 0.00000 * ZbaseAC;
    double AC1X4 = 0.05860 * ZbaseAC;
    std::complex<double> AC1Z4(AC1R4, AC1X4);
    Impedance* br4_ac1 = new Impedance("br4_ac1", "AC1", 3, AC1Z4);
    net.connectElementToBus(br4_ac1, 1, bus3_ac1);
    net.connectElementToBus(br4_ac1, 2, bus6_ac1);

    // Branch 5: bus 6 - bus 7
    double AC1R5 = 0.01190 * ZbaseAC;
    double AC1X5 = 0.10080 * ZbaseAC;
    std::complex<double> AC1Z5(AC1R5, AC1X5);
    Impedance* br5_ac1 = new Impedance("br5_ac1", "AC1", 3, AC1Z5);
    net.connectElementToBus(br5_ac1, 1, bus6_ac1);
    net.connectElementToBus(br5_ac1, 2, bus7_ac1);

    // Branch 6: bus 7 - bus 8
    double AC1R6 = 0.00850 * ZbaseAC;
    double AC1X6 = 0.07200 * ZbaseAC;
    std::complex<double> AC1Z6(AC1R6, AC1X6);
    Impedance* br6_ac1 = new Impedance("br6_ac1", "AC1", 3, AC1Z6);
    net.connectElementToBus(br6_ac1, 1, bus7_ac1);
    net.connectElementToBus(br6_ac1, 2, bus8_ac1);

    // Branch 7: bus 8 - bus 2
    double AC1R7 = 0.00000 * ZbaseAC;
    double AC1X7 = 0.06250 * ZbaseAC;
    std::complex<double> AC1Z7(AC1R7, AC1X7);
    Impedance* br7_ac1 = new Impedance("br7_ac1", "AC1", 3, AC1Z7);
    net.connectElementToBus(br7_ac1, 1, bus8_ac1);
    net.connectElementToBus(br7_ac1, 2, bus2_ac1);

    // Branch 8: bus 8 - bus 9
    double AC1R8 = 0.03200 * ZbaseAC;
    double AC1X8 = 0.16100 * ZbaseAC;
    std::complex<double> AC1Z8(AC1R8, AC1X8);
    Impedance* br8_ac1 = new Impedance("br8_ac1", "AC1", 3, AC1Z8);
    net.connectElementToBus(br8_ac1, 1, bus8_ac1);
    net.connectElementToBus(br8_ac1, 2, bus9_ac1);

    // Branch 9: bus 9 - bus 4
    double AC1R9 = 0.01000 * ZbaseAC;
    double AC1X9 = 0.08500 * ZbaseAC;
    std::complex<double> AC1Z9(AC1R9, AC1X9);
    Impedance* br9_ac1 = new Impedance("br9_ac1", "AC1", 3, AC1Z9);
    net.connectElementToBus(br9_ac1, 1, bus9_ac1);
    net.connectElementToBus(br9_ac1, 2, bus4_ac1);


    ///* ------- 1.8 Add AC Branches (Area 2) ----- */

    // Branch 1: bus 1 - bus 2
    double AC2R1 = 0.01938 * ZbaseAC;
    double AC2X1 = 0.05917 * ZbaseAC;
    std::complex<double> AC2Z1(AC2R1, AC2X1);
    Impedance* br1_ac2 = new Impedance("br1_ac2", "AC2", 3, AC2Z1);
    net.connectElementToBus(br1_ac2, 1, bus1_ac2);
    net.connectElementToBus(br1_ac2, 2, bus2_ac2);

    // Branch 2: bus 1 - bus 5
    double AC2R2 = 0.05403 * ZbaseAC;
    double AC2X2 = 0.22304 * ZbaseAC;
    std::complex<double> AC2Z2(AC2R2, AC2X2);
    Impedance* br2_ac2 = new Impedance("br2_ac2", "AC2", 3, AC2Z2);
    net.connectElementToBus(br2_ac2, 1, bus1_ac2);
    net.connectElementToBus(br2_ac2, 2, bus5_ac2);

    // Branch 3: bus 2 - bus 3
    double AC2R3 = 0.04699 * ZbaseAC;
    double AC2X3 = 0.19797 * ZbaseAC;
    std::complex<double> AC2Z3(AC2R3, AC2X3);
    Impedance* br3_ac2 = new Impedance("br3_ac2", "AC2", 3, AC2Z3);
    net.connectElementToBus(br3_ac2, 1, bus2_ac2);
    net.connectElementToBus(br3_ac2, 2, bus3_ac2);

    // Branch 4: bus 2 - bus 4
    double AC2R4 = 0.05811 * ZbaseAC;
    double AC2X4 = 0.17632 * ZbaseAC;
    std::complex<double> AC2Z4(AC2R4, AC2X4);
    Impedance* br4_ac2 = new Impedance("br4_ac2", "AC2", 3, AC2Z4);
    net.connectElementToBus(br4_ac2, 1, bus2_ac2);
    net.connectElementToBus(br4_ac2, 2, bus4_ac2);

    // Branch 5: bus 2 - bus 5
    double AC2R5 = 0.05695 * ZbaseAC;
    double AC2X5 = 0.17388 * ZbaseAC;
    std::complex<double> AC2Z5(AC2R5, AC2X5);
    Impedance* br5_ac2 = new Impedance("br5_ac2", "AC2", 3, AC2Z5);
    net.connectElementToBus(br5_ac2, 1, bus2_ac2);
    net.connectElementToBus(br5_ac2, 2, bus5_ac2);

    // Branch 6: bus 3 - bus 4
    double AC2R6 = 0.06701 * ZbaseAC;
    double AC2X6 = 0.17103 * ZbaseAC;
    std::complex<double> AC2Z6(AC2R6, AC2X6);
    Impedance* br6_ac2 = new Impedance("br6_ac2", "AC2", 3, AC2Z6);
    net.connectElementToBus(br6_ac2, 1, bus3_ac2);
    net.connectElementToBus(br6_ac2, 2, bus4_ac2);

    // Branch 7: bus 4 - bus 5
    double AC2R7 = 0.01335 * ZbaseAC;
    double AC2X7 = 0.04211 * ZbaseAC;
    std::complex<double> AC2Z7(AC2R7, AC2X7);
    Impedance* br7_ac2 = new Impedance("br7_ac2", "AC2", 3, AC2Z7);
    net.connectElementToBus(br7_ac2, 1, bus4_ac2);
    net.connectElementToBus(br7_ac2, 2, bus5_ac2);

    // Branch 8: bus 4 - bus 7
    double AC2R8 = 0.00000 * ZbaseAC;
    double AC2X8 = 0.20912 * ZbaseAC;
    std::complex<double> AC2Z8(AC2R8, AC2X8);
    Impedance* br8_ac2 = new Impedance("br8_ac2", "AC2", 3, AC2Z8);
    net.connectElementToBus(br8_ac2, 1, bus4_ac2);
    net.connectElementToBus(br8_ac2, 2, bus7_ac2);

    // Branch 9: bus 4 - bus 9
    double AC2R9 = 0.00000 * ZbaseAC;
    double AC2X9 = 0.55618 * ZbaseAC;
    std::complex<double> AC2Z9(AC2R9, AC2X9);
    Impedance* br9_ac2 = new Impedance("br9_ac2", "AC2", 3, AC2Z9);
    net.connectElementToBus(br9_ac2, 1, bus4_ac2);
    net.connectElementToBus(br9_ac2, 2, bus9_ac2);

    // Branch 10: bus 5 - bus 6
    double AC2R10 = 0.00000 * ZbaseAC;
    double AC2X10 = 0.25202 * ZbaseAC;
    std::complex<double> AC2Z10(AC2R10, AC2X10);
    Impedance* br10_ac2 = new Impedance("br10_ac2", "AC2", 3, AC2Z10);
    net.connectElementToBus(br10_ac2, 1, bus5_ac2);
    net.connectElementToBus(br10_ac2, 2, bus6_ac2);

    // Branch 11: bus 6 - bus 11
    double AC2R11 = 0.09498 * ZbaseAC;
    double AC2X11 = 0.19890 * ZbaseAC;
    std::complex<double> AC2Z11(AC2R11, AC2X11);
    Impedance* br11_ac2 = new Impedance("br11_ac2", "AC2", 3, AC2Z11);
    net.connectElementToBus(br11_ac2, 1, bus6_ac2);
    net.connectElementToBus(br11_ac2, 2, bus11_ac2);

    // Branch 12: bus 6 - bus 12
    double AC2R12 = 0.12291 * ZbaseAC;
    double AC2X12 = 0.25581 * ZbaseAC;
    std::complex<double> AC2Z12(AC2R12, AC2X12);
    Impedance* br12_ac2 = new Impedance("br12_ac2", "AC2", 3, AC2Z12);
    net.connectElementToBus(br12_ac2, 1, bus6_ac2);
    net.connectElementToBus(br12_ac2, 2, bus12_ac2);

    // Branch 13: bus 6 - bus 13
    double AC2R13 = 0.06615 * ZbaseAC;
    double AC2X13 = 0.13027 * ZbaseAC;
    std::complex<double> AC2Z13(AC2R13, AC2X13);
    Impedance* br13_ac2 = new Impedance("br13_ac2", "AC2", 3, AC2Z13);
    net.connectElementToBus(br13_ac2, 1, bus6_ac2);
    net.connectElementToBus(br13_ac2, 2, bus13_ac2);

    // Branch 14: bus 7 - bus 8
    double AC2R14 = 0.00000 * ZbaseAC;
    double AC2X14 = 0.17615 * ZbaseAC;
    std::complex<double> AC2Z14(AC2R14, AC2X14);
    Impedance* br14_ac2 = new Impedance("br14_ac2", "AC2", 3, AC2Z14);
    net.connectElementToBus(br14_ac2, 1, bus7_ac2);
    net.connectElementToBus(br14_ac2, 2, bus8_ac2);

    // Branch 15: bus 7 - bus 9
    double AC2R15 = 0.00000 * ZbaseAC;
    double AC2X15 = 0.11001 * ZbaseAC;
    std::complex<double> AC2Z15(AC2R15, AC2X15);
    Impedance* br15_ac2 = new Impedance("br15_ac2", "AC2", 3, AC2Z15);
    net.connectElementToBus(br15_ac2, 1, bus7_ac2);
    net.connectElementToBus(br15_ac2, 2, bus9_ac2);

    // Branch 16: bus 9 - bus 10
    double AC2R16 = 0.03181 * ZbaseAC;
    double AC2X16 = 0.08450 * ZbaseAC;
    std::complex<double> AC2Z16(AC2R16, AC2X16);
    Impedance* br16_ac2 = new Impedance("br16_ac2", "AC2", 3, AC2Z16);
    net.connectElementToBus(br16_ac2, 1, bus9_ac2);
    net.connectElementToBus(br16_ac2, 2, bus10_ac2);

    // Branch 17: bus 9 - bus 14
    double AC2R17 = 0.12711 * ZbaseAC;
    double AC2X17 = 0.27038 * ZbaseAC;
    std::complex<double> AC2Z17(AC2R17, AC2X17);
    Impedance* br17_ac2 = new Impedance("br17_ac2", "AC2", 3, AC2Z17);
    net.connectElementToBus(br17_ac2, 1, bus9_ac2);
    net.connectElementToBus(br17_ac2, 2, bus14_ac2);

    // Branch 18: bus 10 - bus 11
    double AC2R18 = 0.08205 * ZbaseAC;
    double AC2X18 = 0.19207 * ZbaseAC;
    std::complex<double> AC2Z18(AC2R18, AC2X18);
    Impedance* br18_ac2 = new Impedance("br18_ac2", "AC2", 3, AC2Z18);
    net.connectElementToBus(br18_ac2, 1, bus10_ac2);
    net.connectElementToBus(br18_ac2, 2, bus11_ac2);

    // Branch 19: bus 12 - bus 13
    double AC2R19 = 0.22092 * ZbaseAC;
    double AC2X19 = 0.19988 * ZbaseAC;
    std::complex<double> AC2Z19(AC2R19, AC2X19);
    Impedance* br19_ac2 = new Impedance("br19_ac2", "AC2", 3, AC2Z19);
    net.connectElementToBus(br19_ac2, 1, bus12_ac2);
    net.connectElementToBus(br19_ac2, 2, bus13_ac2);

    // Branch 20: bus 13 - bus 14
    double AC2R20 = 0.17093 * ZbaseAC;
    double AC2X20 = 0.34802 * ZbaseAC;
    std::complex<double> AC2Z20(AC2R20, AC2X20);
    Impedance* br20_ac2 = new Impedance("br20_ac2", "AC2", 3, AC2Z20);
    net.connectElementToBus(br20_ac2, 1, bus13_ac2);
    net.connectElementToBus(br20_ac2, 2, bus14_ac2);
    

    ///*  ------ 2.1 Create DC Buses  ---- */
    Bus* bus1_dc = new Bus("DCBUS01", "DC1", 2);
    Bus* bus2_dc = new Bus("DCBUS02", "DC1", 2);
    Bus* bus3_dc = new Bus("DCBUS03", "DC1", 2);

    ///* ------- 2.2 Add DC Branches ----- */

    double ZbaseDC = 500.0 * 500.0 / 100.0;

    // Branch 1: DC bus 1 - DC bus 2
    double DCR1 = 0.052 * ZbaseDC;
    Impedance* br1_dc = new Impedance("br1_dc", "DC1", 2, DCR1);
    net.connectElementToBus(br1_dc, 1, bus1_dc);
    net.connectElementToBus(br1_dc, 2, bus2_dc);

    // Branch 2: DC bus 2 - DC bus 3
    double DCR2 = 0.052 * ZbaseDC;
    Impedance* br2_dc = new Impedance("br2_dc", "DC1", 2, DCR2);
    net.connectElementToBus(br2_dc, 1, bus2_dc);
    net.connectElementToBus(br2_dc, 2, bus3_dc);

    // Branch 3: DC bus 1 - DC bus 3
    double DCR3 = 0.073 * ZbaseDC;
    Impedance* br3_dc = new Impedance("br3_dc", "DC1", 2, DCR3);
    net.connectElementToBus(br3_dc, 1, bus1_dc);
    net.connectElementToBus(br3_dc, 2, bus3_dc);

    ///* ------- 2.3 Add MMC Converters ----- */
    const double omega_nom = 2 * M_PI * 50;
    const double v_ac = 345.0 * 1e3;
    const double v_dc = 500.0 * 1e3;
    const double p_rect = 60.0 * 1e6;
    const double q_rect = 40.0 * 1e6;
    const double p_inv = 35.0 * 1e6;
    const double q_inv = 5.0 * 1e6;

    // MMC1 — rectifier (DC bus 1 ↔ AC1 bus 2): DC P + AC Q control
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
        1, 0, 19.93, 4500, 1, 200.0, // zcc
        1, 0, 117.93, 8.5e4, 2, 800.0, 0, // occ
        1, 0, 19.93, 4500, 2, 0, 0, // ccc
        0 // droop
    };
    MMC* mmc1 = new MMC("MMC1", "AC1", converter_params1, controller_params1);
    net.connectElementToBus(mmc1, 1, bus2_ac1);
    net.connectElementToBus(mmc1, 2, bus1_dc);
    map<string, double> mmc1_info = {
        {"type_dc", 1},
        {"type_ac", 1},
    };
    mmc1->setOPFInfo(mmc1_info);

    // MMC2 — DC slack (DC bus 2 ↔ AC1 bus 5): Vdc + AC voltage control
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
    net.connectElementToBus(mmc2, 1, bus5_ac1);
    net.connectElementToBus(mmc2, 2, bus2_dc);
    map<string, double> mmc2_info = {
        {"type_dc", 2},
        {"type_ac", 2},
    };
    mmc2->setOPFInfo(mmc2_info);

    // MMC3 — inverter (DC bus 3 ↔ AC2 bus 5): DC P + AC Q control
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
        1, 0, 19.93, 4500, 1, -29.16, // zcc
        1, 0, 117.93, 8.5e4, 2, -62.8, 0, // occ
        1, 0, 19.93, 4500, 2, 0, 0, // ccc
        0 // droop
    };
    MMC* mmc3 = new MMC("MMC3", "AC2", converter_params3, controller_params3);
    net.connectElementToBus(mmc3, 1, bus5_ac2);
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
    global_dict["baseMVA"] = 100;
    global_dict["ACbaseKV"] = 345.0; 
    global_dict["DCbaseKV"] = 500.0; 
    global_dict["ACZbase"] =
        global_dict["ACbaseKV"] * global_dict["ACbaseKV"] / global_dict["baseMVA"];
    global_dict["DCZbase"] =
        global_dict["DCbaseKV"] * global_dict["DCbaseKV"] / global_dict["baseMVA"];

    pf.make_OPF(&net, global_dict, true, false, plotting_enabled, true);

    cout << "Press Enter to continue...\n";
    cin.get();

}
