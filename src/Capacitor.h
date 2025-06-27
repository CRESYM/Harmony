#ifndef CAPACITOR_H
#define CAPACITOR_H

#include "Element.h"

class Bus;

class Capacitor : public Element {
public:
    // Frequency-domain or more general symbolic constructor //for frequency-domain admittance or impedance
    Capacitor(const std::string& symbol, int inputPins, int outputPins, double capacitance, double frequency = 0.0);

    // Time-domain constructor for MNA 
    Capacitor(const std::string& symbol, Bus* node1, Bus* node2, double capacitance, double initial = 0)
        : Element(symbol, 1, 1), C(capacitance), initial_value(initial)
    {
        attachBus(node1, 0); // Input terminal
        attachBus(node2, 1); // Output terminal
    }

    ~Capacitor();

    // Retrieve the initial voltage across the capacitor
    //double get_initial_value() const;
    
    void printElementValues() override;

private:
    double C; // Capacitance value in Farads
    double initial_value; // Initial voltage across the capacitor
};

#endif // CAPACITOR_H

