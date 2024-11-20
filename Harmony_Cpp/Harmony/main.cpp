#include "network.h"
#include "Bus.h"
#include "Include_components.h"


int main() {
	// Create Transformer object with 1 pin
	//std::vector<double> transformer_values = { 10.0, 5.0, 12.0, 6.0, 2.0, 0.0 }; // R_primary, X_primary, R_secondary, X_secondary, Turns Ratio
	//Transformer_real* transformer = new Transformer_real("T1", 1, transformer_values);
	//transformer->printElementValues();

	//Eigen::MatrixXd Z(3, 3);  // 3x3 impedance matrix
	//Eigen::MatrixXd Y(3, 3);  // 3x3 admittance matrix

	//// Fill Z and Y with appropriate values
	//Z << 1.0, 0.1, 0.2,
	//	0.1, 1.0, 0.3,
	//	0.2, 0.3, 1.0;

	//Y << 0.5, 0.2, 0.1,
	//	0.2, 0.5, 0.3,
	//	0.1, 0.3, 0.5;

	//Y = Y.sinh();
	//std::cout << "Y parameters matrix:\n" << Y << std::endl;


	std::vector<double> distances = { 11.8, 27.5 };
	std::vector<int> numbers = { 2, 2 };
	std::vector<double> values_g = { 0.9196, 0.0062, 10.0, 7.5, 6.5 };
	Overhead_Line* ohl = new Overhead_Line("ohl", 100.0, make_tuple(1.0, 1.0, 1.0),
		make_tuple("flat", numbers, distances, 0.01436, 0.06266, 10.0, 0.4572),
		make_tuple(2, values_g, 1.0));

	std::vector<double> transformer_values = { 1.0, 1.0, 2.0, 2.0, 2.0 }; // R_primary, X_primary, R_secondary, X_secondary, Turns Ratio
	TransformerDeltaY* transformer = new TransformerDeltaY("T1", 3, transformer_values);
	//transformer->printElementValues();

	//std::vector<double> transmission_line_values = { 0.01, 2.5e-7, 1e-9, 1e-11, 1000 };
	//TransmissionLine* t = new TransmissionLine("tl", 3, transmission_line_values);


	//std::vector<double> generator_values = { 1.0, 0.01, 1.0, 0.1 };
	//Generator* g = new Generator("gen", 3, generator_values);

	//vector<double> vec = {0, 0, 1};
	//Load* y = new Load("l1", 1, vec);
	//Impedance* y = new Impedance("z1", 1, DenseMatrix(1, 1, { div(integer(1), mul(j, omega)) }));
	//Admittance* y = new Admittance("y1", 1, DenseMatrix(1, 1, { mul(j, omega)}));
	AC_source* ac = new AC_source("ac", 1, DenseMatrix(1, 1, { integer(10) }));
	Network* myNetwork = new Network();

	// Create Bus objects
	Bus* bus1 = new Bus("Bus1", 1);
	Bus* gnd = new Bus("gnd", 1);

	// Add elements to the network
	//myNetwork->addElement(y);
	//myNetwork->addElement(transformer);  // Add the transformer to the network
	myNetwork->addElement(ac);

	// Add buses to the network
	myNetwork->addBus("Bus1", bus1);
	myNetwork->addBus("gnd", gnd);

	// Connect elements to buses
	//myNetwork->connectElementToBus(transformer, 1, bus1);  // Connect transformer to bus1
	//myNetwork->connectElementToBus(transformer, 2, gnd);    // Connect transformer to ground
	//myNetwork->connectElementToBus(y, 1, bus1);
	//myNetwork->connectElementToBus(y, 2, gnd);
	myNetwork->connectElementToBus(ac, 1, bus1);
	myNetwork->connectElementToBus(ac, 2, gnd);

	// Print the connections to verify the network
	//myNetwork->printConnections();

	// Frequency for Y-parameter computation
	//double frequency = 50.0; // Example frequency in Hz
	//y->compute_y_parameters(frequency);


	vector<Bus*> start_buses;
	vector<Bus*> end_buses;
	vector<Element*> skip_elements;

	start_buses.push_back(bus1);
	end_buses.push_back(gnd);
	skip_elements.push_back(ac);



	//y->writeFile(10.0, 1000.0, 3);
	double omega_num = 2 * M_PI * 1000;
	//MatrixXcd Yparam = y->compute_y_parameters_num(omega_num);

	//cout << Yparam << endl;

	//Eigen::MatrixXcd equivalent_impedance;
	//myNetwork->compute_equivalent_impedance(start_buses, end_buses, elem);
	
	try {
		// Compute equivalent impedance
		myNetwork->compute_equivalent_impedance_nums(start_buses, end_buses, skip_elements, omega_num);
	}
	catch (const std::exception& e) {
		std::cerr << "Error during impedance computation: " << e.what() << std::endl;
		delete myNetwork;
		return EXIT_FAILURE;
	}

	// Clean up dynamically allocated memory
	delete myNetwork;
	//delete transformer;  // Clean up transformer


	//delete load1;

	return 0;
}
