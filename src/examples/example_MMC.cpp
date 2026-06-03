#include "Examples.h"

#include "../network.h"
#include "../Bus.h"
#include "../Include_components.h"

void example_MMC(bool plotting_enabled /*=true*/) {
	//// Numerically computes the Jacobian matrices A = ∂f/∂x and B = ∂f/∂u at a specified operating point
	double f = 50;
	double omega = 2 * M_PI * f; // Nominal frequency in rad/s
	double Vdc = 200e3; // DC voltage in Volts
	std::vector<double> converter_params = { omega, 100.0e6, 0, 0.0, 100.0e3, 100e6, Vdc, 50e-3, 1.07, 0.01, 400, 0.06, 0.535, 150e-6 };
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

	cout << "\nComputing Jacobian matrices A, B, C using numerical differentiation...\n";

	// Numerical Jacobian
	mmc1->computeABCD();
	auto A1 = mmc1->getA(); 
	auto B1 = mmc1->getB();
	auto C1 = mmc1->getC();

	cout << "\nComputing Jacobian matrices A, B, C using analytical differentiation...\n";

	mmc1->computeABCD_analytical();
	auto A2 = mmc1->getA();
	auto B2 = mmc1->getB();
	auto C2 = mmc1->getC();

	double err1 = (A1 - A2).norm() / A1.norm();
	double err2 = (B1 - B2).norm() / B1.norm();
	double err3 = (C1 - C2).norm() / C1.norm();

	std::cout << "Relative error A: " << err1 << "\n";  // should be < 1e-6
	std::cout << "Relative error B: " << err2 << "\n";  // should be < 1e-6
	std::cout << "Relative error C: " << err3 << "\n";  // should be < 1e-6

	cout << "Press enter to continue...\n";
	cin.get();	

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
	auto A3 = mmc2->getA();
	auto B3 = mmc2->getB();
	auto C3 = mmc2->getC();
	
	mmc2->computeABCD_analytical();
	auto A4 = mmc2->getA();
	auto B4 = mmc2->getB();
	auto C4 = mmc2->getC();

	double err4 = (A3 - A4).norm() / A3.norm();
	double err5 = (B3 - B4).norm() / B3.norm();
	double err6 = (C3 - C4).norm() / C3.norm();

	std::cout << "Relative error A: " << err4 << "\n";  // should be < 1e-6
	std::cout << "Relative error B: " << err5 << "\n";  // should be < 1e-6
	std::cout << "Relative error C: " << err6 << "\n";  // should be < 1e-6

	MatrixXcd Y2 = vectorToMatrix(mmc2->compute_y_parameters(50));
	cout << "\nY-parameters at 50 Hz:\n" << std::setprecision(10) << Y2 << "\n";

	mmc2->printEigenvalues();

	if (plotting_enabled){
		mmc2->plotParticipationFactors();
		mmc1->plotEigenvalues();
		mmc2->plotEigenvalues();
		mmc2->plotYParameters(1, 1000, 500);
	}

	cout << "Press Enter to continue...\n";
	cin.get();
}