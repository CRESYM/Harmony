
#include <complex>
#include <Eigen/Dense>
#include <unsupported/Eigen/MatrixFunctions>
#include <cmath>
#include "cable.h"


//template <typename MatrixType>
//MatrixType matrixCoth(const MatrixType& X)
//{
//	// Compute exp(X) and exp(-X) using Eigen's matrix exponential
//	const Eigen::MatrixExponentialReturnValue<MatrixType> expX = X.exp();
//	const Eigen::MatrixExponentialReturnValue<MatrixType> expNegX = (-X).exp();
//
//	// coth(X) = (exp(X) + exp(-X)) / (exp(X) - exp(-X))
//	return (expX + expNegX) / (expX - expNegX);
//}
////
//template <typename MatrixType>
//MatrixType matrixCsc(const MatrixType& X)
//{
//	// Compute exp(X) and exp(-X) using Eigen's matrix exponential
//	Eigen::MatrixExponentialReturnValue<MatrixType> expX = X.exp();
//	Eigen::MatrixExponentialReturnValue<MatrixType> expNegX = (-X).exp();
//
//	// Compute sinh(X) = (exp(X) - exp(-X)) / 2
//	Eigen::MatrixXd sinhX = (expX - expNegX) / 2.0;
//
//	// Compute csc(X) = 1 / sinh(X)
//	return sinhX.cwiseInverse();  // Element-wise inverse
//}
//
// //Function to compute Z and Y with frequency dependence
//Eigen::MatrixXd compute_Z_with_frequency(const Eigen::MatrixXd& R, const Eigen::MatrixXd& L, double omega) {
//	// Z = R + j * omega * L
//	Eigen::MatrixXd Z = R + Eigen::MatrixXd::Identity(R.rows(), R.cols()) * omega * L;
//	return Z;
//}
//
//Eigen::MatrixXd compute_Y_with_frequency(const Eigen::MatrixXd& C, double omega) {
//	// Y = j * omega * C
//	Eigen::MatrixXd Y = Eigen::MatrixXd::Identity(C.rows(), C.cols()) * std::complex<double>(0, omega) * C;
//	return Y;
//}
//
//Eigen::MatrixXd Cable::compute_y_parameters_nums(const Eigen::MatrixXd& R, const Eigen::MatrixXd& L, const Eigen::MatrixXd& C, double l, double omega)
//{
//    // Step 1: Compute Z and Y matrices based on frequency
//    Eigen::MatrixXd Z = compute_Z_with_frequency(R, L, omega);  // Frequency-dependent Z
//    Eigen::MatrixXd Y = compute_Y_with_frequency(C, omega);     // Frequency-dependent Y
//
//    // Step 2: Compute Gamma = sqrt(Z * Y) using Eigen's matrix square root
//    Eigen::MatrixXd ZY = Z * Y;  // Product of Z and Y
//    Eigen::EigenSolver<Eigen::MatrixXd> solver(ZY);
//    Eigen::MatrixXd Gamma = solver.eigenvalues().real().array().sqrt(); // Eigenvalues' square root as Gamma
//
//    // Step 3: Compute Yc = Z.inverse() * Gamma
//    Eigen::MatrixXd Z_inv = Z.inverse();  // Inverse of Z
//    Eigen::MatrixXd Yc = Z_inv * Gamma;  // Compute Yc
//
//    // Step 4: Initialize the Y parameter matrix
//    int n = Yc.rows();  // Size of the original matrices
//    Eigen::MatrixXd Y_params(2 * n, 2 * n);
//    Y_params.setZero();  // Initialize with zeros
//
//    // Step 5: Compute Gamma_l = Gamma * length (element-wise multiplication)
//    Eigen::MatrixXd Gamma_l = Gamma * l;
//
//    // Step 6: Calculate coth(Gamma_l) and csc(Gamma_l)
//    Eigen::MatrixXd coth_Gamma_l = matrixCoth(Gamma_l);  // coth(Γl)
//    Eigen::MatrixXd csc_Gamma_l = matrixCsc(Gamma_l);    // csc(Γl)
//
//    // Step 7: Fill in the Y parameters matrix
//    Y_params.block(0, 0, n, n) = Yc * coth_Gamma_l;         // Yc * coth(Γl)
//    Y_params.block(0, n, n, n) = -Yc * csc_Gamma_l;        // -Yc * csc(Γl)
//    Y_params.block(n, 0, n, n) = -Yc * csc_Gamma_l;        // -Yc * csc(Γl)
//    Y_params.block(n, n, n, n) = Yc * coth_Gamma_l;        // Yc * coth(Γl)
//
//    return Y_params;
//}


Eigen::MatrixXd coth(const Eigen::MatrixXd& matrix) {
	Eigen::MatrixXd result(matrix.rows(), matrix.cols());
	for (int i = 0; i < matrix.rows(); ++i) {
		for (int j = 0; j < matrix.cols(); ++j) {
			result(i, j) = std::cosh(matrix(i, j)) / std::sinh(matrix(i, j));
		}
	}
	return result;
}

// Function to calculate cosech(x) element-wise for Eigen::MatrixXd
Eigen::MatrixXd cosech(const Eigen::MatrixXd& matrix) {
	Eigen::MatrixXd result(matrix.rows(), matrix.cols());
	for (int i = 0; i < matrix.rows(); ++i) {
		for (int j = 0; j < matrix.cols(); ++j) {
			result(i, j) = 1.0 / std::sinh(matrix(i, j));
		}
	}
	return result;
}

// Function to calculate the Y parameters
Eigen::MatrixXd Cable::compute_y_parameters_nums(
	const Eigen::MatrixXd& Z, // Impedance matrix
	const Eigen::MatrixXd& Y, // Admittance matrix
	double length,            // Length l of the cable
	double omega              // Frequency omega
) {
	// Calculate gamma = sqrt(Z * Y) element-wise
	//Eigen::MatrixXd gamma = (Z * Y).cwiseSqrt();  // Correct matrix multiplicatio
	Eigen::MatrixXd gamma = (Z * Y).array().sqrt(); // element-wise sqrt after matrix multiplication

	std::cout << "Gamma:\n" << gamma << std::endl;

	// Calculate coth(gamma * length) and cosech(gamma * length)
	Eigen::MatrixXd coth_gamma_length = coth(gamma * length);
	Eigen::MatrixXd cosech_gamma_length = cosech(gamma * length);
	std::cout << "Coth(gamma * length):\n" << coth_gamma_length << std::endl;
	std::cout << "Cosech(gamma * length):\n" << cosech_gamma_length << std::endl;
	// Calculate Yc = Z^(-1) * gamma
	Eigen::MatrixXd Yc = Z.inverse() * gamma;

	std::cout << "Yc:\n" << Yc << std::endl;

	// Compute the final Y matrix
	Eigen::MatrixXd Y_top_left = Yc.cwiseProduct(coth_gamma_length);
	Eigen::MatrixXd Y_top_right = -Yc.cwiseProduct(cosech_gamma_length);
	Eigen::MatrixXd Y_bottom_left = -Yc.cwiseProduct(cosech_gamma_length);
	Eigen::MatrixXd Y_bottom_right = Yc.cwiseProduct(coth_gamma_length);

	// Assemble the final Y matrix (should be 3x3, not 6x6)
	Eigen::MatrixXd Y_final(3, 3);
	Y_final << Y_top_left(0, 0), Y_top_right(0, 0), Y_top_right(0, 1),
		Y_bottom_left(0, 0), Y_bottom_right(0, 0), Y_bottom_right(0, 1),
		Y_top_left(1, 0), Y_top_right(1, 0), Y_top_right(1, 1);

	return Y_final;
}

void cable(Cable& c, const std::unordered_map<std::string, std::vector<std::pair<double, double>>>& kwargs) {
	 // Evaluate PI to a double
	double pi_value = rcp_static_cast<const RealDouble>(PI)->as_double();

	}

Cable::Cable(const string& symbol, int pins, const string& type_constructor,
	const string& configuration_constructor, double length_constructor, std::tuple<double, double, double> earth, 
	std::map<string, Conductor*> conductors_constructor, std::map<string, Insulator*> insulators_constructor,
	std::vector<std::pair<double, double>> positions_constructor)
	: Element(symbol, pins, pins), earth_parameters(earth), configuration(configuration_constructor),
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
				d_ij = max(d_ij, conductor.second->ro);
			}

			// Iterate over insulators to find maximum radius
			for (const auto& insulator : insulators) {
				d_ij = max(d_ij, insulator.second->ro);
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
				double dᵢⱼ = sqrt(pow(getPositions()[i].first - getPositions()[j].first, 2) +
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
						max_conductor_radius = max(max_conductor_radius, conductor.second->ro);
					}
					double max_insulator_radius = 0;
					for (const auto& insulator : insulators) {
						max_insulator_radius = max(max_insulator_radius, insulator.second->ro);
					}
					D1 = max(max_conductor_radius, max_insulator_radius);
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


