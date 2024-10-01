// element.cpp

# include "Element.h"
#include "Constants.h"
#include <iostream>
#include <vector>


using namespace SymEngine;

// Destructor
Element::~Element() {}

// Attach terminal to the bus
void Element::attachBus(Bus* bus, int terminal) {
    connections[bus] = terminal;
}

// Getters
std::vector<Bus*> Element::getBuses() {
    std::vector<Bus*> buses;
    for (std::map<Bus*, int>::iterator it = connections.begin(); it != connections.end(); ++it) {
        buses.push_back(it->first);
    }
    return buses;
}

Bus* Element::getOtherBus(Bus* bus) {
    for (std::map<Bus*, int>::iterator it = connections.begin(); it != connections.end(); ++it) {
        if (bus != it->first)
            return it->first;
    }
}

// Implementation of compute_y_parameters for single-phase (default implementation)
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
            //std::cout << Y_val_exact[i][j] << endl;
        }
    }
    return Y_val_exact;
}

void Element::printElementValues() {
    std::cout << "Element : " << getElementSymbol() << std::endl;
    std::cout << "Y matrix symbolic entries: " << endl; 
    for (int i = 0; i < Y_matrix.nrows(); i++) {
        for (int j = 0; j < Y_matrix.ncols(); j++) {
            std::cout << Y_matrix.get(i, j)->__str__() << " "; // Use __str__() for output
        }
        std::cout << std::endl;
    }
}

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
                myfile << Y[i][j] << ",";
            }
        }
        myfile << "\n";

        frequency *= gap; // increase frequency
    }
    
    myfile.close();
}