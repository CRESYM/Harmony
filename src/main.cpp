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
	std::vector<double> transmission_line_values = { 0.01, 2.5e-7, 1e-9, 1e-11, 1000 };
	TransmissionLine* transmission_line = new TransmissionLine("tl", 1, transmission_line_values);
	transmission_line->writeFile(10, 10000000, 1000);


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

	// Generalized stability check
	//myNetwork->checkStability(A_lin);
    //mmc.checkStability();

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
    //myNetwork->checkStability(mmc.getA());  // Analyze stability

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

	
	//// Create a Capacitor between bus1 and bus2
	//double capacitance = 1e-6; // 1 microfarad
	//Capacitor* cap = new Capacitor("C1", bus1, bus2, capacitance, 0.0); //single-phase
	//

	//std::vector<Bus*> all_buses = { bus0, bus1, bus2 };
	//std::unordered_map<Bus*, int> busIndex;
	//int row_index = 0;
	//for (auto* bus : all_buses) {
	//	busIndex[bus] = row_index++;
	//}

	//int num_equations = static_cast<int>(all_buses.size());

	//// MNA stamp
	//DenseMatrix M(num_equations + 1, num_equations + 1);
	//cap->writeMNAmatrix(M, num_equations, 1, symbol("C1"), busIndex);
	//std::cout << "M(3,3) = " << *M.get(3, 3) << "\n";   // branch row, col
	//std::cout << "M(1,3) = " << *M.get(1, 3) << "\n";   // node-row , col
	//std::cout << "M(3,1) = " << *M.get(3, 1) << "\n";   // branch row , node-col

	//// add capacitor to element list so loops / cutsets include it
	//elements.push_back(cap);

	//switch single-phase 
	Bus* a = new Bus("a",1);
	Bus* b = new Bus("b",1);

	Switch* sw = new Switch("SW1", { a }, { b }, { true }); //closed

	std::vector<Bus*> buses1 = { a, b };
	std::unordered_map<Bus*, int> busIndex1 = { {a,0},{b,1} };

	int num_eq = static_cast<int>(buses1.size());
	DenseMatrix A(num_eq + 1, num_eq + 1);      // +1 auxiliary row

	sw->writeMNAmatrix(A, num_eq, 0, symbol("SW1"), busIndex1);

	std::cout << "A(2,0)=" << *A.get(2, 0) << "  A(0,2)=" << *A.get(0, 2) << '\n'; 
	std::cout << "A(2,1)=" << *A.get(2, 1) << "  A(1,2)=" << *A.get(1, 2) << '\n'; 

    //switch three-phase 
	Bus* a1 = new Bus("a1", 1);
	Bus* a2 = new Bus("a2", 1);
	Bus* a3 = new Bus("a3", 1);
	Bus* b1 = new Bus("b1", 1);
	Bus* b2 = new Bus("b2", 1);
	Bus* b3 = new Bus("b3", 1);

	std::vector<Bus*> node1s = { a1, a2, a3 };
	std::vector<Bus*> node2s = { b1, b2, b3 };
	std::vector<bool> phaseStates = { true, false, true };  // for example: phase 0 and 2 closed, phase 1 open

	Switch* sw3p = new Switch("SW3P", node1s, node2s, phaseStates);

	std::vector<Bus*> buses3p = { a1, a2, a3, b1, b2, b3 };
	std::unordered_map<Bus*, int> busIndex3p;
	for (int i = 0; i < 6; ++i) {
		busIndex3p[buses3p[i]] = i;
	}

    int num_eq_3p = static_cast<int>(buses3p.size());
	DenseMatrix A3p(num_eq_3p + 3, num_eq_3p + 3);  // +3 auxiliary rows, one per phase
	
	// Initialize A3p to zero
	for (int i = 0; i < A3p.nrows(); ++i)
		for (int j = 0; j < A3p.ncols(); ++j)
			A3p.set(i, j, zero);

	
	sw3p->writeMNAmatrix(A3p, num_eq_3p, 0, symbol("SW3P"), busIndex3p);  

	std::cout << "\nThree-phase switch matrix entries:\n";

	for (int p = 0; p < 3; ++p) {
		int aux_row = num_eq_3p + p;   
		int idx_n1 = busIndex3p[node1s[p]];
		int idx_n2 = busIndex3p[node2s[p]];

		std::cout << "Diagonal A(aux_row=" << aux_row << ", aux_row=" << aux_row << ") = " << *A3p.get(aux_row, aux_row) << "\n";
		std::cout << "A(aux_row=" << aux_row << ", n1=" << idx_n1 << ") = " << *A3p.get(aux_row, idx_n1) << "\n";
		std::cout << "A(n1=" << idx_n1 << ", aux_row=" << aux_row << ") = " << *A3p.get(idx_n1, aux_row) << "\n";
		std::cout << "A(aux_row=" << aux_row << ", n2=" << idx_n2 << ") = " << *A3p.get(aux_row, idx_n2) << "\n";
		std::cout << "A(n2=" << idx_n2 << ", aux_row=" << aux_row << ") = " << *A3p.get(idx_n2, aux_row) << "\n";


		// For open phase (phase 1)
		if (!phaseStates[p]) {
			std::cout << "Diagonal A(aux_row=" << aux_row << ", aux_row=" << aux_row << ") = " << *A3p.get(aux_row, aux_row) << "\n";
		}
	}

	// Print full matrix
	std::cout << "\n--- MNA Matrix A3p (9x9) ---\n";
	for (int i = 0; i < A3p.nrows(); ++i) {
		for (int j = 0; j < A3p.ncols(); ++j) {
			std::cout << A3p.get(i, j)->__str__() << "\t";
		}
		std::cout << "\n";
	}

    // Clean up
	 delete a; delete b;

	// Cleanup
	delete myNetwork;
	//delete transformer;  
	//delete load1;

	return 0;
}
