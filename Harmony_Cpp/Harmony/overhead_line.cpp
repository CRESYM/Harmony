#include "overhead_line.h"


// Define constructors for Conductors
Overhead_Line::Conductors::Conductors(std::string organization, std::vector<int>& numbers, std::vector<double>& values_distances, double rc, double Rdc, double dsag, double dsb = 0, std::tuple<std::vector<double>, std::vector<double>> positions = { {},{} })
	: organization(organization), rc(rc), Rdc(Rdc), dsb(dsb), dsag(dsag) {
	if (numbers.size() == 1)
		number_bundles = numbers[0];
	else if (numbers.size() == 2) {
		number_bundles = numbers[0];
		number_conductors_bundle = numbers[1];
	}
	else { // illegal number of parameters
		throw std::invalid_argument("It is possible to add up to 2 arguments: number of conductor bundles and number of subconductors per bundle.");
		exit(2);
	}

	if (values_distances.size() == 2) {
		deltaXbc = values_distances[0]; ybc = values_distances[1];
		deltaYbc = 0; deltaTildeXbc = 0;
	}
	else if (values_distances.size() == 3) {
		deltaXbc = values_distances[0]; ybc = values_distances[1];
		deltaYbc = values_distances[2]; deltaTildeXbc = 0;
	}
	else if (values_distances.size() == 4) {
		deltaXbc = values_distances[0]; ybc = values_distances[1];
		deltaYbc = values_distances[2]; deltaTildeXbc = values_distances[2];
	}
	else {
		throw std::invalid_argument("Illegal number of distance parameters.");
		exit(2);
	}

	if (organization == "flat") {
		estimate_flat();
	}


}

//check definitions and calculate parameters
void Overhead_Line::Conductors::estimate_flat() {

	if (number_bundles == 2) {
		positions = { {-deltaXbc / 2, deltaXbc / 2}, {ybc, ybc} };
	}
	else if (number_bundles == 3) {
		positions = { {-deltaXbc, 0, deltaXbc}, {ybc, ybc, ybc} };
	}
	else if (number_bundles == 6) {
		positions = { {-deltaXbc, 0, deltaXbc, -deltaXbc, 0, deltaXbc}, {ybc, ybc, ybc, ybc + deltaYbc, ybc + deltaYbc, ybc + deltaYbc} };
	}
	else {
		throw std::invalid_argument("Invalid definition of flat conductor organization.");
		exit(2);
	}
}

void Overhead_Line::Conductors::estimate_vertical() {
	if (number_bundles == 3) {
		std::vector<double> x;
		std::vector<double> y;
		for (int i = 1; i <= number_bundles; ++i) {
			x.push_back(deltaXbc / 2);
			y.push_back(ybc + i * deltaYbc);
		}
		positions =  std::make_tuple(x, y);
	}
	else if (number_bundles == 6) {
		std::vector<double> x = { deltaXbc / 2, deltaXbc / 2, deltaXbc / 2, -deltaXbc / 2, -deltaXbc / 2, -deltaXbc / 2 };
		std::vector<double> y = { ybc, ybc + deltaYbc, ybc + 2 * deltaYbc, ybc, ybc + deltaYbc, ybc + 2 * deltaYbc };
		positions = std::make_tuple(x, y);
	}
	else {
		throw std::invalid_argument("Invalid definition of vertical conductor organization.");
		exit(2);
	}
}

void Overhead_Line::Conductors::estimate_delta() {
	if (number_bundles % 3 == 0) {
		if (number_bundles == 3) {
			std::vector<double> x = { -deltaXbc / 2, deltaXbc / 2, 0 };
			std::vector<double> y = { ybc, ybc + deltaYbc, ybc };
			positions = std::make_tuple(x, y);
		}
		else if (number_bundles == 6) {
			std::vector<double> x = { -deltaXbc / 2 - deltaTildeXbc, -deltaXbc / 2 - deltaTildeXbc / 2, -deltaXbc / 2,
									  deltaXbc / 2, deltaXbc / 2 + deltaTildeXbc / 2, deltaXbc / 2 + deltaTildeXbc };
			std::vector<double> y = { ybc, ybc + deltaYbc, ybc, ybc, ybc + deltaYbc, ybc };
			positions = std::make_tuple(x, y);
		}
	}
	else {
		throw std::invalid_argument("Delta cannot be constructed from " + std::to_string(number_bundles) + " conductors.");
		exit(2);
	}
}

void Overhead_Line::Conductors::estimate_concentric() {
	if (number_bundles % 3 == 0) {
		if (number_bundles == 3) {
			std::vector<double> x = { -deltaTildeXbc, 0, 0 };
			std::vector<double> y = { ybc + deltaYbc, ybc, ybc + 2 * deltaYbc };
			positions = std::make_tuple(x, y);
		}
		else if (number_bundles == 6) {
			std::vector<double> x = { -deltaXbc / 2 - deltaTildeXbc, -deltaXbc / 2, -deltaXbc / 2,
									  deltaXbc / 2, deltaXbc / 2, deltaXbc / 2 + deltaTildeXbc };
			std::vector<double> y = { ybc + deltaYbc, ybc, ybc + 2 * deltaYbc,
									  ybc, ybc + 2 * deltaYbc, ybc + deltaYbc };
			positions = std::make_tuple(x, y);
		}
	}
	else {
		throw std::invalid_argument("Delta cannot be constructed from " + std::to_string(number_bundles) + " conductors.");
		exit(2);
	}
}

void Overhead_Line::Conductors::estimate_offset() {
	if (number_bundles % 3 == 0) {
		if (number_bundles == 3) {
			std::vector<double> x = { -deltaTildeXbc, 0, 0 };
			std::vector<double> y = { ybc + deltaYbc, ybc, ybc + 2 * deltaYbc };
			positions = std::make_tuple(x, y);
		}
		else if (number_bundles == 6) {
			std::vector<double> x = { -deltaXbc / 2 - deltaTildeXbc, -deltaXbc / 2, -deltaXbc / 2,
									  deltaXbc / 2, deltaXbc / 2, deltaXbc / 2 + deltaTildeXbc };
			std::vector<double> y = { ybc + deltaYbc, ybc, ybc + 2 * deltaYbc,
									  ybc, ybc + 2 * deltaYbc, ybc + deltaYbc };
			positions = std::make_tuple(x, y);
		}
	}
	else {
		throw std::invalid_argument("Delta cannot be constructed from " + std::to_string(number_bundles) + " conductors.");
		exit(2);
	}
}

std::tuple<std::vector<double>, std::vector<double>> Overhead_Line::bundle_position(int n, double d) {
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
}



Overhead_Line::Overhead_Line(const std::string& symbol, double len, std::tuple<double, double, double> earth,
	std::tuple<std::string, std::vector<int>, std::vector<double>, double, double, double, double> conductor)
	: length(len), earthParameters(earth), Element(symbol, 1, 1) {
	bool transformation = false;

	//const auto& [organization, numbers, values_distance, Rdc, rc, dsb] = std::get<std::tuple<std::string, std::vector<int>&, std::vector<double>&, double, double, double>>(conductor);
	conductors = new Conductors(std::get<0>(conductor), std::get<1>(conductor), std::get<2>(conductor), std::get<3>(conductor), std::get<4>(conductor), std::get<5>(conductor));




}

/*
			Conductors& c = tl.conductors;
			const auto& [number_bundles, number_conductors_bundle, ybc, deltaYbc, deltaXbc, deltaTildeXbc, dsag, dsb, rc, RdC, gc, murc, positions, organization] = std::get<std::tuple<int, int, double, double, double, double, double, double, double, double, double, double, std::tuple<std::vector<double>, std::vector<double>>, std::string>>(conductor);

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

	// Extract earth parameters from tl
	auto[mur_earth, epsilonr_earth, resistivity_earth] = earthParameters;

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

		auto[xsb, ysb] = overhead_Line::bundle_position(tl.conductors.number_conductors_bundle, tl.conductors.dsb);
		for (int i = 0; i < tl.conductors.nb; ++i) {
			for (int j = 0; j < tl.conductors.number_conductors_bundle; ++j) {
				x_array.push_back(x[i] + xsb[j]);
				y_array.push_back(y[i] + ysb[j] - 2.0 / 3.0 * tl.conductors.dsag);
				r_array.push_back(tl.conductors.rc);
				rho_array.push_back(tl.conductors.RdC * 1e-3);
				mu_array.push_back(tl.conductors.murc * mu_0);
			}
		}
	}

	// Calculate the number of elements in the matrices
	int Num = tl.conductors.number_bundles * tl.conductors.number_conductors_bundle + tl.groundwires.ng;
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

	if (tl.conductors.number_conductors_bundle != 0) {
		std::vector<int> cond_noElim(tl.conductors.number_bundles);
		for (int i = 0; i < tl.conductors.number_bundles; ++i) {
			cond_noElim[i] = (i * tl.conductors.number_conductors_bundle) + 1;
		}

		for (int iPhase = 0; iPhase < tl.conductors.number_bundles; ++iPhase) {
			int cond_noElim_curr = cond_noElim[iPhase];
			for (int iCond = (tl.conductors.number_conductors_bundle * (iPhase + 1)) + 1; iCond <= tl.conductors.number_conductors_bundle * (iPhase + 1); ++iCond) {
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

	

}
*/

