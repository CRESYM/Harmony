#include "Examples.h"

#include "../network.h"
#include "../Bus.h"
#include "../Include_components.h"

void example_MMC() {
	//// Numerically computes the Jacobian matrices A = ∂f/∂x and B = ∂f/∂u at a specified operating point
	double f = 50;
	double omega = 2 * M_PI * f; // Nominal frequency in rad/s
	double Vdc = 200e3; // DC voltage in Volts
	std::vector<double> converter_params = { omega, 100.0e6, 0, 0.0, 100.0e3, 100e6, Vdc, 50e-3, 1.07, 0.01, 400, 0.06, 0.535, 0.00 };
	std::vector<double> controller_params = { 
		0, //1, 0, 0.001103374, 0.00073, 1, 0, // PLL controller parameters
		0, // 1, 0, 8.0, 272.0, 2, 0, Vdc, // DC voltage controller parameters
		1, 0, 6.6667e-07, 3.3333e-04, 1, 100e6, // active power
		0, // AC voltage
		0, //1, 0, 6.6667e-07, 3.3333e-04, 1, 0, // reactive power
		1, 0, 120, 400, 1, 0, // energy controller parameters 
		1, 0, 19.93, 4500, 1, 166.67, // zcc controller parameters 
		1, 0, 117.93, 8.5e4, 2, 666.67, 0, // occ controller parameters
		1, 0, 19.93, 4500, 2, 0, 0, // ccc controller parameters
		0 //1, 1, -0.0001, 2, Vdc, 100e6 // droop control
	}; 

	std::vector<double> filter_params = { // 0, 
		1, 0.0909, 0, 0, // Vdq filter parameters
		0,
		1, 0, 0.7, 100.48, // active power filter parameters
		1, 0, 0.7, 100.48, // reactive power filter parameters
		1, 0, 0.7, 6280 // DC voltage filter parameters
	};

	MMC* mmc1 = new MMC("MMC1", "AC1_DC1", converter_params, controller_params); // , filter_params);

	// Equilibrium Solution
	std::cout << "\nEquilibrium Solution: \n";
	mmc1->solveEquilibrium();
	cout << std::setprecision(6);
	const Eigen::VectorXd x_eq = mmc1->getEquilibriumState();
	std::cout << std::setprecision(6) << "Equilibrium state:\n" << x_eq.transpose() << "\n";

	// Numerical Jacobian
	mmc1->computeABCD();
	//std::cout << "\nA:\n" << mmc1->getA() << "\n";
	//std::cout << "\nB:\n" << mmc1->getB() << "\n";
	//std::cout << "\nC:\n" << mmc1->getC() << "\n";

	//mmc1->printElementValues();  // Print MMC parameters, together with the reference values for the controllers

	MatrixXcd Y = vectorToMatrix(mmc1->compute_y_parameters(50));
	cout << "\nY-parameters at 50 Hz:\n" << std::setprecision(10) << Y << "\n";

	mmc1->writeFile(1.0, 1000.0, 1000);

	// The other MMC to check
	vector<double> converter_params2 = {
		2 * M_PI * 50,  // Omega (Nominal Frequency in rad/s)
		-50.0 * 1e6,    // Active Power (P) in W
		-20e6,          // Reactive Power (Q) in VA
		0.0,            // Theta (Voltage Angle in rad)
		345.0 * 1e3,    // AC Voltage (V_m) in V
		-50 * 1e6,       // DC power (P_dc) in W
		440.0 * 1e3,    // DC Voltage (V_dc) in kV
		0.05,           // Arm Inductance (L_arm) in H
		1.07,           // Arm Resistance (R_arm) in Ω
		0.01,           // Capacitance per Submodule (C_arm) in F
		400,            // Number of Submodules (N)
		0.0005,         // Reactor Inductance (L_reactor) in H
		0.0001,         // Reactor Resistance (R_reactor) in Ω
		0.0             // Time Delay (t_delay) in seconds
	};
	std::vector<double> controller_params2 = {
		1, 0, 0.001103374, 0.00073, 1, 0, // PLL controller parameters
		1, 0, 2, 82, 2, 0, 440e3, // DC voltage controller parameters
		0, // active power
		0, // AC voltage
		1, 0, 6.6667e-07, 3.3333e-04, 1, -20e6, // reactive power
		1, 0, 120, 400, 1, 0, // energy controller parameters 
		1, 0, 19.93, 4500, 1, -41.66, // zcc controller parameters 
		1, 0, 117.93, 8.5e4, 2, -89.71, 0, // occ controller parameters
		1, 0, 19.93, 4500, 2, 0, 0, // ccc controller parameters
		0  // droop control
	};
	MMC* mmc2 = new MMC("MMC2", "AC2_DC1", converter_params2, controller_params2);

	mmc2->solveEquilibrium();
	cout << std::setprecision(6);
	const Eigen::VectorXd x_eq2 = mmc2->getEquilibriumState();
	std::cout << std::setprecision(6) << "Equilibrium state:\n" << x_eq2.transpose() << "\n";

	mmc2->computeABCD();
	/*std::cout << "\nA:\n" << mmc2->getA() << "\n";
	std::cout << "\nB:\n" << mmc2->getB() << "\n";
	std::cout << "\nC:\n" << mmc2->getC() << "\n";*/

	MatrixXcd Y2 = vectorToMatrix(mmc2->compute_y_parameters(50));
	cout << "\nY-parameters at 50 Hz:\n" << std::setprecision(10) << Y2 << "\n";

	mmc2->printEigenvalues();

	//mmc2->plotParticipationFactors();
	//mmc2->plotEigenvalues();
	mmc2->plotYParameters(1, 1000, 500);
}