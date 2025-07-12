#include "network.h"
#include "Bus.h"
#include "Include_components.h"
#include "Impedance.h"
#include "solveHmo_opf.h"
#include <iomanip>


int main() {
	// OHL constructor check
	//std::vector<double> distances = { 11.8, 27.5 };
	//std::vector<int> numbers = { 2, 2 };
	//std::vector<double> values_g = { 0.9196, 0.0062, 10.0, 7.5, 6.5 };
	//Overhead_Line* ohl = new Overhead_Line("ohl", 100.0, make_tuple(1.0, 1.0, 1.0),
	//	make_tuple("flat", numbers, distances, 0.01436, 0.06266, 10.0, 0.4572),
	//	make_tuple(2, values_g, 1.0));


	// Transformer constructor check
	

    std::vector<double> transformer_values = { 4.3218, 0.45856, 0.7938, 0.084225, 1.0804e+06, 3000, 2.0, 0.0 }; // R_primary, L_primary, R_secondary, L_secondary, Mutual inductance
    TransformerYY_real* transformerYY = new TransformerYY_real("T3", 3, transformer_values);
    transformerYY->writeFile(10, 10000000, 100000);

	//// TL constructor check
	//std::vector<double> transmission_line_values = { 0.01, 2.5e-7, 1e-9, 1e-11, 1000 };
	//TransmissionLine* transmission_line = new TransmissionLine("tl", 1, transmission_line_values);
	//transmission_line->writeFile(10, 10000000, 1000);


	// Generator constructor check
	//std::vector<double> generator_values = { 1.0, 0.01, 1.0, 0.1 };
	//Generator* g = new Generator("gen", 1, generator_values);

	Network* myNetwork = new Network();

	//vector<Bus*> start_buses;
	//vector<Bus*> end_buses;
	//vector<Element*> skip_elements;

	//start_buses.push_back(bus1);
	//end_buses.push_back(gnd);
	//skip_elements.push_back(ac);


	//Eigen::MatrixXcd equivalent_impedance;
	//myNetwork->compute_equivalent_impedance(start_buses, end_buses, skip_elements);


	//// Numerically computes the Jacobian matrices A = ∂f/∂x and B = ∂f/∂u at a specified operating point
	//double Fnom = 50;
	//double Vnom_sec = 333e3;
	//double Pnom = 1000e6;
	//double Vnom_dc = 640e3;
	//double Nb_PM = 36;
	//double C_PM = 1.758e-3;

	//double Zbase = Vnom_sec * Vnom_sec / Pnom;
	//double Larm_pu = 0.15;
	//double Rarm_pu = Larm_pu / 100;

	//double Larm = Larm_pu * (Zbase / (2 * M_PI * Fnom));
	//double Rarm = Rarm_pu * Zbase;

	//double w = 2 * M_PI * Fnom;

	//MMC mmc("MMC1", w, 0, 0, 0,
	//	-Pnom, Pnom, -Pnom, Pnom,  // Power limits
	//	0, Vnom_sec, Vnom_dc,       // Angle, AC/DC voltages
	//	Larm, Rarm, C_PM / Nb_PM,     // Arm parameters
	//	Nb_PM, 0.0, 0.0, 150e-6);   // Submodules, reactor, delay


	//// Nonlinear with Harmonic Injection
	//std::cout << "\nNonlinear Analysis with Harmonics: \n";

	//// Define operating point
	//Eigen::VectorXd x0 = Eigen::VectorXd::Zero(6);  // [ip1,ip2,ip3,in1,in2,in3]
	//x0 << 200e3, 0, 15e3, 20e3, 20e3, 20e3;

	//Eigen::VectorXd u0(8);                          // [VD1,VD2,VS1-VS6]
	//u0 << 0, 0, 400, 400, -400, 400, -400, 400;
	//double t = 5.0;

	//// Numerical Jacobian
	//mmc.computeJacobianNumerically(x0, u0);
	//std::cout << "\nA (numerical):\n" << mmc.getA() << "\n";
	//std::cout << "\nB (numerical):\n" << mmc.getB() << "\n";

	//// Admittance matrix
	//std::complex<double> s = std::complex<double>(0, 2 * M_PI * Fnom);
	//Eigen::MatrixXcd Y = mmc.computeAdmittanceMatrix(s);

	//// Print the admittance matrix
	//std::cout << "Admittance Matrix: " << Fnom << "):\n" << Y << std::endl;

	//// Equilibrium Solution
	//std::cout << "\nEquilibrium Solution: \n";
	//mmc.solveEquilibrium();
	//const Eigen::VectorXd x_eq = mmc.getEquilibriumState();
	//std::cout << "Equilibrium state:\n" << x_eq.transpose() << "\n";

	//// Verify equilibrium
	//const Eigen::VectorXd dx_eq = mmc.computeStateDerivatives(x_eq.head(6), u0);
	//std::cout << "||dx|| at equilibrium: " << dx_eq.norm() << "\n";

	//mmc.printEigenvalues();  // Display eigenvalues


	// state_space
	Bus* bus0 = new Bus("gnd", 1);
	Bus* bus1 = new Bus("1", 1);
	Bus* bus2 = new Bus("2", 1); // Bus for capacitor

    // Add buses to network
    myNetwork->addBus(bus0->getBusName(), bus0);
    myNetwork->addBus(bus1->getBusName(), bus1);
	myNetwork->addBus(bus2->getBusName(), bus2); // Add bus2 for capacitor

    // Create and add elements
	AC_source* ac = new AC_source("AC1", 1, {0.0});
	myNetwork->addElement(ac);
	myNetwork->connectElementToBus(ac, 1, bus1); // Connect AC source to bus1
	myNetwork->connectElementToBus(ac, 2, bus0); // Connect AC source to ground bus 

    Resistor* r1 = new Resistor("R1", 1, { 2.0 });
    myNetwork->addElement(r1);
	myNetwork->connectElementToBus(r1, 1, bus1); // Connect resistor to bus1
	myNetwork->connectElementToBus(r1, 2, bus2); // Connect resistor to ground bus

    // Add capacitor as you showed
    Capacitor* c1 = new Capacitor("C1", 1, { 1e-6 }); // Name, pins, capacitance
    myNetwork->addElement(c1);
	myNetwork->connectElementToBus(c1, 1, bus2); // Connect capacitor to bus2
	myNetwork->connectElementToBus(c1, 2, bus0); // Connect capacitor to ground bus

	// Create the StateSpaceModel object
	StateSpaceModel model;
    model.formState(myNetwork);
	//model.formState_symbolic(myNetwork);

    // Print matrices
    std::cout << "A = \n" << model.getA() << "\n";
    //std::cout << "B = \n" << model.getB() << "\n";
    //std::cout << "C = \n" << model.getC() << "\n";
    //std::cout << "D = \n" << model.getD() << "\n";


//	//Haixiao
//	try {
//        ///* ---------- 0 Set Network Object ---------- */
//        Network net;
//        const auto& data = net.getNetData();
//
//        ///* ---------- 1.1 Create AC Buses ---------- */
//        Bus* bus1_ac = new Bus("ACBUS01", 3);
//        Bus* bus2_ac = new Bus("ACBUS02", 3);
//        Bus* bus3_ac = new Bus("ACBUS03", 3);
//        Bus* bus4_ac = new Bus("ACBUS04", 3);
//        Bus* bus5_ac = new Bus("ACBUS05", 3);
//        std::vector<std::vector<std::string>> dict_ac;
//        net.addBusAC(dict_ac, { "ACBUS01", "1", "100", "345", "1.1", "0.9" }, false);
//        net.addBusAC(dict_ac, { "ACBUS02", "1", "100", "345", "1.1", "0.9" }, false);
//        net.addBusAC(dict_ac, { "ACBUS03", "1", "100", "345", "1.1", "0.9" }, false);
//        net.addBusAC(dict_ac, { "ACBUS04", "1", "100", "345", "1.1", "0.9" }, false);
//        net.addBusAC(dict_ac, { "ACBUS05", "1", "100", "345", "1.1", "0.9" }, false);
//
//        /* ---------- 1.2 Add AC Loads  ---------- */
//        std::vector<double> load_params1 = { 1, 1, 1 };
//        Load* load1 = new Load("LOAD01", 3, load_params1);
//        net.connectElementToBus(load1, 1, bus1_ac);
//
//        std::vector<double> load_params2 = { 1, 1, 1 };
//        Load* load2 = new Load("LOAD02", 3, load_params2);
//        net.connectElementToBus(load2, 1, bus2_ac);
//
//        Network* myNetwork = new Network();
//
//        std::vector<double> load_params3 = { 1, 1, 1 };
//        Load* load3 = new Load("LOAD03", 3, load_params3);
//        net.connectElementToBus(load3, 1, bus3_ac);
//
//        std::vector<double> load_params4 = { 1, 1, 1 };
//        Load* load4 = new Load("LOAD04", 3, load_params4);
//        net.connectElementToBus(load4, 1, bus4_ac);
//
//        std::vector<double> load_params5 = { 1, 1, 1 };
//        Load* load5 = new Load("LOAD05", 3, load_params4);
//        net.connectElementToBus(load5, 1, bus5_ac);
//
//        std::vector<std::vector<std::string>> dict_load;
//        std::vector<std::string> load_info1 = {
//            "Load01",   // 0  load_name
//            "1",        // 1  grid_area
//            "345",      // 2  rated_voltage_kv [kV]
//            "0",      // 3  R [Ω]
//            "0",        // 4  L [H]
//            "0",        // 5  C [F]
//        };
//        net.make_Load(load1, load_info1, false);
//
//        std::vector<std::string> load_info2 = {
//            "Load02",    // 0  load_name
//            "1",         // 1  grid_area
//            "345",       // 2  rated_voltage_kv [kV]
//            "5950",      // 3  R [Ω]
//            "37.9",      // 4  L [H]
//            "0",         // 5  C [F]
//        };
//        net.make_Load(load2, load_info2, false);
//
//        std::vector<std::string> load_info3 = {
//         "Load03",    // 0  load_name
//         "1",         // 1  grid_area
//         "345",       // 2  rated_voltage_kv [kV]
//         "2650",      // 3  R [Ω]
//         "25.2",      // 4  L [H]
//         "0",         // 5  C [F]
//        };
//        net.make_Load(load3, load_info3, false);
//
//        std::vector<std::string> load_info4 = {
//          "Load04",    // 0  load_name
//          "1",         // 1  grid_area
//          "345",       // 2  rated_voltage_kv [kV]
//          "2976",      // 3  R [Ω]
//          "75.7",      // 4  L [H]
//          "0",         // 5  C [F]
//        };
//        net.make_Load(load4, load_info4, false);
//
//        std::vector<std::string> load_info5 = {
//            "Load05",    // 0  load_name
//            "1",         // 1  grid_area
//            "345",       // 2  rated_voltage_kv [kV]
//            "1984",      // 3  R [Ω]
//            "37.9",      // 4  L [H]
//            "0",         // 5  C [F]
//        };
//        net.make_Load(load5, load_info5, false);
//
//        /*  ---------- 1.3 Add AC Generators  ---------- */
//        std::vector<double> gen1_params = { 0.02, 0.3, 0.05, 7.0 };
//        Generator* gen1 = new Generator("GEN01", 3, gen1_params);
//        net.connectElementToBus(gen1, 1, bus1_ac);
//
//        std::vector<double> gen2_params = { 0.02, 0.3, 0.05, 7.0 };
//        Generator* gen2 = new Generator("GEN02", 3, gen2_params);
//        net.connectElementToBus(gen2, 1, bus2_ac);
//
//        std::vector<std::vector<std::string>> dict_gen;
//
//        std::vector<std::string> gen_info1 = {
//            "GEN01",    // 0  generator_name
//            "1",        // 1  grid_area
//            "345",      // 2  rated_voltage_kv [kV]
//            "200",      // 3  P_max  [MW]
//            " 10",      // 4  P_min  [MW]
//            "84",       // 5  Q_max  [MVAr]
//            "84",       // 6  Q_min  [MVAr]
//            "0.11",     // 7  cost_quadratic_coeff
//            "50",       // 8  cost_linear_coeff
//            "150"       // 9  cost_constant_coeff
//        };
//        net.make_Generator(gen1, gen_info1, false);
//
//        std::vector<std::string> gen_info2 = {
//            "GEN02",    // 0  generator_name
//            "1",        // 1  grid_area
//            "345",      // 2  rated_voltage_kv [kV]
//            "40",       // 3  P_max  [MW]
//            "40",       // 4  P_min  [MW]
//            "-31",      // 5  Q_max  [MVAr]
//            "-33",      // 6  Q_min  [MVAr]
//            "0.085",    // 7  cost_quadratic_coeff
//            "1.2",      // 8  cost_linear_coeff
//            "600"       // 9  cost_constant_coeff
//        };
//        net.make_Generator(gen2, gen_info2, false);
//
//        ///*  ---------- 1.4 Add Branches  ---------- */
//        // AC Branches
//        std::map<std::string, double> globals = {
//         {"omega", 2 * M_PI * 50},
//         {"Z_base", 1.0}
//        };
//        std::vector<std::vector<std::string>> dict_br_ac;
//
//        DenseMatrix ACZ1(1, 1);
//        double ACR1 = 0.02;
//        double ACX1 = 0.06;
//        RCP<const Basic> ACZsym1 = real_double(ACR1);
//        ACZ1.set(0, 0, ACZsym1);
//        Impedance* br1_ac = new Impedance("br1_ac", 3, ACZ1);
//        net.connectElementToBus(br1_ac, /*terminal=*/1, bus1_ac);
//        net.connectElementToBus(br1_ac, /*terminal=*/2, bus2_ac);
//        std::vector<std::string> info_br1_ac = { "ACBR1", "1", "0.06" };
//        net.make_BranchAC(br1_ac, globals, info_br1_ac, false);
//
//        DenseMatrix ACZ2(1, 1);
//        double ACR2 = 0.08;
//        double ACX2 = 0.24;
//        RCP<const Basic> ACZsym2 = real_double(ACR2);
//        ACZ2.set(0, 0, ACZsym2);
//        Impedance* br2_ac = new Impedance("br2_ac", 3, ACZ2);
//        net.connectElementToBus(br2_ac, /*terminal=*/1, bus1_ac);
//        net.connectElementToBus(br2_ac, /*terminal=*/2, bus3_ac);
//        std::vector<std::string> info_br2_ac = { "ACBR2", "1", "0.05" };
//        net.make_BranchAC(br2_ac, globals, info_br2_ac, false);
//
//        DenseMatrix ACZ3(1, 1);
//        double ACR3 = 0.06;
//        double ACX3 = 0.18;
//        // RCP<const Basic> ACZsym3 = add(real_double(ACR3),
//        //    mul(I, real_double(ACX3)));
//        RCP<const Basic> ACZsym3 = real_double(ACR3);
//        ACZ3.set(0, 0, ACZsym3);
//        Impedance* br3_ac = new Impedance("br3_ac", 3, ACZ3);
//        net.connectElementToBus(br3_ac, /*terminal=*/1, bus2_ac);
//        net.connectElementToBus(br3_ac, /*terminal=*/2, bus3_ac);
//        std::vector<std::string> info_br3_ac = { "ACBR3", "1", "0.04" };
//        net.make_BranchAC(br3_ac, globals, info_br3_ac, false);
//
//        DenseMatrix ACZ4(1, 1);
//        double ACR4 = 0.06;
//        double ACX4 = 0.18;
//        // RCP<const Basic> ACZsym4 = add(real_double(ACR4),
//        //    mul(I, real_double(ACX4)));
//        RCP<const Basic> ACZsym4 = real_double(ACR4);
//        ACZ4.set(0, 0, ACZsym4);
//        Impedance* br4_ac = new Impedance("br4_ac", 3, ACZ4);
//        net.connectElementToBus(br4_ac, /*terminal=*/1, bus2_ac);
//        net.connectElementToBus(br4_ac, /*terminal=*/2, bus4_ac);
//        std::vector<std::string> info_br4_ac = { "ACBR4", "1", "0.04" };
//        net.make_BranchAC(br4_ac, globals, info_br4_ac, false);
//
//        DenseMatrix ACZ5(1, 1);
//        double ACR5 = 0.04;
//        double ACX5 = 0.12;
//        // RCP<const Basic> ACZsym5 = add(real_double(ACR5),
//        //    mul(I, real_double(ACX5)));
//        RCP<const Basic> ACZsym5 = real_double(ACR5);
//        ACZ5.set(0, 0, ACZsym5);
//        Impedance* br5_ac = new Impedance("br5_ac", 3, ACZ5);
//        net.connectElementToBus(br5_ac, /*terminal=*/1, bus2_ac);
//        net.connectElementToBus(br5_ac, /*terminal=*/2, bus5_ac);
//        std::vector<std::string> info_br5_ac = { "ACBR5", "1", "0.03" };
//        net.make_BranchAC(br5_ac, globals, info_br5_ac, false);
//
//        DenseMatrix ACZ6(1, 1);
//        double ACR6 = 0.01;
//        double ACX6 = 0.03;
//        // RCP<const Basic> ACZsym6 = add(real_double(ACR6),
//        //    mul(I, real_double(ACX6)));
//        RCP<const Basic> ACZsym6 = real_double(ACR6);
//        ACZ6.set(0, 0, ACZsym6);
//        Impedance* br6_ac = new Impedance("br6_ac", 3, ACZ6);
//        net.connectElementToBus(br6_ac, /*terminal=*/1, bus3_ac);
//        net.connectElementToBus(br6_ac, /*terminal=*/2, bus4_ac);
//        std::vector<std::string> info_br6_ac = { "ACBR6", "1", "0.02" };
//        net.make_BranchAC(br6_ac, globals, info_br6_ac, false);
//
//        
//        DenseMatrix ACZ7(1, 1);
//        double ACR7 = 0.08;
//        double ACX7 = 0.24;
//        //RCP<const Basic> ACZsym7 = add(real_double(ACR7),
//        //    mul(I, real_double(ACX7)));
//        RCP<const Basic> ACZsym7 = real_double(ACR7);
//        ACZ7.set(0, 0, ACZsym7);
//        Impedance* br7_ac = new Impedance("br7_ac", 3, ACZ7);
//        net.connectElementToBus(br7_ac, /*terminal=*/1, bus4_ac);
//        net.connectElementToBus(br7_ac, /*terminal=*/2, bus5_ac);
//        std::vector<std::string> info_br7_ac = { "ACBR7", "1", "0.05" };
//        net.make_BranchAC(br7_ac, globals, info_br7_ac, false);
//
//        /*  ---------- 2.1 Create DC Buses  ---------- */
//        Bus* bus1_dc = new Bus("DCBUS01", 1);
//        Bus* bus2_dc = new Bus("DCBUS02", 1);
//        Bus* bus3_dc = new Bus("DCBUS03", 1);
//        std::vector<std::vector<std::string>> dict_dc;
//        net.addBusDC(dict_dc, { "DCBUS01", "330", "1.1", "0.9" }, false);
//        net.addBusDC(dict_dc, { "DCBUS02", "330", "1.1", "0.9" }, false);
//        net.addBusDC(dict_dc, { "DCBUS03", "330", "1.1", "0.9" }, false);
//
//        ///*  ---------- 2.2 Create DC Buses  ---------- */
//
//        DenseMatrix DCZ1(1, 1);
//        double DCR1 = 0.052;
//        RCP<const Basic> DCZsym1 = real_double(DCR1);
//        DCZ1.set(0, 0, DCZsym1);
//        Impedance* br1_dc = new Impedance("br1_dc", 1, DCZ1);
//        net.connectElementToBus(br1_dc, /*terminal=*/1, bus1_dc);
//        net.connectElementToBus(br1_dc, /*terminal=*/2, bus2_dc);
//        std::vector<std::string> info_br1_dc = { "DCBR1" };
//        net.make_BranchDC(br1_dc, globals, info_br1_dc, false);
//
//        DenseMatrix DCZ2(1, 1);
//        double DCR2 = 0.073;
//        RCP<const Basic> DCZsym2 = real_double(DCR2);
//        DCZ2.set(0, 0, DCZsym2);
//        Impedance* br2_dc = new Impedance("br2_dc", 1, DCZ2);
//        net.connectElementToBus(br2_dc, /*terminal=*/1, bus1_dc);
//        net.connectElementToBus(br2_dc, /*terminal=*/2, bus3_dc);
//        std::vector<std::string> info_br2_dc = { "DCBR2" };
//        net.make_BranchDC(br2_dc, globals, info_br2_dc, false);
//
//        DenseMatrix DCZ3(1, 1);
//        double DCR3 = 0.052;
//        RCP<const Basic> DCZsym3 = real_double(DCR3);
//        DCZ3.set(0, 0, DCZsym3);
//        Impedance* br3_dc = new Impedance("br3_dc", 1, DCZ3);
//        net.connectElementToBus(br3_dc, /*terminal=*/1, bus2_dc);
//        net.connectElementToBus(br3_dc, /*terminal=*/2, bus3_dc);
//        std::vector<std::string> info_br3_dc = { "DCBR3" };
//        net.make_BranchDC(br3_dc, globals, info_br3_dc, false);
//
//        /*  ---------- 2.3 Create Converters ---------- */
//        MMC* mmc1 = new MMC(
//            "MMC1",         // Symbol
//            1000.0,         // Omega (Nominal Frequency in rad/s)
//            100.0,          // Active Power (P) in MW
//            50.0,           // Reactive Power (Q) in MVA
//            500.0,          // DC Power (P_dc) in kW
//            -100.0,         // Min Active Power (P_min) in MW
//            200.0,          // Max Active Power (P_max) in MW
//            -50.0,          // Min Reactive Power (Q_min) in MVA
//            100.0,          // Max Reactive Power (Q_max) in MVA
//            0.0,            // Theta (Voltage Angle in rad)
//            330.0,          // AC Voltage (V_m) in kV
//            640.0,          // DC Voltage (V_dc) in kV
//            0.05,           // Arm Inductance (L_arm) in H
//            1.07,           // Arm Resistance (R_arm) in Ω
//            0.01,           // Capacitance per Submodule (C_arm) in F
//            400,            // Number of Submodules (N)
//            0.06,           // Reactor Inductance (L_reactor) in H
//            0.535,          // Reactor Resistance (R_reactor) in Ω
//            0.00015         // Time Delay (t_delay) in seconds
//        );
//        net.connectElementToBus(mmc1, 1, bus2_ac);
//        net.connectElementToBus(mmc1, 2, bus1_dc);
//
//        std::vector<std::string> info_conv1 = {
//         "MMC1",          // 0  generator_name
//         "1",             // 1  grid_area
//         "1",             // 2  type_dc 
//         "1",             // 3  type_ac
//         "0.0015",        // 4  rftc
//         "0.1121",        // 5  xtfc
//         "0.0887",        // 6  bf
//         "0.0001",        // 7  rc
//         "0.16428",       // 8  xc
//         "345",           // 9  basekVac
//         "1.1",           // 10 Vmmax
//         "0.9",           // 11 Vmmin
//         "1.2",           // 12 Imax
//         "1.103",         // 13 LossAC
//         "0.887",         // 14 LossB
//         "2.885",         // 15 LossCrec
//         "4.371",         // 16 LossCinv
//         "0",             // 17 droop
//         "0",             // 18 Pdcset
//         "0",             // 19 Vdcset
//         "0",             // 20 Dvdsetc
//        };
//        net.make_Converter(mmc1, globals, info_conv1, false);
//
//
//        MMC* mmc2 = new MMC(
//            "MMC2",         // Symbol
//            1000.0,         // Omega (Nominal Frequency in rad/s)
//            100.0,          // Active Power (P) in MW
//            50.0,           // Reactive Power (Q) in MVA
//            500.0,          // DC Power (P_dc) in kW
//            -100.0,         // Min Active Power (P_min) in MW
//            200.0,          // Max Active Power (P_max) in MW
//            -50.0,          // Min Reactive Power (Q_min) in MVA
//            100.0,          // Max Reactive Power (Q_max) in MVA
//            0.0,            // Theta (Voltage Angle in rad)
//            330.0,          // AC Voltage (V_m) in kV
//            640.0,          // DC Voltage (V_dc) in kV
//            0.05,           // Arm Inductance (L_arm) in H
//            1.07,           // Arm Resistance (R_arm) in Ω
//            0.01,           // Capacitance per Submodule (C_arm) in F
//            400,            // Number of Submodules (N)
//            0.06,           // Reactor Inductance (L_reactor) in H
//            0.535,          // Reactor Resistance (R_reactor) in Ω
//            0.00015         // Time Delay (t_delay) in seconds
//        );
//        net.connectElementToBus(mmc2, 1, bus3_ac);
//        net.connectElementToBus(mmc2, 2, bus2_dc);
//
//        std::vector<std::string> info_conv2 = {
//         "MMC2",          // 0  generator_name
//         "1",             // 1  grid_area
//         "2",             // 2  type_dc 
//         "2",             // 3  type_ac
//         "0.0015",        // 4  rftc
//         "0.1121",        // 5  xtfc
//         "0.0887",        // 6  bf
//         "0.0001",        // 7  rc
//         "0.16428",       // 8  xc
//         "345",           // 9  basekVac
//         "1.1",           // 10 Vmmax
//         "0.9",           // 11 Vmmin
//         "1.2",           // 12 Imax
//         "1.103",         // 13 LossAC
//         "0.887",         // 14 LossB
//         "2.885",         // 15 LossCrec
//         "4.371",         // 16 LossCinv
//         "0",             // 17 droop
//         "0",             // 18 Pdcset
//         "0",             // 19 Vdcset
//         "0",             // 20 Dvdsetc
//        };
//        net.make_Converter(mmc2, globals, info_conv2, false);
//
//
//        MMC* mmc3 = new MMC(
//            "MMC3",         // Symbol
//            1000.0,         // Omega (Nominal Frequency in rad/s)
//            100.0,          // Active Power (P) in MW
//            50.0,           // Reactive Power (Q) in MVA
//            500.0,          // DC Power (P_dc) in kW
//            -100.0,         // Min Active Power (P_min) in MW
//            200.0,          // Max Active Power (P_max) in MW
//            -50.0,          // Min Reactive Power (Q_min) in MVA
//            100.0,          // Max Reactive Power (Q_max) in MVA
//            0.0,            // Theta (Voltage Angle in rad)
//            330.0,          // AC Voltage (V_m) in kV
//            640.0,          // DC Voltage (V_dc) in kV
//            0.05,           // Arm Inductance (L_arm) in H
//            1.07,           // Arm Resistance (R_arm) in Ω
//            0.01,           // Capacitance per Submodule (C_arm) in F
//            400,            // Number of Submodules (N)
//            0.06,           // Reactor Inductance (L_reactor) in H
//            0.535,          // Reactor Resistance (R_reactor) in Ω
//            0.00015         // Time Delay (t_delay) in seconds
//        );
//        net.connectElementToBus(mmc3, 1, bus5_ac);
//        net.connectElementToBus(mmc3, 2, bus3_dc);
//
//        std::vector<std::string> info_conv3 = {
//         "MMC3",          // 0  generator_name
//         "1",             // 1  grid_area
//         "1",             // 2  type_dc 
//         "1",             // 3  type_ac
//         "0.0015",        // 4  rftc
//         "0.1121",        // 5  xtfc
//         "0.0887",        // 6  bf
//         "0.0001",        // 7  rc
//         "0.16428",       // 8  xc
//         "345",           // 9  basekVac
//         "1.1",           // 10 Vmmax
//         "0.9",           // 11 Vmmin
//         "1.2",           // 12 Imax
//         "1.103",         // 13 LossAC
//         "0.887",         // 14 LossB
//         "2.885",         // 15 LossCrec
//         "4.371",         // 16 LossCinv
//         "0",             // 17 droop
//         "0",             // 18 Pdcset
//         "0",             // 19 Vdcset
//         "0",             // 20 Dvdsetc
//        };
//        net.make_Converter(mmc3, globals, info_conv3, false);
//
//        /*----- 3 OPF Implementatiopn ----- */
//        net.make_OPF(net, true, false, false);
//        return EXIT_SUCCESS;
//    }
//    
//    catch (const std::exception& e) {
//        std::cerr << "Error: " << e.what() << std::endl;
//#ifdef _DEBUG
//        system("pause");
//#endif
//        return EXIT_FAILURE;
//    }
//#ifdef _DEBUG
//    system("pause");
//#endif



	// Cleanup
	delete myNetwork;
    delete bus0;
    delete bus1;
    delete ac;
    delete r1;
    delete c1;
	return 0;

}
