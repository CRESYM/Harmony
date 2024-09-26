#ifndef AC_SOURCE_H
#define AC_SOURCE_H

#include "Element.h"

/*
AC source is modeled as ideal AC voltage source with added series impedance. Its parameters are 
AC voltage magnitude and phase shift, and impedance value. It also can be created as single,
three phase, etc. and thus, pins number is added also as its input.
*/


class AC_source : public Element {
public:
    // Constructor
    AC_source(const std::string& symbol, int pins = 1, double Z = 0, double V = 0, 
        double theta = 0, double P = 0, double Q = 0,
        double P_min = 0, double P_max = 0, double Q_min = 0, double Q_max = 0)
        : Element(symbol, pins, pins), Z(Z), V(V), theta(theta), P(P), Q(Q), 
        P_min(P_min), P_max(P_max), Q_min(Q_min), Q_max(Q_max)
    {
        Y.resize(pins);
        // Initialize Y parameters
        double G = P / (V * V); // Conductance
        double B = Q / (V * V); // Susceptance

        // Create a diagonal Y matrix
        for (int i = 0; i < input_pins; ++i) {
            Y[i] = std::complex<double>(G, -B); // Y = G - jB
        }
    }

    // Function to create Y parameters
    void compute_y_parameters(double frequency) {
    }

    // Function to evaluate Y parameters (could be expanded for numerical evaluations)
    std::vector<std::complex<double>> eval_y() const {
        return Y;
    }

    void printElementValues();

private:
    // Properties
    double Z;      // Source series impedance [Omega]
    double V;      // Voltage amplitude [kV]
    double theta;  // Phase shift [radians]

    // Properties used for power flow
    double P;      // Active power output [MW]
    double Q;      // Reactive power output [MVAr]
    double P_min;  // Min active power output [MW]
    double P_max;  // Max active power output [MW]
    double Q_min;  // Min reactive power output [MVA]
    double Q_max;  // Max reactive power output [MVA]

    std::vector<std::complex<double>> Y; // Y parameters for the source
};

#endif
