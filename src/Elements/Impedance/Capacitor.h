#ifndef CAPACITOR_H
#define CAPACITOR_H

/**
 * @file Capacitor.h
 * @brief Capacitor element with frequency-domain admittance Y = sC.
 */

#include "../Element.h"

class Bus;

/**
 * @class Capacitor
 * @brief Models a single- or multi-phase capacitive branch.
 * @ingroup impedance
 */
class Capacitor : public Element {
public:
    /**
     * @brief Construct a capacitor with per-phase or uniform capacitance values.
     * @param symbol Element identifier.
     * @param location Network area or location string.
     * @param pins Number of pins (phases).
     * @param capacitance Capacitance values in farads (one value or one per pin).
     */
    Capacitor(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& capacitance);


    ~Capacitor();
    
    void writeMNAmatrix(SymEngine::DenseMatrix&, std::unordered_map<Bus*, int>&, int, std::map<Element*, std::vector<RCP<const Basic>>>&) override;

    void printElementValues() override;

private:
    std::vector<double> C; // Capacitance value in Farads
	double initial_value = 0; // Initial voltage across the capacitor, not used for now
};

#endif // CAPACITOR_H