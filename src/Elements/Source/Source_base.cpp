/**
 * @file Source_base.cpp
 * @brief Implementation of Base class for AC and DC voltage sources and generators.
 */
#include "Source_base.h"

#include <cmath>

std::vector<std::vector<std::complex<double>>> Source_base::yFromSeriesResistance() const
{
	const int pins = input_pins;
	const int n = 2 * pins;
	std::vector<std::vector<std::complex<double>>> Y(
		static_cast<size_t>(n),
		std::vector<std::complex<double>>(static_cast<size_t>(n), { 0.0, 0.0 }));
	for (int i = 0; i < pins; ++i) {
		double z = 0.0;
		if (i < static_cast<int>(Zsrc.size()))
			z = Zsrc[static_cast<size_t>(i)];
		else if (!Zsrc.empty())
			z = Zsrc[0];
		if (!std::isfinite(z) || std::abs(z) < 1e-12)
			z = 1e-12;
		const std::complex<double> g(1.0 / z, 0.0);
		Y[static_cast<size_t>(i)][static_cast<size_t>(i)] = g;
		Y[static_cast<size_t>(pins + i)][static_cast<size_t>(pins + i)] = g;
		Y[static_cast<size_t>(i)][static_cast<size_t>(pins + i)] = -g;
		Y[static_cast<size_t>(pins + i)][static_cast<size_t>(i)] = -g;
	}
	return Y;
}

// Power flow computations for AC and DC networks
void Source_base::computePowerFlow(std::map<std::string, double>& gen,
    std::map<std::string, double>& globalParams) const {

	string area = element_location.substr(0, 2); // Extract area code from element_location

	if ((area[0] == 'A' || area[0] == 'a') && (area[1] == 'c' || area[1] == 'C')) { // AC network
		for (auto& [key, value] : element_OPF_info)
			gen[key] = value;

		gen["grid"] = (int)element_location[2] - '0'; // Example of setting grid based on element_location
		gen["area"] = (int)element_location[2] - '0'; // Example of setting area based on element_location
		gen["Vg"] = V[0] * 1.0 / 1e3 / globalParams["ACbaseKV"]; // Convert to kV for OPF
		gen["Zsrc"] = Zsrc[0];
	}
	else 
		throw std::runtime_error("Power flow is defined only for AC networks.");
}