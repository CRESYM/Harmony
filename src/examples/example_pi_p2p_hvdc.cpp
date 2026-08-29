/**
 * @file example_pi_p2p_hvdc.cpp
 * @brief PowerImpedance P2P HVDC README / P2P_HVDC_ALT analogue (fd OHL + cable).
 */
#include "Examples.h"

#include "../network.h"
#include "../Bus.h"
#include "../Include_components.h"
#include "../Elements/Transmission_Line/overhead_line.h"
#include "../Elements/Transmission_Line/Cable.h"
#include "../Solver/Stability_Estimate/Stability_estimate.h"

#include <filesystem>
#include <fstream>
#include <iomanip>

namespace {

std::vector<double> matchedGflControllers(double pacRef)
{
	return {
		1, 0, 0.000879645943005142, 0.03947841760435743, 1, 0,
		0,
		1, 0, 6.666666666666667e-07, 0.00020943933333333334, 1, pacRef,
		0,
		1, 0, 6.666666666666667e-07, 0.00020943933333333334, 1, 0.0,
		0,
		0,
		1, 0, 115.365, 78414.81, 2, 666.6666666666666, 0.0,
		1, 0, 41.92, 19276.56, 2, 0.0, 0.0,
		0
	};
}

Overhead_Line* makeP2pOhl(const std::string& id, const std::string& area, double lengthKm)
{
	std::vector<int> numbers = {3, 1};
	std::vector<double> distances = {10.0, 30.0};
	std::vector<double> gw = {0.92, 0.0062, 10.0, 7.5, 6.5};
	return new Overhead_Line(
		id, area, lengthKm,
		std::make_tuple(1.0, 1.0, 100.0),
		std::make_tuple("flat", numbers, distances, 0.015, 0.063, 10.0, 0.0),
		std::make_tuple(2, gw, 0.0));
}

Cable* makeP2pCable()
{
	auto* c1 = new Cable::Conductor(0.0, 24.25e-3, 1.72e-8);
	auto* c2 = new Cable::Conductor(41.75e-3, 46.25e-3, 2.2e-7);
	auto* c3 = new Cable::Conductor(49.75e-3, 60.55e-3, 1.8e-7, 10.0);
	auto* i1 = new Cable::Insulator(24.25e-3, 41.75e-3, 2.3);
	auto* i2 = new Cable::Insulator(46.25e-3, 49.75e-3, 2.3);
	auto* i3 = new Cable::Insulator(60.55e-3, 65.75e-3, 2.3);
	std::map<std::string, Cable::Conductor*> conductors = {
		{"C1", c1}, {"C2", c2}, {"C3", c3}};
	std::map<std::string, Cable::Insulator*> insulators = {
		{"I1", i1}, {"I2", i2}, {"I3", i3}};
	std::vector<std::pair<double, double>> positions = {{-0.5, 1.0}, {0.5, 1.0}};
	return new Cable(
		"dc_line", "DC1", 1, "underground", 100e3,
		std::make_tuple(1.0, 1.0, 100.0), conductors, insulators, positions);
}

void writeHarmonyP2pCsv(
	const std::filesystem::path& path,
	double f0, double f1, int nPoints,
	StabilityEstimate& se)
{
	std::filesystem::create_directories(path.parent_path());
	std::ofstream out(path);
	out << std::setprecision(12);
	out << "freq_Hz,Re_H00,Im_H00\n";
	for (int i = 0; i < nPoints; ++i) {
		const double t = (nPoints == 1) ? 0.0 : static_cast<double>(i) / (nPoints - 1);
		const double f = std::pow(10.0, std::log10(f0) + t * (std::log10(f1) - std::log10(f0)));
		const Eigen::MatrixXcd tf = se.compute_transfer_function("c2", "AC", f);
		const std::complex<double> h00 = (tf.rows() > 0 && tf.cols() > 0) ? tf(0, 0) : 0.0;
		out << f << ',' << h00.real() << ',' << h00.imag() << '\n';
	}
}

} // namespace

void example_pi_p2p_hvdc(bool plotting_enabled /*=true*/)
{
	std::cout << "=== PowerImpedance analogue: P2P HVDC (fd OHL + cable) ===\n";

	const double f0 = 50.0;
	const double omega = 2.0 * M_PI * f0;
	const double Vac = 380e3 / std::sqrt(3.0);
	const double Vdc = 640e3;
	const double Pac = 100e6;
	const double Qac = 100e6;

	Network net;

	Bus* B2 = new Bus("B2", "AC1", 3);
	Bus* B3 = new Bus("B3", "AC1", 3);
	Bus* B6 = new Bus("B6", "AC2", 3);
	Bus* B7 = new Bus("B7", "AC2", 3);
	Bus* B4 = new Bus("B4", "DC1", 2);
	Bus* B5 = new Bus("B5", "DC1", 2);
	for (Bus* b : {B2, B3, B6, B7, B4, B5})
		net.addBus(b);

	AC_source* g4 = new AC_source("g4", "AC1", 3, Vac, 0.1);
	AC_source* g1 = new AC_source("g1", "AC2", 3, Vac, 0.1);
	net.addElement(g4);
	net.addElement(g1);
	net.connectElementToBus(g4, 1, B2);
	net.connectElementToBus(g1, 1, B7);

	Overhead_Line* tl1 = makeP2pOhl("tl1", "AC1", 25.0);
	Overhead_Line* tl78 = makeP2pOhl("tl78", "AC2", 90.0);
	Cable* dcLine = makeP2pCable();
	net.addElement(tl1);
	net.addElement(tl78);
	net.addElement(dcLine);
	net.connectElementToBus(tl1, 1, B2);
	net.connectElementToBus(tl1, 2, B3);
	net.connectElementToBus(tl78, 1, B6);
	net.connectElementToBus(tl78, 2, B7);
	net.connectElementToBus(dcLine, 1, B4);
	net.connectElementToBus(dcLine, 2, B5);

	std::vector<double> convPlantC1 = {
		omega, -Pac, Qac, 0.0, Vac, -Pac, Vdc,
		50e-3, 1.07, 10e-3, 400.0, 0.06, 0.535, 200e-6
	};
	std::vector<double> convPlantC2 = {
		omega, Pac, Qac, 0.0, Vac, Pac, Vdc,
		30e-3, 1.07, 10e-3, 400.0, 0.0461, 0.4103, 200e-6
	};

	MMC* c1 = new MMC("c1", "AC1_DC1", convPlantC1, matchedGflControllers(Pac));
	MMC* c2 = new MMC("c2", "AC2_DC1", convPlantC2, matchedGflControllers(Pac));
	net.addElement(c1);
	net.addElement(c2);
	net.connectElementToBus(c1, 1, B3);
	net.connectElementToBus(c1, 2, B4);
	net.connectElementToBus(c2, 1, B6);
	net.connectElementToBus(c2, 2, B5);

	c1->solveEquilibrium();
	c1->computeABCD();
	c2->solveEquilibrium();
	c2->computeABCD();

	const std::filesystem::path outDir =
		std::filesystem::path("benchmarking") / "powerimpedance_examples" / "results";
	std::filesystem::create_directories(outDir);

	try {
		StabilityEstimate se;
		se.add_areas(&net);
		writeHarmonyP2pCsv(outDir / "harmony_p2p_Zdd.csv", 10.0, 1000.0, 100, se);
		if (plotting_enabled) {
			se.bodeplotTF("c2", "AC", 10.0, 1000.0, 100);
		}
		std::cout << "Wrote " << (outDir / "harmony_p2p_Zdd.csv").generic_string() << "\n";
	}
	catch (const std::exception& ex) {
		std::cerr << "StabilityEstimate failed: " << ex.what() << "\n";
	}

	std::cout << "P2P fd analogue complete.\n";
}
