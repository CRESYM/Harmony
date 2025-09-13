#include "Cable.h"

Cable::Cable(const string& symbol, const std::string& location, int pins, const string& type_constructor,
	const string& configuration_constructor, double length_constructor, std::tuple<double, double, double> earth, 
	std::map<string, Conductor*> conductors_constructor, std::map<string, Insulator*> insulators_constructor,
	std::vector<std::pair<double, double>> positions_constructor)
	: Element(symbol, location, pins, pins), earth_parameters(earth), configuration(configuration_constructor),
	type(type_constructor), conductors(conductors_constructor), insulators(insulators_constructor), 
	positions(positions_constructor), length(length_constructor)
{

	// Iterate through conductors and re-adjust values
	for (const auto& pair : conductors) {
		const std::string& key = pair.first;
		const auto& conductor = pair.second;

		if (key == "C1") {
			// Access and modify the conductor from the Cable object
			if (conductor != nullptr) {
				double area = conductor->area;
				if (area != 0) {
					conductor->resistivity = (conductor->resistivity * M_PI * pow(conductor->ro,2) / area);

					// Update the conductor in the Cable object
					updateConductor("C1", conductor);
				}
			}
		}
		else if (key == "SC") {
			auto conductorSC = getConductor("SC");
			auto conductorC2 = getConductor("C2");

			if (conductorSC && conductorC2) {
				if (conductorSC->area != 0) {
					getConductor("C2")->ri = sqrt(pow(conductorSC->ro,2) - conductorSC->area / M_PI);
				}
				else {
					getConductor("C2")->ri = conductorSC->ri;
				}

				double c2OuterRadius = sqrt((pow(conductorC2->ro, 2) - pow(conductorSC->ro, 2)) *
					conductorSC->resistivity / conductorC2->resistivity + pow(conductorSC->ro, 2));
				getConductor("C2")->ro = c2OuterRadius;

				// Remove the SC conductor
				removeConductor("SC");

				// Change Insulator 1
				getInsulator("I1")->ro = getConductor("C2")->ri;
				auto insulatorI2 = getInsulator("I2");

				// Change Insulator 2 if present
				if (insulatorI2 != nullptr) {
					double x = log(insulatorI2->ro / conductorC2->ro) / log(insulatorI2->ro / insulatorI2->ri);
					insulatorI2->ri = conductorC2->ro;
					insulatorI2->permittivity *= x; // Using 'permittivity' for dielectric constant
					insulatorI2->permeability /= x; // Assuming 'permeability' is the member representing permeability
				}

			}
			else {
				throw std::invalid_argument("There must be present sheath together with screen layer.");

			}
		}

		// Semiconductor configuration
		if (getInsulator("I1") && getInsulator("I1")->a != 0) {
			double x = log(getInsulator("I1")->ro / getInsulator("I1")->ri) / log(getInsulator("I1")->b / getInsulator("I1")->a);
			getInsulator("I1")->permittivity *= x;
			double N = 1.4;
			// Assuming 'permeability' represents relative permeability
			getInsulator("I1")->permeability *= (1 + 2 * pow(M_PI*N, 2) * (pow(getInsulator("I1")->ro, 2) - pow(getInsulator("I1")->ri, 2)) / log(getInsulator("I1")->ro / getInsulator("I1")->ri));
		}
	}


	// Ground parameters
	double mu_g = std::get<0>(earth_parameters) * mu_0; // Ground permittivity
	double epsilon_g = std::get<1>(earth_parameters) * epsilon_0; // Ground permeability
	double sigma_g = 1 / std::get<2>(earth_parameters); // Ground conductivity
	double rho_g = 1 / sigma_g; // Ground resistivity
	double g = 1e-11;

	double d_ij = 0; // Initialize d_ij

	// Define variables for frequency domain implementation
	size_t n_l = conductors.size(); // Number of cable layers
	size_t n = positions.size();    // Number of cables
	int Num = n_l * n;

	Z = createZeroMatrix(Num, Num);
	P.resize(Num, Num);

	size_t i = 0; // External indicator

	for (const auto& pair : conductors) {
		const auto& conductor = pair.second;
		double r_i = conductor->ri;
		double r_o = conductor->ro;
		double mu = conductor->permeability * mu_0;
		double rho = conductor->resistivity;

		RCP<const Basic> m = sqrt(mul(s, real_double(mu / rho))); // Calculate m as symbolic values
		double delta_r = r_o - r_i; // Calculate Delta_r
		RCP<const Basic> Z_aa, Z_bb, Z_ab;

		if (r_i != 0) {
			RCP<const Basic> exp = mul(m, tanh(mul(m, real_double(delta_r))));
			Z_aa = sub(mul(real_double(rho / (2 * M_PI * r_i)), exp), real_double(rho / (2 * M_PI * r_i * (r_i + r_o))));
			Z_bb = add(mul(real_double(rho / (2 * M_PI * r_o)), exp), real_double(rho / (2 * M_PI * r_o * (r_i + r_o))));
		}
		else {
			RCP<const Basic> exp = mul(m, tanh(mul(m, real_double(0.733*r_o))));
			Z_bb = add(mul(real_double(rho / (2 * M_PI * r_o)), exp), real_double(0.3179 * rho / (M_PI * r_o * r_o)));
		}
		Z_ab = mul(mul(m, real_double(rho / (M_PI * (r_o + r_i)))), div(integer(1), sinh(mul(m, real_double(delta_r)))));

		Z.set(i, i, add(Z.get(i, i), Z_bb));


		if (i > 0) { // if there is more than one conducting layer -> enter
			Z.set(i, i - 1, sub(Z.get(i, i - 1), Z_ab));
			Z.set(i - 1, i, sub(Z.get(i - 1, i), Z_ab));
			Z.set(i - 1, i - 1, add(Z.get(i - 1, i - 1), Z_aa));
		}
		if (i == n_l - 1) {

			RCP<const Basic> m = sqrt(mul(s, real_double(mu_g / rho_g))); // with ground permeability μᵍ and resistivity ρᵍ
			double H = 2 * positions[0].second;
			double d_ij = 0; // Calculate dᵢⱼ from both conductors and insulators

			// Iterate over conductors to find maximum radius
			for (const auto& conductor : conductors) {
				d_ij = std::max(d_ij, conductor.second->ro);
			}

			// Iterate over insulators to find maximum radius
			for (const auto& insulator : insulators) {
				d_ij = std::max(d_ij, insulator.second->ro);
			}

			double x = d_ij;
			RCP<const Basic> Z_g = mul(mul(s, real_double(mu_g / (2 * M_PI))), sub(sub(real_double(0.5), log(mul(m, real_double(gamma_num * d_ij / 2)))), mul(m, real_double(2 * H / 3))));
			Z.set(i, i, add(Z.get(i, i), Z_g));

			i++;
		}
	}

	// make shunt admittance -> Insulator
	// Define variables for shunt admittance calculation
	int insulatorIndex = 0; // Re-assign the value 0 to i
	for (const auto& insulatorPair : insulators) {
		Insulator* insulator = insulatorPair.second;
		double r_i = insulator->ri;
		double r_o = insulator->ro;
		double mu = insulator->permeability * mu_0;
		double epsilon = insulator->permittivity * epsilon_0;

		RCP<const Basic> Z_i = mul(s, real_double(mu / (2 * M_PI) * log(r_o / r_i))); // Insulator layer impedance
		double P_i = log(r_o / r_i) / (2 * M_PI * epsilon); // P expression

		Z.set(insulatorIndex, insulatorIndex, add(Z.get(insulatorIndex, insulatorIndex), Z_i)); // For the impedance matrix diagonal values, add also the insulator impedance
		// Update the elements of P matrix for the current insulator
		for (int j = 0; j <= i; j++) {
			P(j,i) += P_i;
			P(i,j) += P_i;
		}

		// Update the elements of P matrix in the submatrix P[1:i,1:i]
		for (int j = 0; j < i; j++) {
			for (int k = 0; k < i; k++) {
				P(j,k) += P_i;
			}
		}
	}

	for (int i = 1; i <= n; ++i) {
		for (int j = 1; j <= n; ++j) {
			// Copy and translate the same conductor impedance matrix
			for (int k = 1; k <= n_l; ++k) {
				for (int l = 1; l <= n_l; ++l) {
					Z.set((i - 1) * n_l + k, (j - 1) * n_l + l, Z.get(k, l));
					P((i - 1) * n_l + k,(j - 1) * n_l + l) = P(k,l);
				}
			}

			// Adding earth return impedance and mutual impedance between cables
			if (j > i) { // Only calculate for distinct pairs (i, j) where j > i
				RCP<const Basic> m = sqrt(mul(s, real_double(mu_g / rho_g))); // with ground permeability μᵍ and resistivity ρᵍ
				double H = getPositions()[i].second + getPositions()[j].second;
				double d_ij = sqrt(pow(getPositions()[i].first - getPositions()[j].first, 2) +
					pow(getPositions()[i].second - getPositions()[j].second, 2));
				double x = abs(getPositions()[i].first - getPositions()[j].first);
				RCP<const Basic> Z_g = mul(mul(s, real_double(mu_g / (2 * M_PI))), sub(sub(real_double(0.5), log(mul(m, real_double(gamma_num * d_ij / 2)))), mul(m, real_double(2 * H / 3))));

				Z.set(i * n_l, j * n_l, add(Z.get(i * n_l, j * n_l), Z_g));
				Z.set(j * n_l, i * n_l, add(Z.get(j * n_l, i * n_l), Z_g));
			}
		}
	}

	// reduction for represention core, sheath and armor
	for (int k = 0; k < n; ++k) {
		for (int l = 0; l < n; ++l) {
			for (int i = n_l - 2; i >= 0; --i) {
				for (int j = 0; j <= i; ++j) {
					// Update Z matrix
					for (int p = 0; p < n_l; ++p) {
						Z.set(l* n_l + p, (k* n_l) + j, add(Z.get(l* n_l + p, (k* n_l) + j), Z.get(l* n_l + p, (k* n_l) + i + 1)));
					}
				}
			}

			for (int i = n_l - 2; i >= 0; --i) {
				for (int j = 0; j <= i; ++j) {
					// Update Z matrix
					for (int p = 0; p < n_l; ++p) {
						Z.set((k* n_l) + j, l* n_l + p, add(Z.get((k* n_l) + j, l* n_l + p), Z.get((k* n_l) + i + 1, l* n_l + p)));
					}
				}
			}
		}
	}

	//if (c.getType == "underground") {
	std::string expectedType = "underground";
	std::string cableType = getType();

	if (cableType == expectedType) {
		for (int i = 1; i < n; ++i) {
			for (int j = 0; j < n; ++j) {
				double H = positions[i].second + positions[j].second; // H = sum of depth of ith and jth cables
				double x = abs(positions[i].first - positions[j].first);
				double y = abs(positions[i].second - positions[j].second);
				double D1, D2;
				if (i == j) {
					double max_conductor_radius = 0;
					for (const auto& conductor : conductors) {
						max_conductor_radius = std::max(max_conductor_radius, conductor.second->ro);
					}
					double max_insulator_radius = 0;
					for (const auto& insulator : insulators) {
						max_insulator_radius = std::max(max_insulator_radius, insulator.second->ro);
					}
					D1 = std::max(max_conductor_radius, max_insulator_radius);
					D2 = H;
				}
				else {
					D1 = sqrt(x * x + y * y);
					D2 = sqrt(x * x + H * H);
				}
				double P_ij = log(D2 / D1) / (2 * M_PI * epsilon_0);
				for (int k = i * n_l; k < (i + 1) * n_l; ++k) {
					for (int l = j * n_l; l < (j + 1) * n_l; ++l) {
						P(k,l) += P_ij;
					}
				}
			}
		}
	}

	// Matrices Z, and P are created
	// Next -> Kron reduction and determination of Y matrix

	// Kron reduction preparation
	std::vector<int> cond_noElim;
	if (eliminate) {
		for (int i = 0; i < n; ++i) {
			cond_noElim.push_back((i-1)*n_l + 1);
		}

		// Invoke kron reduction
		P = kron_reduction(P, cond_noElim);
		Z = kron_reduction(Z, cond_noElim);
	}

	P = P.inverse();
	Y = createZeroMatrix(Z.nrows(), Z.ncols());
	for (int i = 0; i < Y.nrows(); i++)
		for (int j = 0; j < Y.ncols(); j++) 
			Y.set(i, j, mul(s, real_double(P(i, j))));

	// Calculation of Y parameters
}

// Destructor definition
Cable::~Cable() {
	// Implement the destructor if needed
}


Eigen::MatrixXcd Cable::compute_y_parameters_num(double omega_num)
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

