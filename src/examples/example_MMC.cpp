#include "Examples.h"

#include "../network.h"
#include "../Bus.h"
#include "../Include_components.h"

void example_MMC() {
	//// Numerically computes the Jacobian matrices A = ∂f/∂x and B = ∂f/∂u at a specified operating point
	double f = 50;
	double omega = 2 * M_PI * f; // Nominal frequency in rad/s
	double Vdc = 200e3; // DC voltage in Volts
	std::vector<double> converter_params = { omega, 100.0e6, 0, 0.0, 100.0e3, Vdc, 50e-3, 1.07, 0.01, 100, 0.06, 0.535, 0 };
	std::vector<double> controller_params = { 0,
		//1, 0,001103374, 0,00073, 1, 0, // PLL controller parameters
		0,
		//1, 0.01, 2, 1, Vdc, // DC voltage controller parameters
		1, 6.6667e-07, 3.3333e-04, 1, 100e6, // active power
		0,
		1, 6.6667e-07, 3.3333e-04, 1, 0, // reactive power
		1, 120, 400, 1, 0, // energy controller parameters 
		1, 19.93, 4500, 1, 166.67, // zcc controller parameters 
		1, 117.93, 8.5e4, 2, 666.67, 0, // occ controller parameters
		1, 19.93, 4500, 2, 0, 0 }; // ccc controller parameters

	std::vector<double> filter_params = { // 0, 
		1, 0.0909, 0, 0, // Vdq filter parameters
		0,
		1, 0, 0.7, 100.48, // active power filter parameters
		1, 0, 0.7, 100.48, // reactive power filter parameters
		0 };

	MMC* mmc1 = new MMC("MMC1", converter_params, controller_params, filter_params);

	// Equilibrium Solution
	std::cout << "\nEquilibrium Solution: \n";
	mmc1->solveEquilibrium();
	const Eigen::VectorXd x_eq = mmc1->getEquilibriumState();
	std::cout << "Equilibrium state:\n" << x_eq.transpose() << "\n";

	// Numerical Jacobian
	mmc1->computeABCD();
	std::cout << "\nA:\n" << mmc1->getA() << "\n";
	std::cout << "\nB:\n" << mmc1->getB() << "\n";

	 //Admittance matrix
	Eigen::MatrixXcd Y = mmc1->compute_y_parameters_num(omega);

	// Print the admittance matrix
	std::cout << "Admittance Matrix: " << omega << "):\n" << Y << std::endl;



	// mmc1->printElementValues();  // Print MMC parameters

	//mmc.printEigenvalues();  // Display eigenvalues
}