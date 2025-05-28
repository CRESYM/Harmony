#include "network.h"
#include "Bus.h"
#include "Include_components.h"
#include "../build/solve_opf.h"


int main() {
	// Create Transformer object with 1 pin
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
	//Generator* g = new Generator("gen", 3, generator_values);

	std::vector<double> vec = {1,1,1}; 
	Load* load = new Load("l1", 3, vec);
	DenseMatrix Y = load->compute_y_parameters();
	for (int i = 0; i < Y.nrows(); i++) {
		for (int j = 0; j < Y.ncols(); j++) {
			cout << Y.get(i, j)->__str__() << " ";
		}
		cout << endl;
	}
	cout << endl;

	MatrixXcd Y_num = load->compute_y_parameters_num(10);
	cout << Y_num << endl;


	//Admittance* y = new Admittance("y1", 1, DenseMatrix(1, 1, { mul(j, omega)}));
	Impedance* y = new Impedance("z1", 1, DenseMatrix(1, 1, { div(integer(1), mul(j, omega)) }));
	AC_source* ac = new AC_source("ac", 1, DenseMatrix(1, 1, { integer(10) }));
	Network* myNetwork = new Network();

	// Create Bus objects
	Bus* bus1 = new Bus("Bus1", 1);
	Bus* gnd = new Bus("gnd", 1);

	// Add elements to the network
	myNetwork->addElement(y);
	myNetwork->addElement(ac);

	// Add buses to the network
	myNetwork->addBus("Bus1", bus1);
	myNetwork->addBus("gnd", gnd);

	// Connect elements to buses
	myNetwork->connectElementToBus(y, 1, bus1);
	myNetwork->connectElementToBus(y, 2, gnd);
	myNetwork->connectElementToBus(ac, 1, bus1);
	myNetwork->connectElementToBus(ac, 2, gnd);

	// Print the connections to verify the network
	myNetwork->printConnections();


	vector<Bus*> start_buses;
	vector<Bus*> end_buses;
	vector<Element*> skip_elements;

	start_buses.push_back(bus1);
	end_buses.push_back(gnd);
	skip_elements.push_back(ac);



	//y->writeFile(10.0, 1000.0, 3);
	double omega_num = 1000;
	//MatrixXcd Yparam = y->compute_y_parameters_num(omega_num);

	//cout << Yparam << endl;

	//Eigen::MatrixXcd equivalent_impedance;
	myNetwork->compute_equivalent_impedance(start_buses, end_buses, skip_elements);
	
	try {
		// Compute equivalent impedance
		myNetwork->compute_equivalent_impedance_nums(start_buses, end_buses, skip_elements, omega_num);
	}
	catch (const std::exception& e) {
		std::cerr << "Error during impedance computation: " << e.what() << std::endl;
		delete myNetwork;
		return EXIT_FAILURE;
	}
	//Create and Test MMC**
	MMC* myMMC = new MMC(
		"MMC1",         // Symbol
		2,              // Number of input pins
		2,              // Number of output pins
		1000.0,         // Omega (Nominal Frequency in rad/s)
		100.0,          // Active Power (P) in MW
		50.0,           // Reactive Power (Q) in MVA
		500.0,          // DC Power (P_dc) in kW
		-100.0,         // Min Active Power (P_min) in MW
		200.0,          // Max Active Power (P_max) in MW
		-50.0,          // Min Reactive Power (Q_min) in MVA
		100.0,          // Max Reactive Power (Q_max) in MVA
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

	myNetwork->addElement(myMMC);

	// Example MMC parameters (must match Controller constructor)
	std::vector<double> converter_params = {
		1000.0, 100.0, 50.0, 500.0, -100.0, 200.0, -50.0, 100.0, 314.16, 400,
		1.07, 0.05, 0.01, 0.535, 0.06, 0.00015
	};

	std::cout << "Converter params size: " << converter_params.size() << std::endl;

	try {
		myMMC->init_Controller(converter_params);
		std::cout << "MMC Controller initialized successfully!\n";
	}
	catch (const std::exception& e) {
		std::cerr << "Error initializing MMC Controller: " << e.what() << std::endl;
		delete myMMC;
		delete myNetwork;
		return EXIT_FAILURE;
	}


	// run OPF Haxaio
	solve_opf("mtdc3slack_a", "ac9ac14",
		/*vscControl*/ true,
		/*writeTxt  */ false,
		/*plotResult*/ false);

	// Cleanup
	

	// Clean up dynamically allocated memory
	//delete myNetwork;
	//delete transformer;  // Clean up transformer


	//delete load1;

	return 0;
}
