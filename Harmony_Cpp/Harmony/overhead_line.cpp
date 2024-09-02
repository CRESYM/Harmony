#include "overhead_line.h"
#include "basic.h"
#include "Constants.h"
#include "Element.h"

#include <stdexcept>
#include <functional>
#include <unordered_map>
#include <cmath>
#include <tuple>
#include <vector>
#include <string>


/*
Basic::Basic(double value) : ptr(nullptr) {
	// Allocate memory for a double and store the value
	double* doublePtr = new double(value);
	// Assign the allocated memory to ptr
	ptr = static_cast<void*>(doublePtr);
}

// Define a type for the function pointer
using EstimateFunction = std::function<std::tuple<double, double>(Conductors&)>;

void overhead_Line :: overhead_line(overhead_Line& tl, std::unordered_map<std::string, std::variant<int, double, std::tuple<std::vector<double>, std::vector<double>>, Symbol>>& kwargs) {
	bool transformation = false;

	// Define the unordered_map of organization functions
	std::unordered_map<std::string, std::function<std::tuple<std::vector<double>, std::vector<double>>(const Conductors&)>> dict_organization = {
		{"flat", &overhead_Line::estimate_flat},
		{"vertical", &overhead_Line::estimate_vertical},
		{"delta", &overhead_Line::estimate_delta},
		{"concentric", &overhead_Line::estimate_concentric},
		{"offset", &overhead_Line::estimate_offset},
		{tl.conductors.organization.name, [&tl](const Conductors& c) { return std::make_tuple(std::vector<double>{0}, std::vector<double>{c.ybc}); }}
	};

	for (const auto&[key, val] : kwargs) {
		if (dict_organization.find(key) != dict_organization.end()) {
			auto& func = dict_organization[key];
			auto result = func(tl.conductors); // Call the function with the Conductors object
			// Process result...
		}
		else {
			throw std::invalid_argument("Unknown organization: " + key);
		}
	}
	for (const auto&[key, val] : kwargs) {
		if (key == "length" || key == "eliminate") {
			if (key == "length") {
				tl.length = std::get<double>(val);
			}
			else { // key == "eliminate"
				tl.eliminate = std::get<int>(val);
			}
		} //check definitions and calculate parameters
		else if (key == "conductors") {
			Conductors& c = tl.conductors;
			const auto& [nb, nsb, ybc, deltaYbc, deltaXbc, deltaTildeXbc, dsag, dsb, rc, RdC, gc, murc, positions, organization] = std::get<std::tuple<int, int, double, double, double, double, double, double, double, double, double, double, std::tuple<std::vector<double>, std::vector<double>>, Symbol>>(val);
			c.nb = nb;
			c.nsb = nsb;
			c.ybc = ybc;
			c.deltaYbc = deltaYbc;
			c.deltaXbc = deltaXbc;
			c.deltaTildeXbc = deltaTildeXbc;
			c.dsag = dsag;
			c.dsb = dsb;
			c.rc = rc;
			c.RdC = RdC;
			c.gc = gc;
			c.murc = murc;
			c.positions = positions;
			c.organization = organization;

			Conductors conductors = std::get<Conductors>(val);
			auto[flat_x, flat_y] = overhead_Line::estimate_flat(conductors);
			auto[vertical_x, vertical_y] = overhead_Line::estimate_vertical(conductors);
			auto[delta_x, delta_y] = overhead_Line::estimate_delta(conductors);
			auto[concentric_x, concentric_y] = overhead_Line::estimate_concentric(conductors);
			auto[offset_x, offset_y] = overhead_Line::estimate_offset(conductors);
		}
		else if (key == "groundwires") {
			Groundwires& g = tl.groundwires;
			const auto&[ng, deltaXg, deltaYg, rg, dgSag, RdG, mug, positions] = std::get<std::tuple<int, double, double, double, double, double, double, std::tuple<std::vector<double>, std::vector<double>>>>(val);
			g.ng = ng;
			g.deltaXg = deltaXg;
			g.deltaYg = deltaYg;
			g.rg = rg;
			g.dgSag = dgSag;
			g.RdG = RdG;
			g.mug = mug;
			g.positions = positions;
		}
		else if (key == "earthParameters") {
			tl.earthParameters = std::get<std::tuple<int, int, int>>(val);
		}
		else if (key == "P") {
			tl.P = std::get<std::vector<Basic>>(val);
		}
		else if (key == "Z") {
			tl.Z = std::get<std::vector<Basic>>(val);
		}
		else if (key == "transformation") {
			transformation = std::get<int>(val);
		}
		else {
			throw std::invalid_argument("Unknown property " + key + " of the overhead line.");
		}
	}

	// Extract earth parameters from tl
	auto[mur_earth, epsilonr_earth, resistivity_earth] = tl.earthParameters;

	// Constants
	double mu_0 = 4 * M_PI * 1e-7;
	double epsilon = 8.85e-12;

	// Calculate earth parameters
	double mu_earth = mur_earth * mu_0;
	double epsilon_earth = epsilonr_earth * epsilon;
	double sigma_earth = 1 / resistivity_earth;

	// Initialize arrays
	std::vector<double> x_array;
	std::vector<double> y_array;
	std::vector<double> r_array;
	std::vector<double> rho_array;
	std::vector<double> mu_array;

	//calculate conductor positions
	if (dict_organization.find(tl.conductors.organization.name) == dict_organization.end() ||
		(std::get<0>(tl.conductors.positions).size() == 0 && std::get<1>(tl.conductors.positions).size() == 0)) {
		throw std::invalid_argument("Conductor positions are not defined.");
	}
	else {
		std::vector<double> x;
		std::vector<double> y;
		if (dict_organization.find(tl.conductors.organization.name) != dict_organization.end()) {
			auto[x_, y_] = dict_organization[tl.conductors.organization.name](tl.conductors);
			x = x_;
			y = y_;
		}
		else {
			std::tie(x, y) = tl.conductors.positions;
		}

		auto[xsb, ysb] = overhead_Line::bundle_position(tl.conductors.nsb, tl.conductors.dsb);
		for (int i = 0; i < tl.conductors.nb; ++i) {
			for (int j = 0; j < tl.conductors.nsb; ++j) {
				x_array.push_back(x[i] + xsb[j]);
				y_array.push_back(y[i] + ysb[j] - 2.0 / 3.0 * tl.conductors.dsag);
				r_array.push_back(tl.conductors.rc);
				rho_array.push_back(tl.conductors.RdC * 1e-3);
				mu_array.push_back(tl.conductors.murc * mu_0);
			}
		}
	}

	// Calculate the number of elements in the matrices
	int Num = tl.conductors.nb * tl.conductors.nsb + tl.groundwires.ng;
	std::vector<std::vector<Basic>> P(Num, std::vector<Basic>(Num));
	std::vector<std::vector<Basic>> Z(Num, std::vector<Basic>(Num));

	double s = 0.0; // Define s, the symbol
	double de = sqrt(1 / (s * mu_earth * (sigma_earth + s * epsilon_earth))); // depth of penetration

	// Loop over each phase and groundwire
	for (int iPhase = 0; iPhase < Num; ++iPhase) {
		double x_i = x_array[iPhase];
		double y_i = y_array[iPhase];
		double r_i = r_array[iPhase];
		double rho = rho_array[iPhase] * (M_PI * r_i * r_i);
		double mu = mu_array[iPhase];
		double m = sqrt(s * mu / rho);

		for (int jPhase = 0; jPhase < Num; ++jPhase) {
			double x_j = x_array[jPhase];
			double y_j = y_array[jPhase];
			double Di_j = sqrt(pow((x_i - x_j), 2) + pow((y_i + y_j), 2));
			double D̂i_j = sqrt(pow((x_i - x_j), 2) + pow((y_i + y_j + 2 * de), 2));
			double di_j = sqrt(pow((x_i - x_j), 2) + pow((y_i - y_j), 2));

			if (iPhase != jPhase) {
				Z[iPhase][jPhase] += Basic(s * log(D̂i_j / di_j) * mu_0 / (2 * M_PI));
				P[iPhase][jPhase] += Basic(1 / (2 * M_PI * epsilon) * log(Di_j / di_j));
			}
			else {
				Z[iPhase][jPhase] += s * mu_0 / (2 * M_PI) * log(D̂i_j / r_i) + m * rho / (2 * M_PI * r_i) * 1.0 / tanh(0.733 * m * r_i) + 0.3179 * rho / (M_PI * r_i * r_i);
				P[iPhase][jPhase] = 1 / (2 * M_PI * epsilon) * log(Di_j / r_i);
			}
		}
	}

	// Assuming tl.P and tl.Z are defined as std::vector<std::vector<Basic>>
// If not, replace the type accordingly

	if (tl.conductors.nsb != 0) {
		std::vector<int> cond_noElim(tl.conductors.nb);
		for (int i = 0; i < tl.conductors.nb; ++i) {
			cond_noElim[i] = (i * tl.conductors.nsb) + 1;
		}

		for (int iPhase = 0; iPhase < tl.conductors.nb; ++iPhase) {
			int cond_noElim_curr = cond_noElim[iPhase];
			for (int iCond = (tl.conductors.nsb * (iPhase + 1)) + 1; iCond <= tl.conductors.nsb * (iPhase + 1); ++iCond) {
				// Subtract Z[:,iCond] from Z[:,cond_noElim_curr]
				for (int i = 0; i < Num; ++i) {
					Z[i][iCond] -= Z[i][cond_noElim_curr];
				}
				// Subtract Z[iCond,:] from Z[cond_noElim_curr,:]
				for (int j = 0; j < Num; ++j) {
					Z[iCond][j] -= Z[cond_noElim_curr][j];
				}

				// Subtract P[:,iCond] from P[:,cond_noElim_curr]
				for (int i = 0; i < Num; ++i) {
					P[i][iCond] -= P[i][cond_noElim_curr];
				}
				// Subtract P[iCond,:] from P[cond_noElim_curr,:]
				for (int j = 0; j < Num; ++j) {
					P[iCond][j] -= P[cond_noElim_curr][j];
				}
			}
		}
	}

	// Assign Z and P to tl.P and tl.Z respectively
	tl.setP(P); // Assign P matrix to tl.P
	tl.setZ(Z); // Assign Z matrix to tl.Z

	Element elem(tl.conductors.nb, tl.conductors.nb, tl, transformation);

}

//check definitions and calculate parameters
std::tuple<std::vector<double>, std::vector<double>> overhead_Line::estimate_flat(const Conductors& c) {
	if (c.nb == 2) {
		return { {-c.deltaXbc / 2, c.deltaXbc / 2}, {c.ybc, c.ybc} };
	}
	else if (c.nb == 3) {
		return { {-c.deltaXbc, 0, c.deltaXbc}, {c.ybc, c.ybc, c.ybc} };
	}
	else if (c.nb == 6) {
		return { {-c.deltaXbc, 0, c.deltaXbc, -c.deltaXbc, 0, c.deltaXbc}, {c.ybc, c.ybc, c.ybc, c.ybc + c.deltaYbc, c.ybc + c.deltaYbc, c.ybc + c.deltaYbc} };
	}
	else {
		throw std::invalid_argument("Invalid definition of flat conductor organization.");
	}
}

std::tuple<std::vector<double>, std::vector<double>> overhead_Line::estimate_vertical(const Conductors& c) {
	if (c.nb == 3) {
		std::vector<double> x;
		std::vector<double> y;
		for (int i = 1; i <= c.nb; ++i) {
			x.push_back(c.deltaXbc / 2);
			y.push_back(c.ybc + i * c.deltaYbc);
		}
		return std::make_tuple(x, y);
	}
	else if (c.nb == 6) {
		std::vector<double> x = { c.deltaXbc / 2, c.deltaXbc / 2, c.deltaXbc / 2, -c.deltaXbc / 2, -c.deltaXbc / 2, -c.deltaXbc / 2 };
		std::vector<double> y = { c.ybc, c.ybc + c.deltaYbc, c.ybc + 2 * c.deltaYbc, c.ybc, c.ybc + c.deltaYbc, c.ybc + 2 * c.deltaYbc };
		return std::make_tuple(x, y);
	}
	else {
		throw std::invalid_argument("Invalid definition of vertical conductor organization.");
	}
}

std::tuple<std::vector<double>, std::vector<double>> overhead_Line::estimate_delta(const Conductors& c) {
	if (c.nb % 3 == 0) {
		if (c.nb == 3) {
			std::vector<double> x = { -c.deltaXbc / 2, c.deltaXbc / 2, 0 };
			std::vector<double> y = { c.ybc, c.ybc + c.deltaYbc, c.ybc };
			return std::make_tuple(x, y);
		}
		else if (c.nb == 6) {
			std::vector<double> x = { -c.deltaXbc / 2 - c.deltaTildeXbc, -c.deltaXbc / 2 - c.deltaTildeXbc / 2, -c.deltaXbc / 2,
									  c.deltaXbc / 2, c.deltaXbc / 2 + c.deltaTildeXbc / 2, c.deltaXbc / 2 + c.deltaTildeXbc };
			std::vector<double> y = { c.ybc, c.ybc + c.deltaYbc, c.ybc, c.ybc, c.ybc + c.deltaYbc, c.ybc };
			return std::make_tuple(x, y);
		}
	}
	else {
		throw std::invalid_argument("Delta cannot be constructed from " + std::to_string(c.nb) + " conductors.");
	}
}

std::tuple<std::vector<double>, std::vector<double>> overhead_Line::estimate_concentric(const Conductors& c) {
	if (c.nb % 3 == 0) {
		if (c.nb == 3) {
			std::vector<double> x = { -c.deltaTildeXbc, 0, 0 };
			std::vector<double> y = { c.ybc + c.deltaYbc, c.ybc, c.ybc + 2 * c.deltaYbc };
			return std::make_tuple(x, y);
		}
		else if (c.nb == 6) {
			std::vector<double> x = { -c.deltaXbc / 2 - c.deltaTildeXbc, -c.deltaXbc / 2, -c.deltaXbc / 2,
									  c.deltaXbc / 2, c.deltaXbc / 2, c.deltaXbc / 2 + c.deltaTildeXbc };
			std::vector<double> y = { c.ybc + c.deltaYbc, c.ybc, c.ybc + 2 * c.deltaYbc,
									  c.ybc, c.ybc + 2 * c.deltaYbc, c.ybc + c.deltaYbc };
			return std::make_tuple(x, y);
		}
	}
	else {
		throw std::invalid_argument("Delta cannot be constructed from " + std::to_string(c.nb) + " conductors.");
	}
}

std::tuple<std::vector<double>, std::vector<double>> overhead_Line::estimate_offset(const Conductors& c) {
	if (c.nb % 3 == 0) {
		if (c.nb == 3) {
			std::vector<double> x = { -c.deltaTildeXbc, 0, 0 };
			std::vector<double> y = { c.ybc + c.deltaYbc, c.ybc, c.ybc + 2 * c.deltaYbc };
			return std::make_tuple(x, y);
		}
		else if (c.nb == 6) {
			std::vector<double> x = { -c.deltaXbc / 2 - c.deltaTildeXbc, -c.deltaXbc / 2, -c.deltaXbc / 2,
									  c.deltaXbc / 2, c.deltaXbc / 2, c.deltaXbc / 2 + c.deltaTildeXbc };
			std::vector<double> y = { c.ybc + c.deltaYbc, c.ybc, c.ybc + 2 * c.deltaYbc,
									  c.ybc, c.ybc + 2 * c.deltaYbc, c.ybc + c.deltaYbc };
			return std::make_tuple(x, y);
		}
	}
	else {
		throw std::invalid_argument("Delta cannot be constructed from " + std::to_string(c.nb) + " conductors.");
	}
}

std::tuple<std::vector<double>, std::vector<double>> overhead_Line::bundle_position(int n, double d) {
	if (n == 1) {
		return std::make_tuple(std::vector<double>{0}, std::vector<double>{0});
	}
	else {
		const double phi = 2 * M_PI / n;
		const double r = d / 2 / sin(phi / 2);
		double phi_s = M_PI / 2;
		if (n % 2 == 0) {
			phi_s += phi / 2;
		}

		std::vector<double> xsb;
		std::vector<double> ysb;
		for (int i = 0; i < n; ++i) {
			xsb.push_back(r * cos(phi_s));
			ysb.push_back(r * sin(phi_s));
			phi_s += phi;
		}

		return std::make_tuple(xsb, ysb);
	}
}*/
