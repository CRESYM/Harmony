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
    connections[terminal] = bus;
}

// Getters
std::vector<Bus*> Element::getBuses() {
    std::vector<Bus*> buses;
    for (std::map<int, Bus*>::iterator it = connections.begin(); it != connections.end(); ++it) {
        buses.push_back(it->second);
    }
    return buses;
}

Bus* Element::getOtherBus(Bus* bus) {
    for (std::map<int, Bus*>::iterator it = connections.begin(); it != connections.end(); ++it) {
        if (bus != it->second)
            return it->second;
    }
}

// Implementation of compute_y_parameters for single-phase (default implementation)
std::vector<std::vector<complex<double>>> Element::compute_y_parameters(double frequency) {
    double angular_frequency = 2 * frequency * M_PI;
    map_basic_basic m;
    m[omega] = real_double(angular_frequency);
    std::vector<std::vector<complex<double>>> Y_val_exact(input_pins);
    for (int i = 0; i < input_pins; i++)
        Y_val_exact[i].resize(output_pins);
    for (int i = 0; i < input_pins; ++i) {
        for (int j = 0; j < output_pins; ++j) {
            RCP<const Basic> r = subs(Y_matrix.get(i, j), m);
            Y_val_exact[i][j] = eval_complex_double(*r); // eval_double(Y_matrix.get(i, j));
            std::cout << Y_val_exact[i][j] << endl;
        }
    }
    return Y_val_exact;
}

void Element::printElementValues() {}

void Element::writeFile() {
    std::ofstream myfile;
    myfile.open("files/"+element_symbol);

    // Print the Y-parameters in file

    for (int i = 0; i < input_pins; ++i) {
        for (int j = 0; j < output_pins; ++j) {
            double Y_val_abs = eval_double(*abs(Y_matrix.get(i, j)));
            std::cout << "| Y" << (i + 1) << (j + 1) << "|: " << Y_val_abs << " S" << std::endl;
        }
    }
    myfile << "1,2,3.456\n";
    myfile.close();
}