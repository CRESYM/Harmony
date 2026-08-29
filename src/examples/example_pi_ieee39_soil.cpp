/**
 * @file example_pi_ieee39_soil.cpp
 * @brief IEEE39 T8_9 OHL earth-resistivity sanity (line-level vs PI NetworkBuilder overlay).
 */
#include "Examples.h"

#include "../network.h"
#include "../Bus.h"
#include "../Include_components.h"
#include "../Elements/Transmission_Line/overhead_line.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>

namespace {

Overhead_Line* makeIeee39T89(double soilRho)
{
	std::vector<int> numbers = {3, 1};
	std::vector<double> distances = {10.0, 30.0};
	std::vector<double> gw = {0.92, 0.0062, 10.0, 7.5, 6.5};
	return new Overhead_Line(
		"T8_9", "AC1", 64.009,
		std::make_tuple(1.0, 1.0, soilRho),
		std::make_tuple("flat", numbers, distances, 0.015, 0.063, 10.0, 0.0),
		std::make_tuple(2, gw, 6.5));
}

void writeLineYCsv(
	const std::string& path,
	Overhead_Line& ohl,
	double f0, double f1, int nPoints)
{
	std::ofstream out(path);
	out << std::setprecision(12);
	out << "freq_Hz,Re_Y11,Im_Y11\n";
	for (int i = 0; i < nPoints; ++i) {
		const double t = (nPoints == 1) ? 0.0 : static_cast<double>(i) / (nPoints - 1);
		const double f = std::pow(10.0, std::log10(f0) + t * (std::log10(f1) - std::log10(f0)));
		const auto Y = ohl.compute_y_parameters(f);
		out << f << ',' << Y[0][0].real() << ',' << Y[0][0].imag() << '\n';
	}
}

} // namespace

void example_pi_ieee39_soil(bool /*plotting_enabled*/ /*=true*/)
{
	std::cout << "=== IEEE39 T8_9 soil-resistivity line sanity (Harmony) ===\n";

	const std::filesystem::path outDir =
		std::filesystem::path("benchmarking") / "powerimpedance_examples" / "results";
	std::filesystem::create_directories(outDir);

	for (double rho : {10.0, 100.0, 1000.0}) {
		Overhead_Line* ohl = makeIeee39T89(rho);
		const std::string tag = "harmony_ieee39_t89_rho" + std::to_string(static_cast<int>(rho)) + ".csv";
		writeLineYCsv((outDir / tag).string(), *ohl, 1.0, 5000.0, 160);
		std::cout << "Wrote " << tag << " (rho=" << rho << " Ohm·m)\n";
		delete ohl;
	}

	std::cout << "Compare with export_pi_ieee39_soil.jl + compare_ieee39_soil.py\n";
}
