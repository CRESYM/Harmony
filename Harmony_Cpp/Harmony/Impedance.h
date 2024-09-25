#ifndef _Impedance_h_

#define _Impedance_h_

#include "Element.h"
#include <vector> // For using vectors


class Impedance : public Element {
public:
    // Single-phase
    //double R = 5.0;  // Default resistance value
    //double X = 10; // Default reactance value
    
    // 3x3 matrix for representing the impedance between three phases
    std::vector<std::vector<double>> Z_matrix;

    // Constructor for Single-phase Impedance
    //Impedance(const std::string& symbol, int inputPins, int outputPins)
    //    : Element(symbol, inputPins, outputPins) {} 

    // Constructor for Three-phase Impedance
    Impedance(const std::string& symbol, int inputPins, int outputPins,
        const std::vector<std::vector<double>>& impedance_matrix)
        : Element(symbol, inputPins, outputPins), Z_matrix(impedance_matrix) {}

    ~Impedance() {}

    void compute_y_parameters(double frequency) override; // Declaration
private:
    // Helper function to calculate the Y-parameters for a 3-phase system
    //std::vector<std::vector<double>> calculate_y_parameters(double frequency);
};

#endif


