#include <gtest/gtest.h>

#include "../network.h"
#include "../Bus.h"
#include "../Include_components.h"
#include "../Solver/Stability_Estimate/Stability_estimate.h"


class TestMMC : public testing::Test {};

TEST_F (TestMMC, TestYMatrix) {
	double f = 50;
	double omega = 2 * M_PI * f; // Nominal frequency in rad/s
	double Vdc = 200e3; // DC voltage in Volts
	std::vector<double> converter_params = { omega, 100.0e6, 0, 0.0, 100.0e3, 100e6, Vdc, 50e-3, 1.07, 0.01, 400, 0.06, 0.535, 0.00 };
	std::vector<double> controller_params = {
		0, // PLL controller parameters
		0, // DC voltage controller parameters
		1, 0, 6.6667e-07, 3.3333e-04, 1, 100e6, // active power
		0, // AC voltage
		1, 0, 6.6667e-07, 3.3333e-04, 1, 0, // reactive power
		1, 0, 120, 400, 1, 0, // energy controller parameters 
		1, 0, 19.93, 4500, 1, 166.67, // zcc controller parameters 
		1, 0, 117.93, 8.5e4, 2, 666.67, 0, // occ controller parameters
		1, 0, 19.93, 4500, 2, 0, 0, // ccc controller parameters
		0 // droop control
	};


	MMC* mmc1 = new MMC("MMC1", "AC1_DC1", converter_params, controller_params); 

	// Equilibrium Solution
	mmc1->solveEquilibrium();
	const Eigen::VectorXd x_eq = (mmc1->getEquilibriumState().tail(12)).transpose();
	VectorXd eq_expected = VectorXd::Zero(12);
	eq_expected << 666.67, 0, 168, -3e-4, -0.04, -1767, 10340, -184.5, 8.5, -883, 5282, 2e5;
	EXPECT_TRUE(x_eq.isApprox(eq_expected, 1e-2));

	mmc1->computeABCD();

	MatrixXcd Y = vectorToMatrix(mmc1->compute_y_parameters(50));
	MatrixXcd Y_expected(3, 3);
	Y_expected << complex<double>(0.030598988858771, 0.013090785619443), complex<double>(0.000432075427694, -0.009662340082696), complex<double>(0.008157806351866, 0.000426692938430),
		complex<double>(-0.000580247572352, 0.000512180201861), complex<double>(-0.000419860929455, 0.001307932399178), complex<double>(-0.000256318838786, 0.000113364113827),
		complex<double>(0.001403957847627, 0.002111651402860), complex<double>(0.000962692543081, -0.000376228714279), complex<double>(0.000901726714724, -0.000314670340452);
	EXPECT_TRUE(Y.isApprox(Y_expected, 1e-1));

}