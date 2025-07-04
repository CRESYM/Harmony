#ifndef CAPACITOR_H
#define CAPACITOR_H

#include "Element.h"
#include <symengine/basic.h>
class Bus;

class Capacitor : public Element {
public:
    // Frequency-domain or more general symbolic constructor //for frequency-domain admittance or impedance
    Capacitor(const std::string& symbol, int inputPins, int outputPins, double capacitance, double frequency = 0.0);

     //Time‑domain unified constructor single OR multi-phase
    Capacitor(const std::string& symbol,
        const std::vector<Bus*>& node1s,
        const std::vector<Bus*>& node2s,
        double capacitance,
        const std::vector<double>& initialVoltage = { 0.0 });

    ~Capacitor();
    
    void writeMNAmatrix(SymEngine::DenseMatrix& A,
        int num_equations,
        int firstBranchIndex,
        const SymEngine::RCP<const SymEngine::Basic>& value,
        const std::unordered_map<Bus*, int>& busIndex) override;

    void printElementValues() override;

private:
    double C; // Capacitance value in Farads
    double initial_value; // Initial voltage across the capacitor
};

#endif // CAPACITOR_H