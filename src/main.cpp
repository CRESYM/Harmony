#include "network.h"
#include "Bus.h"
#include "Include_components.h"

int main() {
	//Create Transformer object with 1 pin
	//std::vector<double> transformer_values = { 10.0, 5.0, 12.0, 6.0, 2.0, 0.0 }; // R_primary, X_primary, R_secondary, X_secondary, Turns Ratio
	//Transformer_real* transformer = new Transformer_real("T1", 1, transformer_values);
	//transformer->printElementValues();

	// OHL constructor check
	//std::vector<double> distances = { 11.8, 27.5 };
	//std::vector<int> numbers = { 2, 2 };
	//std::vector<double> values_g = { 0.9196, 0.0062, 10.0, 7.5, 6.5 };
	//Overhead_Line* ohl = new Overhead_Line("ohl", 100.0, make_tuple(1.0, 1.0, 1.0),
	//	make_tuple("flat", numbers, distances, 0.01436, 0.06266, 10.0, 0.4572),
	//	make_tuple(2, values_g, 1.0));


	// Transformer constructor check
	//std::vector<double> transformer_values = { 1.0, 1.0, 2.0, 2.0, 2.0 }; // R_primary, X_primary, R_secondary, X_secondary, Turns Ratio
	//TransformerDeltaY* transformer = new TransformerDeltaY("T1", 3, transformer_values);
	//transformer->printElementValues();

	// TL constructor check
	//std::vector<double> transmission_line_values = { 0.01, 2.5e-7, 1e-9, 1e-11, 1000 };
	//TransmissionLine* t = new TransmissionLine("tl", 3, transmission_line_values);


	// Generator constructor check
	//std::vector<double> generator_values = { 1.0, 0.01, 1.0, 0.1 };
	//Generator* g = new Generator("gen", 1, generator_values);

	//std::vector<double> vec = {1,1,1}; 
	//Load* load = new Load("l1", 3, vec);
	//DenseMatrix Y = load->compute_y_parameters();

	//Admittance* y = new Admittance("y1", 1, DenseMatrix(1, 1, { mul(j, omega)}));
	//std::cout << "Admitance" << std::endl;
	//y->printElementInfo();
	//Impedance* y = new Impedance("z1", 1, DenseMatrix(1, 1, { div(integer(1), mul(j, omega)) }));
	//AC_source* ac = new AC_source("ac", 1, DenseMatrix(1, 1, { integer(10) }));

	//Network* myNetwork = new Network();

	// Create Bus objects
	//Bus* b1 = new Bus("Bus1", 1);
	//Bus* gnd = new Bus("gnd", 1);

	// Add elements to the network
    //myNetwork->addElement(g);
	//myNetwork->addElement(ac);

	// Add buses to the network
	//myNetwork->addBus("Bus1", b1);
	//myNetwork->addBus("gnd", gnd);
	//std::cout << "connection 0\n";


	// Connect elements to buses
	//myNetwork->connectElementToBus(g, 1, b1);
	//std::cout << "connection 1\n";
	//myNetwork->connectElementToBus(g, 2, gnd);
	//std::cout << "connection 2\n";

	//myNetwork->connectElementToBus(ac, 1, b1);
	//std::cout << "connection 3\n";

	//myNetwork->connectElementToBus(ac, 2, gnd);
	//std::cout << "connection4\n";


	//std::cout << "Connecting y to bus1 and gnd...\n";
	//std::cout << "Input pins: " << g->getInputPins()
	//	<< ", Output pins: " << g->getOutputPins()
	//	<< ", bus1 pins: " << b1->getPinNumber()
	//	<< ", gnd pins: " << gnd->getPinNumber() << std::endl;

	
	//std::cout << "[Debug] Calling printConnectedElements() for Bus1\n";
	//b1->printConnectedElements();


	// Print the connections to verify the network
	//myNetwork->printConnections();


	//vector<Bus*> start_buses;
	//vector<Bus*> end_buses;
	//vector<Element*> skip_elements;

	//start_buses.push_back(bus1);
	//end_buses.push_back(gnd);
	//skip_elements.push_back(ac);

	////y->writeFile(10.0, 1000.0, 3);
	//double omega_num = 1000;
	//MatrixXcd Yparam = y->compute_y_parameters_num(omega_num);

	//cout << Yparam << endl;

	//Eigen::MatrixXcd equivalent_impedance;
	//myNetwork->compute_equivalent_impedance(start_buses, end_buses, skip_elements);
	
	//try {
	//	// Compute equivalent impedance
	//	myNetwork->compute_equivalent_impedance_nums(start_buses, end_buses, skip_elements, omega_num);
	//}
	//catch (const std::exception& e) {
	//	std::cerr << "Error during impedance computation: " << e.what() << std::endl;
	//	delete myNetwork;
	//	return EXIT_FAILURE;
	//}
	
	//Create and Test MMC**
	//MMC* myMMC = new MMC(
	//	"MMC1",         // Symbol
	//	2,              // Number of input pins
	//	2,              // Number of output pins
	//	1000.0,         // Omega (Nominal Frequency in rad/s)
	//	100.0,          // Active Power (P) in MW
	//	50.0,           // Reactive Power (Q) in MVA
	//	500.0,          // DC Power (P_dc) in kW
	//	-100.0,         // Min Active Power (P_min) in MW
	//	200.0,          // Max Active Power (P_max) in MW
	//	-50.0,          // Min Reactive Power (Q_min) in MVA
	//	100.0,          // Max Reactive Power (Q_max) in MVA
	//	0.0,            // Theta (Voltage Angle in rad)
	//	330.0,          // AC Voltage (V_m) in kV
	//	640.0,          // DC Voltage (V_dc) in kV
	//	0.05,           // Arm Inductance (L_arm) in H
	//	1.07,           // Arm Resistance (R_arm) in Ω
	//	0.01,           // Capacitance per Submodule (C_arm) in F
	//	400,            // Number of Submodules (N)
	//	0.06,           // Reactor Inductance (L_reactor) in H
	//	0.535,          // Reactor Resistance (R_reactor) in Ω
	//	0.00015         // Time Delay (t_delay) in seconds
	//);

	//myNetwork->addElement(myMMC);

	//// Example MMC parameters (must match Controller constructor)
	//std::vector<double> converter_params = {
	//	1000.0, 100.0, 50.0, 500.0, -100.0, 200.0, -50.0, 100.0, 314.16, 400,
	//	1.07, 0.05, 0.01, 0.535, 0.06, 0.00015
	//};

	//std::cout << "Converter params size: " << converter_params.size() << std::endl;

	//try {
	//	myMMC->init_Controller(converter_params);
	//	std::cout << "MMC Controller initialized successfully!\n";
	//}
	//catch (const std::exception& e) {
	//	std::cerr << "Error initializing MMC Controller: " << e.what() << std::endl;
	//	delete myMMC;
	//	delete myNetwork;
	//	return EXIT_FAILURE;
	//}

	// Numerically computes the Jacobian matrices A = ∂f/∂x and B = ∂f/∂u at a specified operating point
	double Fnom = 50;
    double Vnom_sec = 333e3;
    double Pnom = 1000e6;
    double Vnom_dc = 640e3;
    double Nb_PM = 36;
    double C_PM = 1.758e-3;

    double Zbase = Vnom_sec * Vnom_sec / Pnom;
    double Larm_pu = 0.15;
    double Rarm_pu = Larm_pu / 100;

    double Larm = Larm_pu * (Zbase / (2 * M_PI * Fnom));
    double Rarm = Rarm_pu * Zbase;

    double w = 2 * M_PI * Fnom;

    MMC mmc("MMC1", 2, 2, w, 0, 0, 0,
		-Pnom, Pnom, -Pnom, Pnom,  // Power limits
	    0, Vnom_sec, Vnom_dc,       // Angle, AC/DC voltages
	    Larm, Rarm, C_PM / Nb_PM,     // Arm parameters
	    Nb_PM, 0.0, 0.0, 150e-6);   // Submodules, reactor, delay
   
    // Linear Jacobian Analysis
    std::cout << "=== Linear Jacobian Analysis ===\n"; 
    mmc.computeJacobianLinear();

    Eigen::MatrixXd A_lin = mmc.getA();
    Eigen::MatrixXd B_lin = mmc.getB();

    // Print A matrix
    std::cout << "A = \n" << A_lin << "\n\n";

    // Print B matrix
    std::cout << "B = \n" << B_lin << "\n\n"; 
   
    // Eigenvalue analysis
    Eigen::EigenSolver<Eigen::MatrixXd> es(A_lin);
    std::cout << "Eigenvalues (linear):\n" << es.eigenvalues() << "\n";
    mmc.checkStability();

    // Nonlinear with Harmonic Injection
    std::cout << "\nNonlinear Analysis with Harmonics: \n";
    // Assign nonlinear parameters to match test case
    double Rm = 10.0;
    double Rp = Rarm;  // Equal to R_arm used in MMC
    double Rn = Rarm;

    mmc.setRarmPositive(Rp);
    mmc.setRarmNegative(Rn);
    mmc.setRarmMutual(Rm);

    // Set 2nd harmonic amplitudes and phases 
    mmc.setSecondHarmonicInjection(5000.0, 3000.0, 2*M_PI/3, 2*M_PI/3, 6*M_PI/3);

    // Define operating point
    Eigen::VectorXd x0 = Eigen::VectorXd::Zero(6);  // [ip1,ip2,ip3,in1,in2,in3]
    x0 << 200e3, 0, 15e3, 20e3, 20e3, 20e3;

    Eigen::VectorXd u0(8);                          // [VD1,VD2,VS1-VS6]
    u0 << 0, 0, 400, 400, -400, 400, -400, 400;
    double t = 5.0;

    // Numerical Jacobian
    mmc.computeJacobianNumerically(x0, u0);
    std::cout << "\nA (numerical):\n" << mmc.getA() << "\n";
    std::cout << "\nB (numerical):\n" << mmc.getB() << "\n";

    // Admittance matrix
    std::complex<double> s = std::complex<double>(0, 2 * M_PI * Fnom); 
    Eigen::MatrixXcd Y = mmc.computeAdmittanceMatrix(s);

    // Print the admittance matrix
    std::cout << "Admittance Matrix: " << Fnom << "):\n" << Y << std::endl;
    
    // Equilibrium Solution
    std::cout << "\nEquilibrium Solution: \n";
    mmc.solveEquilibrium();
    const Eigen::VectorXd x_eq = mmc.getEquilibriumState();
    std::cout << "Equilibrium state:\n" << x_eq.transpose() << "\n";

    // Verify equilibrium
    const Eigen::VectorXd dx_eq = mmc.computeStateDerivatives(x_eq.head(6), u0);
    std::cout << "||dx|| at equilibrium: " << dx_eq.norm() << "\n";

    mmc.printEigenvalues();  // Display eigenvalues
    mmc.checkStability();  // Analyze stability
	

	// Run OPF Haxaio
	//solve_opf("../../src/mtdc3slack_a", "../../src/ac9ac14",
	//	/*vscControl*/ true,
	//	/*writeTxt  */ false,
	//	/*plotResult*/ false);

	// Cutset
	Bus* bus0 = new Bus("0", 1);
	Bus* bus1 = new Bus("1", 1);
	Bus* bus2 = new Bus("2", 1);

	std::vector<Bus*> buses = { bus0, bus1, bus2 };
	// Create Elements (assumes ACSource and Resistor constructors take two buses and a value)
	AC_source* ac = new AC_source("ac", 1, DenseMatrix(1, 1, { integer(10) }));
	Resistor* r1 = new Resistor("R1", 2, 2.0);
	Resistor* r2 = new Resistor("R2", 2, 2.0);
	Resistor* r3 = new Resistor("R3", 2, 2.0);

	//r1->printElementInfo();       // Base class method
	//r1->printElementValues();     // Calls Resistor::printElementValues

	// Manually connect elements to buses
	ac->attachBus(bus1, 1);
	ac->attachBus(bus0, 2);

	r1->attachBus(bus1, 1);
	r1->attachBus(bus0, 2);

	r2->attachBus(bus1, 1);
	r2->attachBus(bus2, 2);

	r3->attachBus(bus2, 1);
	r3->attachBus(bus0, 2);
	
	// Collect elements
	std::vector<Element*> elements = { ac, r1, r2, r3 };

	auto busToElementsMap = StateSpaceModel::generateBusToElementsMap(elements);

	// Create the StateSpaceModel object
	StateSpaceModel model;

	//Generate all bus cutsets
	std::vector<std::vector<Bus*>> cutset_nodes = model.from_cutset_nodes(buses, {});

	std::cout << "Cutset Nodes:" << std::endl;
	for (const auto& group : cutset_nodes) {
		std::cout << "[ ";
		for (Bus* b : group) {
			std::cout << b->getBusName() << " ";
		}
		std::cout << "]" << std::endl;
	}

	// Generate element cutsets from the bus cutsets
	std::vector<std::vector<Element*>> cutset_elements = model.from_cutsets(cutset_nodes, busToElementsMap);

	std::cout << "\nCutset Elements:" << std::endl;
	for (const auto& group : cutset_elements) {
		std::cout << "[ ";
		for (Element* e : group) {
			std::cout << e->getElementSymbol() << " ";
		}
		std::cout << "]" << std::endl;
	}

	// Find loops
	auto loops = StateSpaceModel::findLoops(buses, busToElementsMap);

	// Print loop results
	std::cout << "\nLoops found:" << std::endl;
	for (size_t i = 0; i < loops.size(); ++i) {
		std::cout << "Loop " << i + 1 << ": ";
		for (Element* e : loops[i]) {
			std::cout << e->getElementSymbol() << " ";
		}
		std::cout << std::endl;
	}

	delete bus0;
	delete bus1;
	delete bus2;

	delete ac;
	delete r1;
	delete r2;
	delete r3;

	// Cleanup
	//delete myNetwork;
	//delete transformer;  
	//delete load1;

	return 0;
}
