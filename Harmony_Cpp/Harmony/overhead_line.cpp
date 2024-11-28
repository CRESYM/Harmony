#include "overhead_line.h"


// Define constructors for Conductors
Overhead_Line::Conductors::Conductors(std::string organization, std::vector<int>& numbers, std::vector<double>& values_distances, double rc, double Rdc, double dsag, double dsb = 0, std::tuple<std::vector<double>, std::vector<double>> pos = { {},{} })
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

	std::vector<double> x;
	std::vector<double> y;
	std::tie(x, y) = positions;

	std::vector<double> xsb;
	std::vector<double> ysb;
	std::tie(xsb, ysb) = bundle_position();

	std::vector<double> x_array;
	std::vector<double> y_array;

	for (int i = 0; i < number_bundles; ++i) {
		for (int j = 0; j < number_conductors_bundle; ++j) {
			x_array.push_back(x[i] + xsb[j]);
			y_array.push_back(y[i] + ysb[j] - 2.0 / 3.0 * dsag);
		}
	}

	positions = make_tuple(x_array, y_array);
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

std::tuple<std::vector<double>, std::vector<double>> Overhead_Line::Conductors::bundle_position() {
	if (number_bundles == 1) {
		return std::make_tuple(std::vector<double>{0}, std::vector<double>{0});
	}
	else {
		const double phi = 2 * M_PI / number_bundles;
		const double r = dsb / 2 / sin(phi / 2);
		double phi_s = M_PI / 2;
		if (number_bundles % 2 == 0) {
			phi_s += phi / 2;
		}

		std::vector<double> xsb;
		std::vector<double> ysb;
		for (int i = 0; i < number_bundles; ++i) {
			xsb.push_back(r * cos(phi_s));
			ysb.push_back(r * sin(phi_s));
			phi_s += phi;
		}

		return std::make_tuple(xsb, ysb);
	}
}

// Groundwires definition
Overhead_Line::Groundwires::Groundwires(int ng, std::vector<double>& values, double ybc, double mu_g = 1.0, std::tuple<std::vector<double>, std::vector<double>> pos = { {},{} })
	: ng(ng), mu_g(mu_g)
{
	if (values.size() < 4) {
		throw std::invalid_argument("Illegal number of parameters. It must be at least 5 (Rgdc, rg, dgsag, DeltaYg) with additional parameters (DeltaXg, mu_g)");
		exit(2);
	}

	Rgdc = values[0]; rg = values[1];
	dgsag = values[2]; deltaYg = values[3];
	deltaXg = (values.size() == 5) ? values[4] : 0;

	std::vector<double> x;
	std::vector<double> y;
	for (int i = 0; i < ng; i++) {
		x.push_back(deltaXg * (-(ng - 1.0) / 2 + i));
		y.push_back(deltaYg + ybc -2.0 / 3.0 * dgsag); 
	}
	positions = std::make_tuple(x, y);
}


Overhead_Line::Overhead_Line(const std::string& symbol, double len, std::tuple<double, double, double> earth,
	std::tuple<std::string, std::vector<int>, std::vector<double>, double, double, double, double> conductor,
	std::tuple<int, std::vector<double>, double> groundwire) : length(len), earthParameters(earth), Element(symbol, 1, 1) {

	conductors = new Conductors(std::get<0>(conductor), std::get<1>(conductor), std::get<2>(conductor), std::get<3>(conductor), std::get<4>(conductor), std::get<5>(conductor), std::get<6>(conductor));
	groundwires = new Groundwires(std::get<0>(groundwire), std::get<1>(groundwire), conductors->ybc, std::get<2>(groundwire));

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

	
	for (int i = 0; i < conductors->number_bundles * conductors->number_conductors_bundle; ++i) {
		x_array.push_back(x[i]);
		y_array.push_back(y[i]);
		r_array.push_back(conductors->rc);
		rho_array.push_back(conductors->Rdc * 1e-3);
		mu_array.push_back(conductors->mu_rc * mu_0);
	}

	std::tie(x, y) = groundwires->positions;
	for (int i = 0; i < groundwires->ng; ++i) {
		x_array.push_back(x[i]);
		y_array.push_back(y[i]);
		r_array.push_back(conductors->rc);
		rho_array.push_back(conductors->Rdc * 1e-3);
		mu_array.push_back(conductors->mu_rc * mu_0);
	}
	
	// Calculate the number of elements in the matrices
	int Num = conductors->number_bundles * conductors->number_conductors_bundle + groundwires->ng;
	Y = createZeroMatrix(Num, Num);
	P.resize(Num, Num);
	Z = createZeroMatrix(Num, Num);

	RCP<const Basic> de = sqrt(div(integer(1), mul(mul(s, mu_earth), add(sigma_earth, mul(s, epsilon_earth))))); // depth of penetration
	
	// Loop over each phase and groundwire
	for (int iPhase = 0; iPhase < Num; ++iPhase) {
		double x_i = x_array[iPhase];
		double y_i = y_array[iPhase];
		double r_i = r_array[iPhase];
		double rho = rho_array[iPhase] * (M_PI * r_i * r_i);
		double mu = mu_array[iPhase];
		RCP<const Basic> m = sqrt(mul(s, real_double(mu / rho))); // symbolic variable
		// cout << m->__str__() << endl;
	
		for (int jPhase = 0; jPhase < Num; ++jPhase) {
			double x_j = x_array[jPhase];
			double y_j = y_array[jPhase];
			double Di_j = sqrt(pow((x_i - x_j), 2) + pow((y_i + y_j), 2));
			RCP<const Basic> tildeDi_j = sqrt(add(real_double(pow((x_i - x_j), 2)), pow(add(real_double(y_i + y_j), mul(integer(2), de)), integer(2))));
			double di_j = sqrt(pow((x_i - x_j), 2) + pow((y_i - y_j), 2));
	
			if (iPhase != jPhase) {
				RCP<const Basic> Z0 = mul(mul(s, log(div(tildeDi_j, real_double(di_j)))), real_double(mu_0 / (2 * M_PI)));
				Z.set(iPhase, jPhase, add(Z.get(iPhase, jPhase), Z0));
				// P.set(iPhase, jPhase, real_double(1 / (2 * M_PI * epsilon_0) * log(Di_j / di_j)));
				P(iPhase, jPhase) = 1 / (2 * M_PI * epsilon_0) * log(Di_j / di_j);
			}
			else {
				double exp_help = 0.3179 * rho / (M_PI * r_i * r_i);
				RCP<const Basic> Zi = div(mul(m, real_double(rho / (2 * M_PI * r_i))), tanh(mul(m, real_double(0.733 * r_i))));
				Zi = add(Zi, real_double(exp_help));
				RCP<const Basic> Z0 = mul(mul(s, log(div(tildeDi_j, real_double(r_i)))), real_double(mu_0 / (2 * M_PI)));
				Z.set(iPhase, jPhase, add(Z.get(iPhase, jPhase), add(Zi, Z0)));
				// P.set(iPhase, jPhase, real_double(1 / (2 * M_PI * epsilon_0) * log(Di_j / r_i)));
				P(iPhase, jPhase) = 1 / (2 * M_PI * epsilon_0) * log(Di_j / r_i);
			}
		}
	}	
	
	// Kron reduction preparation
	std::vector<int> cond_noElim;
	if (conductors->number_conductors_bundle != 0) {
		for (int i = 0; i < conductors->number_bundles; ++i) {
			cond_noElim.push_back((i * conductors->number_conductors_bundle) + 1);
		}
		for (int iPhase = 0; iPhase < conductors->number_bundles; ++iPhase) {
			int cond_noElim_curr = cond_noElim[iPhase];
			for (int iCond = (conductors->number_conductors_bundle * (iPhase + 1)) + 1; iCond <= conductors->number_conductors_bundle * (iPhase + 1); ++iCond) {
				// Subtract Z/P[:,iCond] from Z/P[:,cond_noElim_curr]
				for (int i = 0; i < Num; ++i) {
					Z.set(i, iCond, sub(Z.get(i, iCond), Z.get(i, cond_noElim_curr)));
					P(i, iCond) -= P(i, cond_noElim_curr);
				}
				// Subtract Z/P[iCond,:] from Z/P[cond_noElim_curr,:]
				for (int j = 0; j < Num; ++j) {
					Z.set(iCond, j, sub(Z.get(iCond, j), Z.get(cond_noElim_curr, j)));
					P(iCond, j) -= P(cond_noElim_curr, j);
				}
			}
		}
	}

	// Invoke kron reduction
	P = kron_reduction(P, cond_noElim);
	Z = kron_reduction(Z, cond_noElim);

	// Determine Y matrix
	P = P.inverse();
	for (int i = 0; i < conductors->number_conductors_bundle; i++)
		for (int j = 0; j < conductors->number_conductors_bundle; j++) {
			Y.set(i, j, mul(s, real_double(P(i,j))));
			if (i == j) {
				Y.set(i, j, add(Y.get(i, j), real_double(conductors->gc)));
			}
		}
}

Eigen::MatrixXcd Overhead_Line::compute_y_parameters_num(double omega_num)
{
	// Step 1: Compute Z and Y matrices based on frequency
	complex<double> s_num = 1i * complex<double>(2 * M_PI * omega_num);
	Eigen::MatrixXcd Z_num = substitute_symbol(Z, s, s_num);
	Eigen::MatrixXcd Y_num = substitute_symbol(Y, s, s_num);

	// Step 2: Compute Gamma = sqrt(Z * Y) using Eigen's matrix square root
	Eigen::MatrixXcd ZY = Z_num * Y_num;  // Product of Z and Y
	Eigen::MatrixXcd Gamma = ZY.sqrt(); // Eigenvalues' square root as Gamma

	// Step 3: Compute Yc = Z.inverse() * Gamma
	Eigen::MatrixXcd Z_inv = Z_num.inverse();  // Inverse of Z
	Eigen::MatrixXcd Yc = Z_inv * Gamma;  // Compute Yc

	// Step 4: Initialize the Y parameter matrix
	int n = Z.nrows();  // Size of the original matrices
	Eigen::MatrixXcd Y_params(2 * n, 2 * n);
	Y_params.setZero();  // Initialize with zeros

	// Step 5: Compute Gamma_l = Gamma * length (element-wise multiplication)
	Eigen::MatrixXcd Gamma_l = Gamma * length;

	// Step 6: Calculate coth(Gamma_l) and csc(Gamma_l)
	Eigen::MatrixXcd coth_Gamma_l = Gamma_l.cosh() * (Gamma_l.sinh()).inverse();  // coth(Γl)
	Eigen::MatrixXcd csc_Gamma_l = (Gamma_l.sinh()).inverse();    // csc(Γl)

	// Step 7: Fill in the Y parameters matrix
	Y_params.block(0, 0, n, n) = Yc * coth_Gamma_l;         // Yc * coth(Γl)
	Y_params.block(0, n, n, n) = -Yc * csc_Gamma_l;        // -Yc * csc(Γl)
	Y_params.block(n, 0, n, n) = -Yc * csc_Gamma_l;        // -Yc * csc(Γl)
	Y_params.block(n, n, n, n) = Yc * coth_Gamma_l;        // Yc * coth(Γl)

	return Y_params;
}


	

