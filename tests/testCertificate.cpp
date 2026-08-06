/**
 * @file testCertificate.cpp
 * @brief Unit tests for decentralized certificate metrics and workflows.
 */
#include <gtest/gtest.h>

#include "../src/Elements/Impedance/Impedance.h"
#include "../src/Elements/Converter/MMC.h"
#include "../src/Solver/Certificate/Device_gate.h"
#include "../src/Solver/Certificate/PnP_library.h"
#include "../src/Solver/Certificate/Stability_certificate.h"

#include <cmath>
#include <complex>
#include <filesystem>

TEST(CertificateMetrics, PassiveiveAdmittanceIsPassive)
{
	Eigen::MatrixXcd Y(2, 2);
	Y << std::complex<double>(1.0, 0.0), std::complex<double>(0.0, 0.0),
		std::complex<double>(0.0, 0.0), std::complex<double>(2.0, 0.0);
	EXPECT_GT(passivityIndex(Y), 0.0);
	EXPECT_GT(dwHermitianMargin(Y), 0.0);
	EXPECT_LT(maxEigenPhaseDeg(Y), 1.0);
}

TEST(CertificateMetrics, AntiPassiveAdmittanceFails)
{
	Eigen::MatrixXcd Y(2, 2);
	Y << std::complex<double>(-1.0, 0.0), std::complex<double>(0.0, 0.0),
		std::complex<double>(0.0, 0.0), std::complex<double>(-0.5, 0.0);
	EXPECT_LT(passivityIndex(Y), 0.0);
	EXPECT_GT(shiftedPassivityIndex(Y, 2.0), 0.0);
}

TEST(CertificateWorkflows, DeviceGateAllowsPassiveLine)
{
	std::complex<double> Z(2.0, 0.5);
	Impedance line("Z1", "AC1", 3, Z);
	CertificateSpec spec;
	spec.f_min_Hz = 10.0;
	spec.f_max_Hz = 100.0;
	spec.n_points = 10;
	spec.ac_dq_block = false;
	spec.phase_limit_deg = 90.0;
	spec.require_phase = false;
	const DeviceGateResult g = evaluateDeviceGate(line, "Z1", spec);
	EXPECT_TRUE(g.allow);
	EXPECT_TRUE(g.pass_passivity);

	PnPCertificateLibrary lib;
	lib.addFromGate(g, "Impedance");
	EXPECT_TRUE(lib.isAllowed("Z1"));
	std::filesystem::create_directories("files");
	lib.saveJson("files/test_pnp_library.json");
	PnPCertificateLibrary lib2;
	lib2.loadJson("files/test_pnp_library.json");
	EXPECT_TRUE(lib2.isAllowed("Z1"));
	std::filesystem::remove("files/test_pnp_library.json");
}

TEST(CertificateMetrics, SweepDeviceCertificateFillsVectors)
{
	std::complex<double> Z(1.0, 0.2);
	Impedance line("Z2", "AC1", 3, Z);
	CertificateSpec spec;
	spec.n_points = 8;
	spec.ac_dq_block = false;
	const CertificateSweep sw = sweepDeviceCertificate(line, spec);
	EXPECT_EQ(sw.freq_Hz.size(), 8u);
	EXPECT_EQ(sw.passivity_index.size(), 8u);
	EXPECT_EQ(sw.shifted_passivity.size(), 8u);
	EXPECT_EQ(sw.dw_margin.size(), 8u);
	EXPECT_EQ(sw.nr_phase_deg.size(), 8u);
	EXPECT_EQ(sw.small_gain.size(), 8u);
	EXPECT_TRUE(sw.pass_passivity);
}

TEST(GeometricCertificates, PassiveiveHasZeroOutsideNumericalRange)
{
	Eigen::MatrixXcd Y(2, 2);
	Y << std::complex<double>(2.0, 0.0), std::complex<double>(0.0, 0.0),
		std::complex<double>(0.0, 0.0), std::complex<double>(3.0, 0.0);
	const NumericalRangeCertificate nr = certifyNumericalRange(Y, 90.0, 120);
	EXPECT_FALSE(nr.contains_zero);
	EXPECT_GT(nr.zero_margin, 0.0);
	EXPECT_LT(nr.max_phase_deg, 1.0);
	EXPECT_TRUE(nr.pass_phase);
	EXPECT_GT(excessPassivity(Y), 0.0);
	EXPECT_DOUBLE_EQ(shortagePassivity(Y), 0.0);
	EXPECT_TRUE(certifySector(Y, -0.5, 10.0));
}

TEST(GeometricCertificates, AntiPassiveContainsZeroOrLargePhase)
{
	Eigen::MatrixXcd Y(2, 2);
	Y << std::complex<double>(-1.0, 0.5), std::complex<double>(0.0, 0.0),
		std::complex<double>(0.0, 0.0), std::complex<double>(-0.5, -0.2);
	const NumericalRangeCertificate nr = certifyNumericalRange(Y, 90.0, 120);
	// Left-half W: either contains 0 or NR phase is large / passivity fails.
	EXPECT_LT(nr.min_real, 0.0);
	EXPECT_GT(shortagePassivity(Y), 0.0);

	const DwShellCertificate dw = certifyDwShell(Y, 90.0, 10.0, 0.0, 90, 8);
	EXPECT_FALSE(dw.samples.empty());
	EXPECT_GT(dw.max_singular, 0.0);
}

TEST(GeometricCertificates, SmallGainAndDwGateOnPassive)
{
	Eigen::MatrixXcd Y(2, 2);
	Y << std::complex<double>(0.2, 0.0), std::complex<double>(0.0, 0.0),
		std::complex<double>(0.0, 0.0), std::complex<double>(0.3, 0.0);
	EXPECT_LT(spectralNorm(Y), 1.0);
	const DwShellCertificate dw = certifyDwShell(Y, 90.0, 1.0, 0.0, 90, 8);
	EXPECT_TRUE(dw.pass_small_gain);
	EXPECT_TRUE(dw.pass_excess);
	EXPECT_TRUE(dw.nr.pass_phase);

	std::complex<double> Z(5.0, 0.1); // Y = 1/Z small
	Impedance line("Zg", "AC1", 3, Z);
	CertificateSpec spec;
	spec.f_min_Hz = 10.0;
	spec.f_max_Hz = 100.0;
	spec.n_points = 6;
	spec.ac_dq_block = false;
	spec.require_passivity = true;
	spec.require_phase = false;
	spec.require_nr_phase = true;
	spec.require_nr_zero_outside = true;
	spec.require_small_gain = true;
	spec.require_dw = true;
	spec.gain_limit = 2.0;
	const DeviceGateResult g = evaluateDeviceGate(line, "Zg", spec);
	EXPECT_TRUE(g.allow);
	EXPECT_TRUE(g.pass_nr_phase);
	EXPECT_TRUE(g.pass_dw);
}
