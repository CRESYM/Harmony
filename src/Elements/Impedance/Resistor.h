#ifndef _RESISTOR_H_
#define _RESISTOR_H_

/**
 * @file Resistor.h
 * @brief Resistor element for nodal admittance (MNA) stamping.
 */

#include "../Element.h"

/**
 * @class Resistor
 * @brief Models a single- or multi-phase resistive branch.
 * @ingroup impedance
 */
class Resistor : public Element {
public:
    /**
     * @brief Construct a resistor with per-phase or uniform resistance values.
     * @param symbol Element identifier.
     * @param location Network area or location string.
     * @param pins Number of pins (phases).
     * @param R Resistance values in ohms (one value or one per pin).
     */
    Resistor(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& R);

    ~Resistor();
    
    // Override writeMNAmatrix for resistor stamping(nodal admittance)
    void writeMNAmatrix(SymEngine::DenseMatrix&, std::unordered_map<Bus*, int>&, int, std::map<Element*, std::vector<RCP<const Basic>>>&) override;
   
        
    // Override to print resistor-specific data
    void printElementValues() override;

private:
    std::vector<double> R_values;
};

#endif // _RESISTOR_H_
