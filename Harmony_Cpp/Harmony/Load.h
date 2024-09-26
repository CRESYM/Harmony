#ifndef LOAD
#define LOAD

#include "Element.h"
#include <vector>

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

    ~Load() {}

    void compute_y_parameters(double frequency) override; // Declaration

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