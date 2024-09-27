#ifndef LOAD
#define LOAD

#include "Element.h"
#include <vector>
#include <stdexcept>


/*
Creates load with resistive, inductive and capacitive components in series. Its constructor 
gets information about pin number and furthermore, R, L, C values. These values can be given 
as single value per each component R, L, C, and thus, input 3 values as vector. Or there can be
a separate value per each component R, L, C per pin/phase and thus, it gets 3 * pins input 
values.
*/

class Load : public Element {
public:
    //Single-phase
    //Load(const std::string& symbol, int inputPins, int outputPins)
        //: Element(symbol, inputPins, outputPins) {}

    Load(const std::string& symbol, int pins, std::vector<double> values);

    // Unified constructor for both single-phase and three-phase systems
    /*Load(const std::string& symbol, int pins, const std::vector<double>& values)
        : Element(symbol, pins, pins), R(pins, 0.0), L(pins, 0.0), C(pins, 0.0) {

        if (values.size() == 3) {
            // Single-phase case: One value each for R, L, and C
            for (int i = 0; i < pins; ++i) {
                R[i] = values[0]; // Assign the same R value for each phase/pin
                L[i] = values[1]; // Assign the same L value for each phase/pin
                C[i] = values[2]; // Assign the same C value for each phase/pin
            }
        }
        else if (values.size() == 3 * pins) {
            // Three-phase case: Separate values for each phase
            for (int i = 0; i < pins; ++i) {
                R[i] = values[i];            // R values for each pin
                L[i] = values[pins + i];      // L values for each pin
                C[i] = values[2 * pins + i];  // C values for each pin
            }
        }
        else {
            throw std::invalid_argument("Incorrect number of values provided. Use 3 values for single-phase or 3*pins values for three-phase systems.");
        }

        // Print information about the Load type
        if (pins == 1) {
            std::cout << "Single-phase Load created for element: " << symbol << std::endl;
        }
        else {
            std::cout << "Three-phase Load created for element: " << symbol << std::endl;
        }
    }*/

    ~Load() {}

    void compute_y_parameters(double frequency) override; // Declaration
    void printElementValues() override; // Declaration

    double getResistance(int phase) const {
        if (phase >= 0 && phase < R.size()) {
            return R[phase];
        }
        throw std::out_of_range("Invalid phase index");
    }

    double getInductance(int phase) const {
        if (phase >= 0 && phase < L.size()) {
            return L[phase];
        }
        throw std::out_of_range("Invalid phase index");
    }

    double getCapacitance(int phase) const {
        if (phase >= 0 && phase < C.size()) {
            return C[phase];
        }
        throw std::out_of_range("Invalid phase index");
    }
private:
    //Single-phase
    //double R = 10.0;
    //double L = 0.01;
    //double C = 0.001;

    //Three-phase
    std::vector<double> R;  // Resistance for each phase
    std::vector<double> L;  // Inductance for each phase
    std::vector<double> C;  // Capacitance for each phase
};
#endif