#include "Examples.h"

#include "../network.h"
#include "../Bus.h"
#include "../Include_components.h"

void example_admittance_parameters() {
	Inductor* ind = new Inductor("IND1", "AC1", 3, { 1.0 });

	cout << "Inductor Y-parameters without transformation: " << endl;
	vector<vector<complex<double>>> Y_ind = ind->compute_y_parameters(20.0);
	for (const auto& row : Y_ind) {
		for (const auto& val : row) {
			cout << val << " ";
		}
		cout << endl;
	}

	ind->setTransformation(true);
	cout << "Inductor Y-parameters with transformation: " << endl;
	Y_ind = ind->compute_y_parameters(20.0);
	for (const auto& row : Y_ind) {
		for (const auto& val : row) {
			cout << val << " ";
		}
		cout << endl;
	}

}