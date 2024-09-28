#include "Element.h"
//#include "TransmissionLine.h"
//#include "Generator.h"
//#include "Load.h"
//#include "Transformer.h"
//#include "Bus.h"
//#include "Impedance.h"
#include "network.h"
#include "Admittance.h"
//#include "AC_source.h"


#include <symengine/symbol.h>
#include <symengine/add.h>
#include <symengine/mul.h>
#include <symengine/pow.h>
#include <symengine/real_double.h>
#include <symengine/eval.h>
#include <symengine/functions.h>
#include <symengine/complex.h>
#include <symengine/complex_double.h>
#include <symengine/expression.h>
#include <symengine/symengine_config.h>
#include <SymEngine/Matrix.h> // Ensure you include the necessary SymEngine headers
#include <symengine/expression.h>
#include <symengine/eval_double.h>
#include <symengine/symengine_config.h>

#include <iostream>
#include <complex>
#include <vector>

using namespace std;
using namespace SymEngine;

int main() {

	SymEngine::vec_basic elems{integer(10), symbol("s"), integer(20) };
	SymEngine::DenseMatrix A = SymEngine::DenseMatrix(1, 3, elems);

	for (int i = 0; i < A.nrows(); i++)
		for (int j = 0; j < A.ncols(); j++)
			std::cout << A.get(i, j)->__str__() << endl;

	Admittance* y = new Admittance("y1", 3, A);
	y->printElementValues();

	Network* myNetwork = new Network();

	// Create Bus objects
	Bus* bus1 = new Bus("Bus1");
	Bus* gnd = new Bus("gnd");

	// Add elements to the network
	myNetwork->addElement(y->getElementSymbol(), y);


	// Add buses to the network
	myNetwork->addBus("Bus1", bus1);
	myNetwork->addBus("gnd", gnd);

	// Connect elements to buses
	myNetwork->connectElementToBus(y, bus1);
	myNetwork->connectElementToBus(y, gnd);

	// Print the connections to verify the network
	myNetwork->printConnections();

	delete myNetwork;

	//// Frequency for Y-parameter computation
	//double frequency = 50.0; // Example frequency in Hz


	//// Clean up dynamically allocated memory
	//delete bus1;
	//delete bus2;
	//delete load1;
	//delete load2;
	//delete generator;


	//delete load1;

	return 0;
}
