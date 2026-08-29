/**
 * @file example_pi_branch_shunt.cpp
 * @brief PowerImpedance Gridspace_uncertainty / SmallSignal analogue: branch||shunt Z(f).
 *
 * Mirrors the deterministic cases in PowerImpedance.jl examples
 * Gridspace_uncertainty.jl and SmallSignal_Gridspace.jl (Z_branch in {8,10},
 * Z_shunt = 20 or 4). Exports driving-point admittance/impedance at the bus.
 */
#include "Examples.h"

#include "../network.h"
#include "../Bus.h"
#include "../Include_components.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>

namespace {

void writeDrivingPointCsv(
	const std::string& path,
	double zBranch,
	double zShunt,
	double f0,
	double f1,
	int nPoints)
{
	std::filesystem::create_directories(std::filesystem::path(path).parent_path());
	std::ofstream out(path);
	out << "freq_Hz,Re_Y,Im_Y,Re_Z,Im_Z,Re_Z_analytic,Im_Z_analytic\n";
	out << std::setprecision(12);

	const std::complex<double> yBranch = 1.0 / zBranch;
	const std::complex<double> yShunt = 1.0 / zShunt;
	const std::complex<double> yBus = yBranch + yShunt;
	const std::complex<double> zAnalytic = 1.0 / yBus;

	// Frequency-independent resistors: Y is constant; still write the sweep
	// grid so compare scripts align with PowerImpedance frequency axes.
	for (int i = 0; i < nPoints; ++i) {
		const double t = (nPoints == 1) ? 0.0 : static_cast<double>(i) / (nPoints - 1);
		const double f = std::pow(10.0, std::log10(f0) + t * (std::log10(f1) - std::log10(f0)));
		const std::complex<double> z = zAnalytic;
		out << f << ','
			<< yBus.real() << ',' << yBus.imag() << ','
			<< z.real() << ',' << z.imag() << ','
			<< zAnalytic.real() << ',' << zAnalytic.imag() << '\n';
	}
}

} // namespace

void example_pi_branch_shunt(bool plotting_enabled /*=true*/)
{
	std::cout << "=== PowerImpedance analogue: branch || shunt (Gridspace_uncertainty) ===\n";

	const std::filesystem::path outDir =
		std::filesystem::path("benchmarking") / "powerimpedance_examples" / "results";
	std::filesystem::create_directories(outDir);

	// Deterministic Grid cases from Gridspace_uncertainty.jl: z = 8 and 10, shunt = 20.
	for (double zBranch : {8.0, 10.0}) {
		Network net;
		Bus* bus = new Bus("bus", "AC1", 1);
		Bus* gnd = new Bus("gnd", "AC1", 1);
		net.addBus(bus);
		net.addBus(gnd);

		Impedance* branch = new Impedance("branch", "AC1", 1, zBranch);
		Impedance* shunt = new Impedance("shunt", "AC1", 1, 20.0);
		net.addElement(branch);
		net.addElement(shunt);
		net.connectElementToBus(branch, 1, bus);
		net.connectElementToBus(branch, 2, gnd);
		net.connectElementToBus(shunt, 1, bus);
		net.connectElementToBus(shunt, 2, gnd);

		const auto Yb = branch->compute_y_parameters(50.0);
		const auto Ys = shunt->compute_y_parameters(50.0);
		std::cout << "z_branch=" << zBranch << "  Y_branch(50Hz)[0][0]=" << Yb[0][0]
			<< "  Y_shunt[0][0]=" << Ys[0][0] << "\n";

		const std::string tag = (zBranch == 8.0) ? "z8" : "z10";
		writeDrivingPointCsv(
			(outDir / ("harmony_branch_shunt_" + tag + ".csv")).string(),
			zBranch, 20.0, 1.0, 1e3, 40);

		if (plotting_enabled) {
			branch->plotYParameters(1.0, 1e3, 40);
		}
	}

	// SmallSignal_Gridspace.jl uses shunt = 4 with uncertain branch ~ U(2,5).
	// Fixed mid-point branch = 3.5 for a single deterministic Nyquist-ready case.
	{
		Network net;
		Bus* bus = new Bus("bus", "AC1", 1);
		Bus* gnd = new Bus("gnd", "AC1", 1);
		net.addBus(bus);
		net.addBus(gnd);
		Impedance* branch = new Impedance("branch", "AC1", 1, 3.5);
		Impedance* shunt = new Impedance("shunt", "AC1", 1, 4.0);
		net.addElement(branch);
		net.addElement(shunt);
		net.connectElementToBus(branch, 1, bus);
		net.connectElementToBus(branch, 2, gnd);
		net.connectElementToBus(shunt, 1, bus);
		net.connectElementToBus(shunt, 2, gnd);

		writeDrivingPointCsv(
			(outDir / "harmony_branch_shunt_smallsignal.csv").string(),
			3.5, 4.0, 1.0, 1e3, 80);
		std::cout << "SmallSignal fixed case z_branch=3.5, z_shunt=4 written.\n";
	}

	std::cout << "CSVs written under " << outDir.generic_string() << "\n";
}
