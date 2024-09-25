#ifndef LOAD
#define LOAD

#include "Element.h"
#include <vector>

class Load : public Element {
public:
    //Single-phase
    //Load(const std::string& symbol, int inputPins, int outputPins)
        //: Element(symbol, inputPins, outputPins) {}

    Load(const std::string& symbol, int inputPins, int outputPins)
        : Element(symbol, inputPins, outputPins), R(3, 10.0), L(3, 0.01), C(3, 0.001) {}

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