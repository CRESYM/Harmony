#ifndef CAPACITOR_H
#define CAPACITOR_H

#include "../Element.h"

class Bus;

class Capacitor : public Element {
public:
    // Frequency-domain or more general symbolic constructor //for frequency-domain admittance or impedance
    Capacitor(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& capacitance);


    ~Capacitor();
    
    void writeMNAmatrix(SymEngine::DenseMatrix&, std::unordered_map<Bus*, int>&, int, std::map<Element*, std::vector<RCP<const Basic>>>&) override;

    void printElementValues() override;

private:
    std::vector<double> C; // Capacitance value in Farads
	double initial_value = 0; // Initial voltage across the capacitor, not used for now
};

#endif // CAPACITOR_H