#include "Examples.h"

#include "../network.h"
#include "../Bus.h"
#include "../Include_components.h"

void example_MMC() {
	//// Numerically computes the Jacobian matrices A = ∂f/∂x and B = ∂f/∂u at a specified operating point
	double f = 50;
	double omega = 2 * M_PI * f; // Nominal frequency in rad/s
	std::vector<double> converter_params = { omega, 100.0e6, 0, 0.0, 100.0e3, 200e3, 50e-3, 1.07, 0.01, 100, 0.06, 0.535, 10e-6 };
	std::vector<double> controller_params = { 0, 0,
		//1, 0.01, 2, 2, 0, 200e3, // DC voltage controller parameters
		1, 6.6667e-07, 3.3333e-04, 1, 100e6, // active power
		0,
		1, 6.6667e-07, 3.3333e-04, 1, 0, // reactive power
		1, 120, 400, 1, 0, // energy controller parameters 
		1, 19.93, 4500, 1, 166.67, // zcc controller parameters 
		1, 117.93, 8.5e4, 2, 666.67, 0, // occ controller parameters
		1, 19.93, 4500, 2, 0, 0 }; // ccc controller parameters

	MMC* mmc1 = new MMC("MMC1", converter_params, controller_params);


	//// Define operating point
	// Eigen::VectorXd x0 = Eigen::VectorXd::Zero(6); 
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

	// Equilibrium Solution
	std::cout << "\nEquilibrium Solution: \n";
	mmc1->solveEquilibrium();
	const Eigen::VectorXd x_eq = mmc1->getEquilibriumState();
	std::cout << "Equilibrium state:\n" << x_eq.transpose() << "\n";

	// mmc1->printElementValues();  // Print MMC parameters

	//// Verify equilibrium
	//const Eigen::VectorXd dx_eq = mmc.computeStateDerivatives(x_eq.head(6), u0);
	//std::cout << "||dx|| at equilibrium: " << dx_eq.norm() << "\n";

	//mmc.printEigenvalues();  // Display eigenvalues
}