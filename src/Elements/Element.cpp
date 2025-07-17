#include "Element.h"
#include "../Bus.h"

// Helper functions
#include "../Solver/Helper_Functions/Helper_Functions.h"


// Destructor for Element
Element::~Element() {}

// Attach terminal to the bus
void Element::attachBus(Bus* bus, int terminal) {
    connections[bus] = terminal;
}

// Getters to retrieve connected Buses
std::vector<Bus*> Element::getBuses() {
    std::vector<Bus*> buses;
    for (std::map<Bus*, int>::iterator it = connections.begin(); it != connections.end(); ++it) {
        buses.push_back(it->first);
    }
    return buses;
}

// Function to get the other Bus connected to the Element (not the one provided)
Bus* Element::getOtherBus(Bus* bus) {
    for (std::map<Bus*, int>::iterator it = connections.begin(); it != connections.end(); ++it) {
        if (bus != it->first)
            return it->first;
    }
    return nullptr; // No other bus found
}

// Implementation of compute_y_parameters 
std::vector<std::vector<complex<double>>> Element::compute_y_parameters(double frequency) {
    double angular_frequency = 2 * frequency * M_PI;
    map_basic_basic m;
    m[omega] = real_double(angular_frequency);
    std::vector<std::vector<complex<double>>> Y_val_exact(2 * input_pins);
    for (int i = 0; i < 2 * input_pins; i++)
        Y_val_exact[i].resize(2 * output_pins);
    for (int i = 0; i < 2 * input_pins; ++i) {
        for (int j = 0; j < 2 * output_pins; ++j) {
            RCP<const Basic> r = subs(Y_matrix.get(i, j), m);
            Y_val_exact[i][j] = eval_complex_double(*r); // eval_double(Y_matrix.get(i, j));
        }
    }
    return Y_val_exact;
}

// Function to print the Element's values and Y_matrix entries
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
// Function to write the Y-parameter matrix to a file over a frequency range
void Element::writeFile(double start_frequency, int end_frequency, int number_of_points) {
    std::ofstream myfile;
    myfile.open("files/" + element_symbol + ".csv");

    // Print the Y-parameters in file
    double gap = (end_frequency - start_frequency) / (number_of_points - 1);
    int frequency = start_frequency;
    for (int p = 0; p < number_of_points; p++) {
        std::vector<std::vector<complex<double>>> Y = compute_y_parameters(frequency);
        
        // write in file
        myfile << frequency << ",";
        for (int i = 0; i < 2 * input_pins; ++i) {
            for (int j = 0; j < 2 * output_pins; ++j) {
                myfile << Y[i][j].real() << "+1i*(" << Y[i][j].imag() << "),";
            }
        }
        myfile << "\n";

        frequency = frequency + gap; // increase frequency
    }
    
    myfile.close();
}

// Function to plot the Y-parameter matrix in logarithmic scale and in dB and angles
void Element::plotYParameters(double start_frequency, int end_frequency, int number_of_points) {
    std::vector<double> angular_frequencies;
    std::vector<std::vector<double>> magnitudes(number_of_points, std::vector<double>(pow(input_pins + output_pins, 2), 0.0));
    std::vector<std::vector<double>> phases(number_of_points, std::vector<double>(pow(input_pins + output_pins, 2), 0.0));
    std::vector<std::string> labels;
    double gap = (end_frequency - start_frequency) / (number_of_points - 1);
    double frequency = start_frequency;
    for (int p = 0; p < number_of_points; p++) {
        angular_frequencies.push_back(frequency);
        std::vector<std::vector<complex<double>>> Y = compute_y_parameters(frequency);
        
        for (int i = 0; i < 2 * input_pins; ++i) {
            for (int j = 0; j < 2 * output_pins; ++j) {
                double magnitude = 20 * log10(std::abs(Y[i][j]));
                double phase = std::arg(Y[i][j]) * 180.0 / M_PI; // Convert to degrees
                
                magnitudes[p][i+j] = magnitude;
                phases[p][i+j] = phase;
            }
        }
        frequency += gap; // increase frequency
    }

    // Making labels
    for (int i = 0; i < 2 * input_pins; ++i) {
        for (int j = 0; j < 2 * output_pins; ++j) {
            labels.push_back("Y_{" + to_string(i+1) + to_string(j+1) + "}");
        }
    }

    bode_plot(angular_frequencies, magnitudes, phases, labels, "Y-Parameters of " + element_symbol);
}