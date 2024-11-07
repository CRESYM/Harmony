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
	else if (organization == "vertical")
		estimate_vertical();
	else if (organization == "delta")
		estimate_delta();
	else if (organization == "concentric")
		estimate_concentric();
	else if (organization == "offset")
		estimate_offset();
	else if (organization == "absolute") {
		if (std::get<0>(positions).size() != number_bundles) {
			throw std::invalid_argument("Uncorrect absolute positions. Number of positions does not match the number of bundles.");
			exit(2);
		}
	}
	else {
		throw std::invalid_argument("Unknown conductor bundle organization.");
		exit(2);
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

// Groundwires definition
Overhead_Line::Groundwires::Groundwires(int ng, std::vector<double>& values, double mu_g = 1.0, std::tuple<std::vector<double>, std::vector<double>> positions = { {},{} })
	: ng(ng), mu_g(mu_g)
{
	if (values.size() < 4) {
		throw std::invalid_argument("Illegal number of parameters. It must be at least 5 (Rgdc, rg, dgsag, DeltaYg) with additional parameters (DeltaXg, mu_g)");
		exit(2);
	}

	Rgdc = values[0]; rg = values[1];
	dgsag = values[2]; deltaYg = values[3];
	deltaXg = (values.size() == 5) ? values[4] : 0;
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
	std::tuple<std::string, std::vector<int>, std::vector<double>, double, double, double, double> conductor,
	std::tuple<int, std::vector<double>, double> groundwire) : length(len), earthParameters(earth), Element(symbol, 1, 1) {

	conductors = new Conductors(std::get<0>(conductor), std::get<1>(conductor), std::get<2>(conductor), std::get<3>(conductor), std::get<4>(conductor), std::get<5>(conductor));
	groundwires = new Groundwires(std::get<0>(groundwire), std::get<1>(groundwire), std::get<2>(groundwire));

	// Calculate earth parameters
	RCP<const Basic> mu_earth = real_double(std::get<0>(earthParameters) * mu_0);
	RCP<const Basic> epsilon_earth = real_double(std::get<1>(earthParameters) * epsilon_0);
	RCP<const Basic> sigma_earth = real_double(1 / std::get<2>(earthParameters));

	// Initialize arrays
	std::vector<double> x_array;
	std::vector<double> y_array;
	std::vector<double> r_array;
	std::vector<double> rho_array;
	std::vector<double> mu_array;

	std::vector<double> x;
	std::vector<double> y;
	std::tie(x, y) = conductors->positions;

	std::vector<double> xsb;
	std::vector<double> ysb;
	std::tie(xsb, ysb) = bundle_position(conductors->number_conductors_bundle, conductors->dsb);
	for (int i = 0; i < conductors->number_bundles; ++i) {
		for (int j = 0; j < conductors->number_conductors_bundle; ++j) {
			x_array.push_back(x[i] + xsb[j]);
			y_array.push_back(y[i] + ysb[j] - 2.0 / 3.0 * conductors->dsag);
			r_array.push_back(conductors->rc);
			rho_array.push_back(conductors->Rdc * 1e-3);
			mu_array.push_back(conductors->mu_rc * mu_0);
		}
	}
	
	// Calculate the number of elements in the matrices
	int Num = conductors->number_bundles * conductors->number_conductors_bundle + groundwires->ng;
	P = createZeroMatrix(Num, Num);
	Z = createZeroMatrix(Num, Num);

	RCP<const Basic> de = sqrt(div(integer(1), mul(mul(s, mu_earth), add(sigma_earth, mul(s, epsilon_earth))))); // depth of penetration
	
	// Loop over each phase and groundwire
	for (int iPhase = 0; iPhase < Num; ++iPhase) {
		double x_i = x_array[iPhase];
		double y_i = y_array[iPhase];
		double r_i = r_array[iPhase];
		double rho = rho_array[iPhase] * (M_PI * r_i * r_i);
		double mu = mu_array[iPhase];
		// double m = sqrt(s * mu / rho);
	
		for (int jPhase = 0; jPhase < Num; ++jPhase) {
			double x_j = x_array[jPhase];
			double y_j = y_array[jPhase];
			double Di_j = sqrt(pow((x_i - x_j), 2) + pow((y_i + y_j), 2));
			// double D̂i_j = sqrt(pow((x_i - x_j), 2) + pow((y_i + y_j + 2 * de), 2));
			double di_j = sqrt(pow((x_i - x_j), 2) + pow((y_i - y_j), 2));
	
			if (iPhase != jPhase) {
				// Z[iPhase][jPhase] += Basic(s * log(D̂i_j / di_j) * mu_0 / (2 * M_PI));
				// P[iPhase][jPhase] += Basic(1 / (2 * M_PI * epsilon) * log(Di_j / di_j));
			}
			else {
				// Z[iPhase][jPhase] += s * mu_0 / (2 * M_PI) * log(D̂i_j / r_i) + m * rho / (2 * M_PI * r_i) * 1.0 / tanh(0.733 * m * r_i) + 0.3179 * rho / (M_PI * r_i * r_i);
				// P[iPhase][jPhase] = 1 / (2 * M_PI * epsilon) * log(Di_j / r_i);
			}
		}
	}
	//
	//
	//	if (conductors.number_conductors_bundle != 0) {
	//		std::vector<int> cond_noElim(conductors.number_bundles);
	//		for (int i = 0; i < conductors.number_bundles; ++i) {
	//			cond_noElim[i] = (i * conductors.number_conductors_bundle) + 1;
	//		}
	//
	//		for (int iPhase = 0; iPhase < tl.conductors.number_bundles; ++iPhase) {
	//			int cond_noElim_curr = cond_noElim[iPhase];
	//			for (int iCond = (tl.conductors.number_conductors_bundle * (iPhase + 1)) + 1; iCond <= tl.conductors.number_conductors_bundle * (iPhase + 1); ++iCond) {
	//				// Subtract Z[:,iCond] from Z[:,cond_noElim_curr]
	//				for (int i = 0; i < Num; ++i) {
	//					Z[i][iCond] -= Z[i][cond_noElim_curr];
	//				}
	//				// Subtract Z[iCond,:] from Z[cond_noElim_curr,:]
	//				for (int j = 0; j < Num; ++j) {
	//					Z[iCond][j] -= Z[cond_noElim_curr][j];
	//				}
	//
	//				// Subtract P[:,iCond] from P[:,cond_noElim_curr]
	//				for (int i = 0; i < Num; ++i) {
	//					P[i][iCond] -= P[i][cond_noElim_curr];
	//				}
	//				// Subtract P[iCond,:] from P[cond_noElim_curr,:]
	//				for (int j = 0; j < Num; ++j) {
	//					P[iCond][j] -= P[cond_noElim_curr][j];
	//				}
	//			}
	//		}
	//	}
}


	

