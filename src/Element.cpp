// element.cpp

#include "Element.h"
#include "Constants.h"
#include <iostream>
#include <vector>


using namespace SymEngine;

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

// Generic MNA stamping 
void Element::writeMNAmatrix(DenseMatrix& A,
    int num_equations,
    int index,
    const RCP<const Basic>& value,
    const std::unordered_map<Bus*, int>& busIndex)
{
    // Symbolic current variable like "vc1", "ir1", etc.
    RCP<const Basic> current_symbol = symbol("v" + element_symbol + std::to_string(index));

    int row = num_equations + index - 1;
    int col = num_equations;  // Last column for extra variable (e.g., branch voltage)

    A.set(row, col, current_symbol);  // Stamp current equation

    auto buses = getBuses();  // [0] = node1, [1] = node2

    if (buses[0] && busIndex.count(buses[0])) {
        int n1 = busIndex.at(buses[0]);
        A.set(n1, row, value);     // node1: KCL: +value
        A.set(row, n1, one);       // branch eq: voltage +
    }

    if (buses[1] && busIndex.count(buses[1])) {
        int n2 = busIndex.at(buses[1]);
        A.set(n2, row, mul(integer(-1), value)); // node2: KCL: -value
        A.set(row, n2, integer(-1));             // branch eq: voltage -
    }
}

// Function to print the Element's values and Y_matrix entries
void Element::printElementValues() {
    std::cout << "Element : " << getElementSymbol() << std::endl;
    std::cout << "Y matrix symbolic entries: " << endl; 
    for (int i = 0; i < Y_matrix.nrows(); i++) {
        for (int j = 0; j < Y_matrix.ncols(); j++) {
            std::cout << simplify(Y_matrix.get(i, j))->__str__() << " "; // Use __str__() for output
        }
        std::cout << std::endl;
    }
}
// Function to write the Y-parameter matrix to a file over a frequency range
void Element::writeFile(double start_frequency, int end_frequency, int number_of_points) {
    std::ofstream myfile;
    myfile.open("files/"+element_symbol+".csv");

    // Print the Y-parameters in file
    double gap = (log10(end_frequency) - log10(start_frequency)) / (number_of_points - 1);
    gap = pow(10, gap);
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

        frequency *= gap; // increase frequency
    }
    
    myfile.close();
}