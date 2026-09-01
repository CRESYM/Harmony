/**
 * @file Element.cpp
 * @brief Implementation of Abstract base class for all electrical network components.
 */
#include "Element.h"
#include "network/Bus.h"
#include "ui/Visualization.h"

#include <stdexcept>


/**
 * @brief Destructor for the Element class.
 */
Element::~Element() {}

bool Element::isAcLocation() const {
	if (element_location.size() < 2) return false;
	const char a = element_location[0], c = element_location[1];
	return (a == 'A' || a == 'a') && (c == 'C' || c == 'c');
}

bool Element::isDcLocation() const {
	if (element_location.size() < 2) return false;
	const char d = element_location[0], c = element_location[1];
	return (d == 'D' || d == 'd') && (c == 'C' || c == 'c');
}

bool Element::isMmcLocation() const {
	return element_location.find('_') < element_location.length();
}

void Element::fillOpfBranchFromY(std::map<std::string, double>& branchData,
	std::map<std::string, double>& globalParams,
	const std::vector<std::vector<std::complex<double>>>& Y) const
{
	const int n = static_cast<int>(Y.size());
	if (n < 2 || (n % 2) != 0 || Y[0].size() != static_cast<size_t>(n))
		throw std::runtime_error("[OPF] " + element_symbol + " Y-matrix is not a square two-port.");
	const int p = n / 2;

	const std::complex<double> Y11 = Y[0][0];
	const std::complex<double> Y12 = Y[0][static_cast<size_t>(p)];
	if (std::abs(Y12) < 1e-18)
		throw std::runtime_error("[OPF] " + element_symbol + " series admittance Y12 is zero.");

	const bool dc = isDcLocation();
	const double Zbase = dc ? globalParams.at("DCZbase") : globalParams.at("ACZbase");
	const std::complex<double> Zs = -std::complex<double>(1.0, 0.0) / Y12 / Zbase;
	const std::complex<double> Yend = Y11 + Y12;

	branchData["r"] = std::real(Zs);
	branchData["x"] = dc ? 0.0 : std::imag(Zs);
	branchData["b"] = dc ? 0.0 : 2.0 * std::imag(Yend) * Zbase;
	if (!dc) {
		branchData["transformer"] = 0;
		branchData["tap"] = 1.0;
		branchData["shift"] = 0.0;
		branchData["c_rating_a"] = 1.0;
		branchData["g_fr"] = std::real(Yend);
		branchData["b_fr"] = std::imag(Yend);
		branchData["g_to"] = std::real(Yend);
		branchData["b_to"] = std::imag(Yend);
	}
	if (element_location.size() >= 3)
		branchData["grid"] = static_cast<int>(element_location[2] - '0');
	for (auto& [key, value] : element_OPF_info)
		branchData[key] = value;
}

double Element::finiteOmega(double omega) {
	const double min_w = 1e-6;
	if (std::abs(omega) < min_w)
		return (omega < 0.0) ? -min_w : min_w;
	return omega;
}

// Bipolar loop two-port. Phase order is [end1 core0, end1 core1, end2 core0, end2 core1].
// V = V+ − V−, I = (I+ − I−)/2 with V+ = V/2, V− = −V/2:
//   T = [1/2, −1/2]^T at each end,  Y_eq = blkdiag(T,T)^T  Y  blkdiag(T,T).
std::vector<std::vector<complex<double>>> Element::reduceDcY(
	const std::vector<std::vector<complex<double>>>& Y) const {
	if (Y.size() == 2 && !Y[0].empty() && Y[0].size() == 2)
		return Y;
	if (Y.size() < 4 || Y[0].size() < 4 || (Y.size() % 2) != 0 || Y.size() != Y[0].size())
		return Y;
	const int n = static_cast<int>(Y.size()) / 2;
	auto mix = [&Y](int i0, int i1, int j0, int j1) {
		return (Y[i0][j0] + Y[i1][j1] - Y[i0][j1] - Y[i1][j0]) / 4.0;
	};
	std::vector<std::vector<complex<double>>> Ydc(2, std::vector<complex<double>>(2));
	Ydc[0][0] = mix(0, 1, 0, 1);
	Ydc[0][1] = mix(0, 1, n, n + 1);
	Ydc[1][0] = mix(n, n + 1, 0, 1);
	Ydc[1][1] = mix(n, n + 1, n, n + 1);
	return Ydc;
}

/**
 * @brief Attaches a bus to a specific terminal of the element.
 * @param bus Pointer to the Bus object to attach.
 * @param terminal The terminal number to which the bus is connected.
 */
void Element::attachBus(Bus* bus, int terminal) {
    connections[bus] = terminal;
}

/**
 * @brief Retrieves all buses connected to this element.
 * @return A vector of pointers to the connected Bus objects.
 */
std::vector<Bus*> Element::getBuses() {
    std::vector<Bus*> buses;
    for (std::map<Bus*, int>::iterator it = connections.begin(); it != connections.end(); ++it) {
        buses.push_back(it->first);
    }
    return buses;
}

/**
 * @brief Gets the other bus connected to the element, assuming it's a two-terminal element.
 * @param bus A pointer to one of the connected buses.
 * @return A pointer to the other connected bus, or nullptr if no other bus is found.
 */
Bus* Element::getOtherBus(Bus* bus) {
    for (std::map<Bus*, int>::iterator it = connections.begin(); it != connections.end(); ++it) {
        if (bus != it->first)
            return it->first;
    }
    return nullptr; // No other bus found
}

/**
 * @brief Computes the numerical Y-parameter matrix at a given frequency.
 * @param frequency The frequency in Hz for which to compute the Y-parameters.
 * @return A 2D vector of complex numbers representing the Y-parameter matrix.
 */
std::vector<std::vector<complex<double>>> Element::compute_y_parameters(double frequency) {
    double angular_frequency = 2 * frequency * M_PI;
    map_basic_basic m;
    m[omega] = real_double(angular_frequency);
	double omega_0 = 100.0 * M_PI; // Default frequency 50 Hz
	map_basic_basic m1, m2;
	m1[omega] = real_double(angular_frequency - omega_0);
	m2[omega] = real_double(angular_frequency + omega_0);

    bool is_ac = isAcLocation();
	bool is_dc = isDcLocation();
    bool is_mmc = isMmcLocation();

    if (transformation && is_ac && !is_mmc) {
        std::vector<std::vector<complex<double>>> Y_val_exact1(Y_matrix.nrows());
        std::vector<std::vector<complex<double>>> Y_val_exact2(Y_matrix.nrows());
        for (int i = 0; i < Y_matrix.nrows(); i++) {
            Y_val_exact1[i].resize(Y_matrix.ncols());
            Y_val_exact2[i].resize(Y_matrix.ncols());
        }
        for (int i = 0; i < Y_matrix.nrows(); ++i) {
            for (int j = 0; j < Y_matrix.ncols(); ++j) {
                RCP<const Basic> r = subs(Y_matrix.get(i, j), m1);
                Y_val_exact1[i][j] = eval_complex_double(*r);
				r = subs(Y_matrix.get(i, j), m2);
				Y_val_exact2[i][j] = eval_complex_double(*r);
            }
        }
		//cout << "Applying transformation to element: " << element_symbol << endl;
		vector<vector<complex<double>>> Y = apply_transformation(Y_val_exact1, Y_val_exact2);
		return Y;
    }
    else if (is_dc && transformation) {
        std::vector<std::vector<complex<double>>> Y_val_exact(Y_matrix.nrows());
        for (int i = 0; i < Y_matrix.nrows(); i++)
            Y_val_exact[i].resize(Y_matrix.ncols());
        for (int i = 0; i < Y_matrix.nrows(); ++i) {
            for (int j = 0; j < Y_matrix.ncols(); ++j) {
                RCP<const Basic> r = subs(Y_matrix.get(i, j), m);
                Y_val_exact[i][j] = eval_complex_double(*r);
            }
        }
        return reduceDcY(Y_val_exact);
    }
    else {
        std::vector<std::vector<complex<double>>> Y_val_exact(Y_matrix.nrows());
        for (int i = 0; i < Y_matrix.nrows(); i++)
            Y_val_exact[i].resize(Y_matrix.ncols());
        for (int i = 0; i < Y_matrix.nrows(); ++i) {
            for (int j = 0; j < Y_matrix.ncols(); ++j) {
                RCP<const Basic> r = subs(Y_matrix.get(i, j), m);
                Y_val_exact[i][j] = eval_complex_double(*r);
            }
        }
        return Y_val_exact;
    }
}

std::vector<std::vector<complex<double>>> Element::compute_y_parameters_abc(double frequency) {
    const bool saved = transformation;
    transformation = false;
    auto Y = compute_y_parameters(frequency);
    transformation = saved;
    return Y;
}

/**
 * @brief Applies a transformation (e.g., abc to dq) to the admittance matrices.
 * @param Y1 The admittance matrix computed at angular frequency (omega - omega_0).
 * @param Y2 The admittance matrix computed at angular frequency (omega + omega_0).
 * @return The transformed 2D vector of complex numbers representing the Y-parameter matrix in the new frame.
 */
std::vector<std::vector<complex<double>>> Element::apply_transformation(std::vector<std::vector<complex<double>>>& Y1, std::vector<std::vector<complex<double>>>& Y2) {
    return apply_park_A0(Y1, Y2);
}

/**
 * @brief Prints the symbolic Y-parameter matrix of the element to the console.
 */
void Element::printElementValues() {
    std::cout << "Element : " << getElementSymbol() << std::endl;
    std::cout << "Y matrix symbolic entries: " << endl; 
    for (int i = 0; i < Y_matrix.nrows(); i++) {
        for (int j = 0; j < Y_matrix.ncols(); j++) {
            std::cout << simplify(Y_matrix.get(i, j))->__str__() << " "; 
        }
        std::cout << std::endl;
    }
}

/**
 * @brief Writes the Y-parameter matrix to a CSV file over a specified frequency range.
 * @param start_frequency The starting frequency for the sweep.
 * @param end_frequency The ending frequency for the sweep.
 * @param number_of_points The number of frequency points to compute and write.
 */
void Element::writeFile(double start_frequency, double end_frequency, int number_of_points) {
    std::ofstream myfile;
    myfile.open("./files/" + element_symbol + ".csv");

    // Print the Y-parameters in file
    // Use (N-1) so the last sample is exactly end_frequency (log-spaced).
    const int n = std::max(number_of_points, 2);
    const double gap = std::pow(10.0,
        (std::log10(end_frequency) - std::log10(start_frequency)) / (n - 1));
    double frequency = start_frequency;
    for (int p = 0; p < n; p++) {
        std::vector<std::vector<complex<double>>> Y = compute_y_parameters(frequency);
        
        // write in file
        myfile << frequency << ",";
		//cout << "Frequency: " << frequency << " Hz" << endl;
        for (int i = 0; i < Y_matrix.nrows(); ++i) {
            for (int j = 0; j < Y_matrix.ncols(); ++j) {
                myfile << Y[i][j].real() << "+1i*(" << Y[i][j].imag() << "),";
            }
        }
        myfile << "\n";

        frequency = frequency * gap; // increase frequency
    }
    
    myfile.close();
}

/**
 * @brief Generates data and triggers a Bode plot for the Y-parameter matrix.
 * @param start_frequency The starting frequency for the plot.
 * @param end_frequency The ending frequency for the plot.
 * @param number_of_points The number of points to plot across the frequency range.
 */
void Element::plotYParameters(double start_frequency, double end_frequency, int number_of_points) {
    const int n = std::max(number_of_points, 2);
    std::vector<double> frequencies;
    std::vector<std::vector<double>> magnitudes(n, std::vector<double>(pow(input_pins + output_pins, 2), 0.0));
    std::vector<std::vector<double>> phases(n, std::vector<double>(pow(input_pins + output_pins, 2), 0.0));
    std::vector<std::string> labels;
    const double gap = std::pow(10.0,
        (std::log10(end_frequency) - std::log10(start_frequency)) / (n - 1));
    double frequency = start_frequency;
    for (int p = 0; p < n; p++) {
        frequencies.push_back(frequency);
        std::vector<std::vector<complex<double>>> Y = compute_y_parameters(frequency);

        for (int i = 0; i < Y_matrix.nrows(); ++i) {
            for (int j = 0; j < Y_matrix.ncols(); ++j) {
                double magnitude = 20 * log10(std::abs(Y[i][j]));
                double phase = std::arg(Y[i][j]) * 180.0 / M_PI; // Convert to degrees

                magnitudes[p][Y_matrix.ncols() * i + j] = magnitude;
                phases[p][Y_matrix.ncols() * i + j] = phase;
            }
        }
        // cout << "Frequency: " << frequency << " Hz" << endl;
        frequency *= gap; // increase frequency
    }

    // Making labels
    for (int i = 0; i < Y_matrix.nrows(); ++i) {
        for (int j = 0; j < Y_matrix.ncols(); ++j) {
            labels.push_back("Y_{" + to_string(i+1) + to_string(j+1) + "}");
        }
    }

    bode_plot_implot(frequencies, magnitudes, phases, labels, "Y-Parameters of " + element_symbol);
}