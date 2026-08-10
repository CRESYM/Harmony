#include <gtest/gtest.h>
#include <complex>

#include "../network.h"
#include "../Bus.h"
#include "../Include_components.h"
#include "../Solver/Stability_Estimate/Stability_estimate.h"


namespace {

MatrixXcd yFromAbcd(
	const Eigen::MatrixXd& A,
	const Eigen::MatrixXd& B,
	const Eigen::MatrixXd& C,
	const Eigen::MatrixXd& D,
	double frequency,
	bool applyDcCorrection,
	double C_arm,
	int numSubmodules)
{
	const double omega_num = 2.0 * M_PI * frequency;
	const std::complex<double> s(0.0, omega_num);
	const Eigen::MatrixXcd Ac = A.cast<std::complex<double>>();
	const Eigen::MatrixXcd Bc = B.cast<std::complex<double>>();
	const Eigen::MatrixXcd Cc = C.cast<std::complex<double>>();
	const Eigen::MatrixXcd Dc = D.cast<std::complex<double>>();
	const int n = static_cast<int>(A.rows());

	Eigen::MatrixXcd Y = Cc * (s * Eigen::MatrixXcd::Identity(n, n) - Ac).inverse() * Bc + Dc;
	Y(1, 1) = -Y(1, 1);
	Y(2, 2) = -Y(2, 2);

	if (applyDcCorrection) {
		Y(0, 0) = 1.0 / Y(0, 0);
		Y(0, 0) = 2.0 * (Y(0, 0) - s * C_arm * (6.0 / numSubmodules));
	}
	return Y;
}

Eigen::Vector3d operatingInput(double Vdc, double Vm, double theta) {
	const double Vgd = Vm * std::cos(theta);
	const double Vgq = -Vm * std::sin(theta);
	Eigen::Vector3d u;
	u << Vdc, Vgd, Vgq;
	return u;
}

} // namespace


class TestMMC : public testing::Test {};

TEST_F(TestMMC, TestYMatrix) {
	const double f = 50.0;
	const double omega = 2.0 * M_PI * f;
	const double Vdc = 200e3;
	const double Vm = 100.0e3;
	const double theta = 0.0;
	std::vector<double> converter_params = {
		omega, 100.0e6, 0, 0.0, 100.0e3, 100e6, Vdc,
		50e-3, 1.07, 0.01, 400, 0.06, 0.535, 150e-6
	};
	std::vector<double> controller_params = {
		0, 0,
		1, 0, 6.6667e-07, 3.3333e-04, 1, 100e6,
		0, 0,
		1, 0, 120, 400, 1, 0,
		1, 0, 19.93, 4500, 1, 166.67,
		1, 0, 117.93, 8.5e4, 2, 666.67, 0,
		1, 0, 19.93, 4500, 2, 0, 0,
		0
	};

	MMC mmc("MMC1", "AC1_DC1", converter_params, controller_params);

	ASSERT_NO_THROW(mmc.solveEquilibrium());
	const Eigen::VectorXd x_eq = mmc.getEquilibriumState();
	EXPECT_GT(x_eq.size(), 12u);
	EXPECT_TRUE(x_eq.allFinite());
	EXPECT_GT(x_eq.norm(), 0.0);

	const Eigen::Vector3d u = operatingInput(Vdc, Vm, theta);
	const Eigen::VectorXd dx = mmc.computeStateDerivatives(x_eq, u);
	EXPECT_TRUE(dx.allFinite());
	EXPECT_LT(dx.lpNorm<Eigen::Infinity>(), 0.5)
		<< "Equilibrium residual too large; KINSOL may have stopped at a loose tolerance.";

	mmc.computeABCD();
	EXPECT_GT(mmc.getA().rows(), 0);
	EXPECT_TRUE(mmc.getA().allFinite());
	EXPECT_TRUE(mmc.getB().allFinite());
	EXPECT_TRUE(mmc.getC().allFinite());
	EXPECT_TRUE(mmc.getD().allFinite());

	const MatrixXcd Y = vectorToMatrix(mmc.compute_y_parameters(f));
	EXPECT_EQ(Y.rows(), 3);
	EXPECT_EQ(Y.cols(), 3);
	EXPECT_TRUE(Y.allFinite());

	const MatrixXcd Y_ref = yFromAbcd(
		mmc.getA(), mmc.getB(), mmc.getC(), mmc.getD(),
		f, false, 0.01, 400);
	EXPECT_TRUE(Y.isApprox(Y_ref, 1e-9))
		<< "compute_y_parameters must match the ABCD frequency response.";

	const MatrixXcd Y_repeat = vectorToMatrix(mmc.compute_y_parameters(f));
	EXPECT_TRUE(Y.isApprox(Y_repeat, 1e-12));
}

TEST_F(TestMMC, TestGfmModeEquilibrium) {
	// Match example_MMC_gfm MVP (energy + ZCC + GFM). The previous pack
	// (energy off, fixed ZCC gains 19.93/4500) does not converge with KINSOL.
	const double omega = 2.0 * M_PI * 50.0;
	const double Vdc = 240e3;
	const double Vm = 100e3;
	const double Pac = 100e6;
	const double Qac = 0.0;
	const double Larm = 50e-3;
	const double Rarm = 1.07;
	const double Lf = 60e-3;
	const double Rf = 0.535;
	const double Reqac = Rf + Rarm / 2.0;
	const double Leqac = Lf + Larm / 2.0;
	const double Id = (2.0 / 3.0) * Pac / Vm;
	const double Iq = 0.0;
	const double vMd0 = Vm + Reqac * Id + omega * Leqac * Iq;
	const double vMq0 = 0.0 + Reqac * Iq - omega * Leqac * Id;
	const double Egfm_ref = std::hypot(vMd0, vMq0);
	const double Kdroop_P = 2.0 * M_PI * 0.5 / 100e6;
	const double Kdroop_Q = 0.02 * Egfm_ref / 50e6;
	const double zeta = 0.7;
	const double w0_ccc = 300.0;
	const double Ki_zcc = Larm * w0_ccc * w0_ccc;
	const double Kp_zcc = 2.0 * zeta * std::sqrt(Ki_zcc * Larm) - Rarm;

	std::vector<double> converter_params = {
		omega, Pac, Qac, 0.0, Vm, Pac, Vdc,
		Larm, Rarm, 0.01, 50, Lf, Rf, 0.0
	};
	std::vector<double> controller_params = {
		0, 0, 0, 0, 0,
		1, 0, 120, 400, 1, 0, // energy
		1, 0, Kp_zcc, Ki_zcc, 1, Pac / (3.0 * Vdc), // zcc
		0, // occ
		0, // ccc
		0, // droop
		1, 0, Kdroop_P, Kdroop_Q, 4, 20e-3, 20e-3, 0.0, 0.0 // gfm
	};

	MMC mmc("MMC_GFM", "AC1_DC1", converter_params, controller_params);
	ASSERT_NO_THROW(mmc.solveEquilibrium());
	const Eigen::VectorXd x_eq = mmc.getEquilibriumState();
	EXPECT_GT(x_eq.size(), 15u);
	EXPECT_TRUE(x_eq.allFinite());

	ASSERT_NO_THROW(mmc.computeABCD());
	EXPECT_TRUE(mmc.getA().allFinite());

	const auto u = operatingInput(Vdc, Vm, 0.0);
	const Eigen::VectorXd dx = mmc.computeStateDerivatives(x_eq, u);
	EXPECT_TRUE(dx.allFinite());
	EXPECT_LT(dx.lpNorm<Eigen::Infinity>(), 1e-2)
		<< "GFM closed-loop equilibrium residual too large";
	const int plant = static_cast<int>(x_eq.size()) - 12;
	const double Id_h = x_eq(plant);
	const double Iq_h = x_eq(plant + 1);
	// True GFM SS (MATLAB t→∞): Pac=Pref ⇒ Id=2P/(3V) when Vgq=0; Iq ≠ 0 from Q-V droop / plant.
	EXPECT_NEAR(Id_h, (2.0 / 3.0) * Pac / Vm, 1e-3);
	EXPECT_NEAR(Iq_h, 8.3256, 0.05);
}
