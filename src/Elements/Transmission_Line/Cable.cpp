#include "Cable.h"

/**
 * @brief Constructs a new Cable object.
 * @details This constructor initializes a cable element with its physical and electrical properties. It performs extensive calculations to model the cable's behavior, including impedance and admittance matrix computations using symbolic mathematics. The constructor handles various configurations, such as single-core, multi-layer cables, and multi-cable systems, and accounts for earth return path effects.
 *
 * @param symbol The unique identifier for the cable element.
 * @param location The location of the element within the circuit.
 * @param pins The number of connection pins for the element.
 * @param type_constructor The type of the cable (e.g., "underground").
 * @param configuration_constructor A string describing the cable's physical configuration.
 * @param length_constructor The length of the cable in meters.
 * @param earth A tuple containing the ground properties: relative permeability (μᵣ), relative permittivity (εᵣ), and resistivity (ρ) in Ωm.
 * @param conductors_constructor A map of conductor layers, where the key is a string identifier (e.g., "C1", "SC") and the value is a pointer to a Conductor object.
 * @param insulators_constructor A map of insulator layers, where the key is a string identifier (e.g., "I1") and the value is a pointer to an Insulator object.
 * @param positions_constructor A vector of (x, y) coordinate pairs specifying the position of each cable in a multi-cable system.
 */
Cable::Cable(const string& symbol, const std::string& location, int pins, const string& type_constructor,
	double length_constructor, std::tuple<double, double, double> earth, 
	std::map<string, Conductor*> conductors_constructor, std::map<string, Insulator*> insulators_constructor,
	std::vector<std::pair<double, double>> positions_constructor)
	: Element(symbol, location, pins, pins), earth_parameters(earth),
	type(type_constructor), conductors(conductors_constructor), insulators(insulators_constructor), 
	positions(positions_constructor), length(length_constructor)
    // --- Begin replacement constructor body ---
{
    updateLayers();

    // Ground parameters
    double mu_g = std::get<0>(earth_parameters) * mu_0;     // ground permeability (relative * mu0)
    double epsilon_g = std::get<1>(earth_parameters) * epsilon_0; // ground permittivity
    double sigma_g = 1.0 / std::get<2>(earth_parameters);       // ground conductivity
    double rho_g = 1.0 / sigma_g;                            // ground resistivity
    double g_const = 1e-11;

    // Make sizes
    const size_t n_l = conductors.size(); // number of layers per cable
    const size_t n = positions.size();  // number of cables
    const int Num = static_cast<int>(n_l * n);

    // Prepare matrices (assume createZeroMatrix and DenseMatrix behave like before)
    Z = createZeroMatrix(Num, Num);
    P.resize(Num, Num);
    P.setZero(Num, Num);  // Eigen way to zero-initialize

    // -----------------------------
    // 1) Build series impedance for a single cable layer stack (Z[0:n_l-1,0:n_l-1])
    // -----------------------------
    int i_layer = 0; // index of current conducting layer
    for (const auto& pair : conductors) {
        const auto& conductor = pair.second;
        double r_i = conductor->ri;
        double r_o = conductor->ro;
        double mu_layer = conductor->permeability * mu_0;
        double rho = conductor->resistivity;

        // symbolic m = sqrt(s * mu / rho)
        RCP<const Basic> m = sqrt(mul(s, real_double(mu_layer / rho)));
        double delta_r = r_o - r_i;

        RCP<const Basic> Z_aa = real_double(0);
        RCP<const Basic> Z_bb = real_double(0);
        RCP<const Basic> Z_ab = real_double(0);

        if (r_i != 0) {
            // hollow conductor case
            RCP<const Basic> coth_term = coth(mul(m, real_double(delta_r)));
            Z_aa = sub(mul(real_double(rho / (2.0 * M_PI * r_i)), mul(m, coth_term)),
                real_double(rho / (2.0 * M_PI * r_i * (r_i + r_o))));
            Z_bb = add(mul(real_double(rho / (2.0 * M_PI * r_o)), mul(m, coth_term)),
                real_double(rho / (2.0 * M_PI * r_o * (r_i + r_o))));
        }
        else {
            // solid conductor
            RCP<const Basic> coth_approx = coth(mul(m, real_double(0.733 * r_o))); // used same style as original
            Z_bb = add(mul(real_double(rho / (2.0 * M_PI * r_o)), mul(m, coth_approx)),
                real_double(0.3179 * rho / (M_PI * r_o * r_o)));
        }
        // off-diagonal coupling inside conductor layers
        Z_ab = mul(mul(m, real_double(rho / (M_PI * (r_o + r_i)))),
            div(integer(1), sinh(mul(m, real_double(delta_r)))));

        // add computed elements into Z (diagonal and coupling with adjacent layer)
        Z.set(i_layer, i_layer, add(Z.get(i_layer, i_layer), Z_bb));
        if (i_layer > 0) {
            Z.set(i_layer, i_layer - 1, sub(Z.get(i_layer, i_layer - 1), Z_ab));
            Z.set(i_layer - 1, i_layer, sub(Z.get(i_layer - 1, i_layer), Z_ab));
            Z.set(i_layer - 1, i_layer - 1, add(Z.get(i_layer - 1, i_layer - 1), Z_aa));
        }

        // If this is the outermost conductor (last layer), add ground return Zg
        if (i_layer == static_cast<int>(n_l) - 1) {
            RCP<const Basic> m_g = sqrt(mul(s, real_double(mu_g / rho_g)));
            double H = 2.0 * positions[0].second;

            // compute maximum radius among conductors and insulators
            double d_ij = 0.0;
            for (const auto& cPair : conductors) d_ij = std::max(d_ij, cPair.second->ro);
            for (const auto& iPair : insulators) d_ij = std::max(d_ij, iPair.second->ro);

            RCP<const Basic> Z_g = mul(mul(s, real_double(mu_g / (2.0 * M_PI))),
                sub(sub(real_double(0.5),
                    log(mul(m_g, real_double(gamma_num * d_ij / 2.0)))),
                    mul(m_g, real_double(2.0 * H / 3.0))));
            Z.set(i_layer, i_layer, add(Z.get(i_layer, i_layer), Z_g));
        }

        // increment layer index (IMPORTANT: do this every iteration)
        ++i_layer;
    }

    // -----------------------------
    // 2) Shunt admittance: insulator contributions (P) and insulator impedances to Z
    //    add P_i to the submatrix P[0:insIndex, 0:insIndex].
    // -----------------------------
    int insIndex = 0;
    for (const auto& insPair : insulators) {
        Insulator* ins = insPair.second;
        double r_i = ins->ri;
        double r_o = ins->ro;
        double mu_layer = ins->permeability * mu_0;
        double eps_layer = ins->permittivity * epsilon_0;

        RCP<const Basic> Z_i = mul(s, real_double(mu_layer / (2.0 * M_PI) * log(r_o / r_i)));
        double P_i = log(r_o / r_i) / (2.0 * M_PI * eps_layer);

        // diagonal addition to Z (insulator self impedance)
        Z.set(insIndex, insIndex, add(Z.get(insIndex, insIndex), Z_i));

        // P[0:insIndex, 0:insIndex] += P_i (match Julia P[1:i,1:i] += ones(i,i)*P_i)
        for (int r = 0; r <= insIndex; ++r) {
            for (int c = 0; c <= insIndex; ++c) {
                // P is Eigen::MatrixXd (or Matrix-like). Add P_i to each element.
                P(r, c) += P_i;
            }
        }

        ++insIndex;
    }

    // -----------------------------
    // 3) Expand single-cable block to multi-cable system and add mutual earth impedances
    //    Use 0-based indices everywhere. Copy base Z[0..n_l-1, 0..n_l-1] into blocks Z[(i*n_l)..((i+1)*n_l-1), (j*n_l)..]
    // -----------------------------
    for (int i = 0; i < static_cast<int>(n); ++i) {
        for (int j = 0; j < static_cast<int>(n); ++j) {
            // Copy base block Z[0..n_l-1, 0..n_l-1] into the (i,j) block
            for (int k = 0; k < static_cast<int>(n_l); ++k) {
                for (int l = 0; l < static_cast<int>(n_l); ++l) {
                    Z.set((i * n_l) + k, (j * n_l) + l, Z.get(k, l));
                    P((i * n_l) + k, (j * n_l) + l) = P(k, l);
                }
            }
        }
    }

    // Add earth return (mutual) impedances and mutual coupling between different cables
    for (int i = 0; i < static_cast<int>(n); ++i) {
        for (int j = i + 1; j < static_cast<int>(n); ++j) { // j > i as in Julia
            RCP<const Basic> m_g = sqrt(mul(s, real_double(mu_g / rho_g)));
            double H = positions[i].second + positions[j].second;
            double dx = positions[i].first - positions[j].first;
            double dy = positions[i].second - positions[j].second;
            double d_ij_num = sqrt(dx * dx + dy * dy);

            RCP<const Basic> Z_g = mul(mul(s, real_double(mu_g / (2.0 * M_PI))),
                sub(sub(real_double(0.5),
                    log(mul(m_g, real_double(gamma_num * d_ij_num / 2.0)))),
                    mul(m_g, real_double(2.0 * H / 3.0))));

            // Add mutual Zg to the last layer entry of each cable (index = (i*n_l + n_l - 1))
            int idx_i = i * n_l + static_cast<int>(n_l) - 1;
            int idx_j = j * n_l + static_cast<int>(n_l) - 1;
            Z.set(idx_i, idx_j, add(Z.get(idx_i, idx_j), Z_g));
            Z.set(idx_j, idx_i, add(Z.get(idx_j, idx_i), Z_g));
        }
    }

    // -----------------------------
    // 4) Reduction for core/sheath/armor (same algorithm as Julia, translated to 0-based)
    // -----------------------------
    for (int k = 0; k < static_cast<int>(n); ++k) {
        for (int l = 0; l < static_cast<int>(n); ++l) {
            for (int i = static_cast<int>(n_l) - 2; i >= 0; --i) {
                for (int j = 0; j <= i; ++j) {
                    for (int p = 0; p < static_cast<int>(n_l); ++p) {
                        Z.set(l * n_l + p, k * n_l + j, add(Z.get(l * n_l + p, k * n_l + j),
                            Z.get(l * n_l + p, k * n_l + i + 1)));
                    }
                }
            }

            for (int i = static_cast<int>(n_l) - 2; i >= 0; --i) {
                for (int j = 0; j <= i; ++j) {
                    for (int p = 0; p < static_cast<int>(n_l); ++p) {
                        Z.set(k * n_l + j, l * n_l + p, add(Z.get(k * n_l + j, l * n_l + p),
                            Z.get(k * n_l + i + 1, l * n_l + p)));
                    }
                }
            }
        }
    }

    // -----------------------------
    // 5) Underground P matrix additions (P_ij) 
    // -----------------------------
    if (getType() == "underground") {
        for (int i = 0; i < static_cast<int>(n); ++i) {
            for (int j = 0; j < static_cast<int>(n); ++j) {
                double H = positions[i].second + positions[j].second;
                double x = std::abs(positions[i].first - positions[j].first);
                double y = std::abs(positions[i].second - positions[j].second);

                double D1, D2;
                if (i == j) {
                    double max_conductor_radius = 0.0;
                    for (const auto& cPair : conductors) max_conductor_radius = std::max(max_conductor_radius, cPair.second->ro);
                    double max_insulator_radius = 0.0;
                    for (const auto& iPair : insulators) max_insulator_radius = std::max(max_insulator_radius, iPair.second->ro);
                    D1 = std::max(max_conductor_radius, max_insulator_radius);
                    D2 = H;
                }
                else {
                    D1 = sqrt(x * x + y * y);
                    D2 = sqrt(x * x + H * H);
                }

                double P_ij = log(D2 / D1) / (2.0 * M_PI * epsilon_0);
                // Add P_ij to the n_l x n_l block for cables i and j
                for (int k = i * n_l; k < (i + 1) * n_l; ++k) {
                    for (int l = j * n_l; l < (j + 1) * n_l; ++l) {
                        P(k, l) += P_ij;
                    }
                }
            }
        }
    }

    // -----------------------------
    // 6) Kron reduction (prepare cond_noElim) and compute matrices Y and reduced Z
    //    cond_noElim = [0, n_l, 2*n_l, ...]
    // -----------------------------
    std::vector<int> cond_noElim;
    if (eliminate) {
        for (int idx = 0; idx < static_cast<int>(n); ++idx) {
            cond_noElim.push_back(idx * static_cast<int>(n_l)); // 0-based core index
        }
        P = kron_reduction(P, cond_noElim);
        Z = kron_reduction(Z, cond_noElim);
    }

    // invert P to build Y = s * P^-1
    //MatrixXd P_inv = P.inverse();
    Eigen::LLT<Eigen::MatrixXd> llt(P);
    MatrixXd P_inv = llt.solve(Eigen::MatrixXd::Identity(P.rows(), P.cols()));
    int final_size = static_cast<int>(cond_noElim.size());
    // resize/reassign Z to final_size x final_size (assuming kron_reduction returned full-size DenseMatrix,
    // but we still ensure Z size matches)
    Z.resize(final_size, final_size);
    // Build Y symbolic matrix using s * P_inv
    Y = createZeroMatrix(Z.nrows(), Z.ncols());
    for (int r = 0; r < Y.nrows(); ++r) {
        for (int c = 0; c < Y.ncols(); ++c) {
            Y.set(r, c, mul(s, real_double(P_inv(r, c))));
        }
    }

    // Prepare Y_matrix placeholder 
    Y_matrix = createZeroMatrix(2 * final_size, 2 * final_size);
}

// Destructor definition
Cable::~Cable() {
	// Implement the destructor if needed
}

std::vector<std::vector<complex<double>>> Cable::compute_y_parameters(double frequency)
{
    // Step 1: Compute Z and Y matrices based on frequency
    double angular_frequency = 2 * frequency * M_PI;
    map_basic_basic m;
    m[omega] = real_double(angular_frequency);
    int n = Z.nrows();  // Size of the original matrices
    std::vector<std::vector<complex<double>>> Y_val_exact(2 * n);
    for (int i = 0; i < 2 * n; i++)
        Y_val_exact[i].resize(2 * n);

    Eigen::MatrixXcd Z_num = substitute_symbol(Z, omega, angular_frequency);
    Eigen::MatrixXcd Y_num = substitute_symbol(Y, omega, angular_frequency);

    // Step 2: Compute Gamma = sqrt(Z * Y) using Eigen's matrix square root
    Eigen::MatrixXcd ZY = Z_num * Y_num;  // Product of Z and Y
    Eigen::MatrixXcd Gamma = ZY.sqrt();

    // Step 3: Compute Yc = Z.inverse() * Gamma
    Eigen::MatrixXcd Z_inv = Z_num.inverse();  // Inverse of Z
    Eigen::MatrixXcd Yc = Z_inv * Gamma;  // Compute Yc

    // Step 4: Compute Gamma_l = Gamma * length (element-wise multiplication)
    Eigen::MatrixXcd Gamma_l = Gamma * length;

    // Step 5: Calculate coth(Gamma_l) and csc(Gamma_l)
    Eigen::MatrixXcd coth_Gamma_l = Gamma_l.cosh() * (Gamma_l.sinh()).inverse();  // coth(Γl)
    Eigen::MatrixXcd csc_Gamma_l = (Gamma_l.sinh()).inverse();    // csc(Γl)

    // Step 6: Initialize the matrix blocks
    Eigen::MatrixXcd Y11 = Yc * coth_Gamma_l;         // Yc * coth(Γl)
    Eigen::MatrixXcd Y12 = -Yc * csc_Gamma_l;        // -Yc * csc(Γl)

    // Step 7: Fill in the Y parameters matrix
    for (int i = 0; i < Y11.rows(); ++i) {
        for (int j = 0; j < Y11.cols(); ++j) {
            Y_val_exact[i][j] = Y11(i, j);
            Y_val_exact[i][j + Y11.cols()] = Y12(i, j);
            Y_val_exact[i + Y11.rows()][j] = Y12(i, j);
            Y_val_exact[i + Y11.rows()][j + Y11.cols()] = Y11(i, j);
        }
    }

    //cout << Y_val_exact[0][0] << endl;

    return Y_val_exact;
}


void Cable::updateLayers() {
	// Iterate through conductors and re-adjust values
	for (const auto& pair : conductors) {
		const std::string& key = pair.first;
		const auto& conductor = pair.second;

		if (key == "C1") {
			// Access and modify the conductor from the Cable object
			if (conductor != nullptr) {
				double area = conductor->area;
				if (area != 0) {
					conductor->resistivity = (conductor->resistivity * M_PI * pow(conductor->ro, 2) / area);

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
					getConductor("C2")->ri = sqrt(pow(conductorSC->ro, 2) - conductorSC->area / M_PI);
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
			getInsulator("I1")->permeability *= (1 + 2 * pow(M_PI * N, 2) * (pow(getInsulator("I1")->ro, 2) - pow(getInsulator("I1")->ri, 2)) / log(getInsulator("I1")->ro / getInsulator("I1")->ri));
		}
	}
}

void Cable::printElementValues() {
	std::cout << "Cable Element Values:" << std::endl;
	std::cout << "Type: " << type << std::endl;
	std::cout << "Length: " << length << " meters" << std::endl;
	std::cout << "Earth Parameters (mu_r, epsilon_r, rho [Omega m]): (" 
		<< std::get<0>(earth_parameters) << ", "
		<< std::get<1>(earth_parameters) << ", "
		<< std::get<2>(earth_parameters) << ")" << std::endl;
	std::cout << "Conductors:" << std::endl;
	for (const auto& pair : conductors) {
		const std::string& key = pair.first;
		const auto& conductor = pair.second;
		std::cout << "  " << key << ": "
			<< "ri = " << conductor->ri << " m, "
			<< "ro = " << conductor->ro << " m, "
			<< "resistivity = " << conductor->resistivity << " Omega m, "
			<< "permeability = " << conductor->permeability << ", "
			<< "area = " << conductor->area << " m^2" << std::endl;
	}
	std::cout << "Insulators:" << std::endl;
	for (const auto& pair : insulators) {
		const std::string& key = pair.first;
		const auto& insulator = pair.second;
		std::cout << "  " << key << ": "
			<< "ri = " << insulator->ri << " m, "
			<< "ro = " << insulator->ro << " m, "
			<< "permittivity = " << insulator->permittivity << ", "
			<< "permeability = " << insulator->permeability;
		if (insulator->a != 0 || insulator->b != 0) {
			std::cout << ", a = " << insulator->a << " m, b = " << insulator->b << " m";
		}
		std::cout << std::endl;
	}
	std::cout << "Positions of Cables:" << std::endl;
	for (size_t i = 0; i < positions.size(); ++i) {
		std::cout << "  Cable " << i + 1 << ": (x = " << positions[i].first << " m, y = " << positions[i].second << " m)" << std::endl;
	}
}
