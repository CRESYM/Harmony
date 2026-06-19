/**
 * @file Element.cpp
 * @brief Implementation of Abstract base class for all electrical network components.
 */
#include "Element.h"
#include "../Bus.h"

// Helper functions
#include "../Solver/Helper_Functions/Helper_Functions.h"


/**
 * @brief Destructor for the Element class.
 */
Element::~Element() {}

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

    bool is_ac = (element_location[0] == 'A' || element_location[0] == 'a') && (element_location[1] == 'C' || element_location[1] == 'c');
	bool is_dc = (element_location[0] == 'D' || element_location[0] == 'd') && (element_location[1] == 'C' || element_location[1] == 'c');
    bool is_mmc = (element_location.find('_') < element_location.length());

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
        std::vector<std::vector<complex<double>>> Y_val_exact(2);
        for (int i = 0; i < 2; i++)
            Y_val_exact[i].resize(2);

        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                RCP<const Basic> r = subs(Y_matrix.get(2*i, 2*j), m);
                Y_val_exact[i][j] = eval_complex_double(*r);
                //cout << "Computing Y[" << i << "][" << j << "] for DC element with transformation: " << element_symbol << " equal to: " << Y_val_exact[i][j] << endl;
            }
        }
        return Y_val_exact;
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

/**
 * @brief Applies a transformation (e.g., abc to dq) to the admittance matrices.
 * @param Y1 The admittance matrix computed at angular frequency (omega - omega_0).
 * @param Y2 The admittance matrix computed at angular frequency (omega + omega_0).
 * @return The transformed 2D vector of complex numbers representing the Y-parameter matrix in the new frame.
 */
std::vector<std::vector<complex<double>>> Element::apply_transformation(std::vector<std::vector<complex<double>>>& Y1, std::vector<std::vector<complex<double>>>& Y2) {
    
    // The transformation is applied to a 6x6 matrix, so we expect Y1 and Y2 to be of that size.
    if (Y1.size() != 6 || Y1[0].size() != 6 || Y2.size() != 6 || Y2[0].size() != 6) {
        // Return Y1 if dimensions are not as expected, or handle error appropriately
        return Y1;
    }

    // Transformation matrix for abc to dq0 (Clarke-Park)
    complex<double> ang = std::exp(complex<double>(0, 2.0 * M_PI / 3.0));
    complex<double> imag_unit(0, 1);

    // Using vector<vector> for transformation matrices
    vector<vector<complex<double>>> a(3, vector<complex<double>>(3));   
    a[0] = { 1.0, ang, ang * ang };
    a[1] = { imag_unit, imag_unit*ang, imag_unit*ang * ang };
    a[2] = { 0.0, 0.0, 0.0 };
	vector<vector<complex<double>>> a_tran = mat_transpose(a);

    vector<vector<complex<double>>> a_conj(3, vector<complex<double>>(3));
    a_conj[0] = { 1.0, conj(ang), conj(ang * ang) };
    a_conj[1] = { -imag_unit, -imag_unit * conj(ang), -imag_unit * conj(ang * ang) };
    a_conj[2] = { 0.0, 0.0, 0.0 };
	vector<vector<complex<double>>> a_conj_tran = mat_transpose(a_conj);
    
	// Admittance at angular frequency omega - omega_0
    auto Y11 = get_block(Y1, 0, 0, 3, 3);
    auto Y12 = get_block(Y1, 0, 3, 3, 3);
    auto Y21 = get_block(Y1, 3, 0, 3, 3);
    auto Y22 = get_block(Y1, 3, 3, 3, 3);

	// Admittance at angular frequency omega + omega_0
    auto Y2_11 = get_block(Y2, 0, 0, 3, 3);
    auto Y2_12 = get_block(Y2, 0, 3, 3, 3);
    auto Y2_21 = get_block(Y2, 3, 0, 3, 3);
    auto Y2_22 = get_block(Y2, 3, 3, 3, 3);

    // Perform transformation: Y_dq = T_inv * Y_abc * T
    auto Y11_dq = mul_scalar(mat_add(mat_mul(mat_mul(a, Y11), a_conj_tran), mat_mul(mat_mul(a_conj, Y2_11), a_tran)), 1.0 / 6.0);
	auto Y12_dq = mul_scalar(mat_add(mat_mul(mat_mul(a, Y12), a_conj_tran), mat_mul(mat_mul(a_conj, Y2_12), a_tran)), 1.0 / 6.0);
	auto Y21_dq = mul_scalar(mat_add(mat_mul(mat_mul(a, Y21), a_conj_tran), mat_mul(mat_mul(a_conj, Y2_21), a_tran)), 1.0 / 6.0);
	auto Y22_dq = mul_scalar(mat_add(mat_mul(mat_mul(a, Y22), a_conj_tran), mat_mul(mat_mul(a_conj, Y2_22), a_tran)), 1.0 / 6.0);

    // Combine transformed blocks into a single matrix
    vector<vector<complex<double>>> Y_dq(4, vector<complex<double>>(4));
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            Y_dq[i][j] = Y11_dq[i][j];
            Y_dq[i][j + 2] = Y12_dq[i][j];
            Y_dq[i + 2][j] = Y21_dq[i][j];
            Y_dq[i + 2][j + 2] = Y22_dq[i][j];
        }
    }
    return Y_dq;
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
    double gap = (log10(end_frequency) - log10(start_frequency)) * 1.0 / (number_of_points);
	gap = pow(10, gap);
    double frequency = start_frequency;
    for (int p = 0; p < number_of_points; p++) {
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
    std::vector<double> frequencies;
    std::vector<std::vector<double>> magnitudes(number_of_points, std::vector<double>(pow(input_pins + output_pins, 2), 0.0));
    std::vector<std::vector<double>> phases(number_of_points, std::vector<double>(pow(input_pins + output_pins, 2), 0.0));
    std::vector<std::string> labels;
    double gap = (log10(end_frequency) - log10(start_frequency)) * 1.0 / (number_of_points);
	gap = pow(10, gap);
	// cout << gap << endl;
    double frequency = start_frequency;
    for (int p = 0; p < number_of_points; p++) {
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