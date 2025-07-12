#ifndef _RESISTOR_H_
#define _RESISTOR_H_

#include "Element.h"

class Resistor : public Element {
public:
    // Unified constructor supports both single-phase and multi-phase
    Resistor(const std::string& symbol, int pins, const std::vector<double>& R);

    ~Resistor();
    
    // Override writeMNAmatrix for resistor stamping(nodal admittance)
    void writeMNAmatrix(SymEngine::DenseMatrix&, std::unordered_map<Bus*, int>&, int, std::map<Element*, std::vector<RCP<const Basic>>>&) override;
   
        
    // Override to print resistor-specific data
    void printElementValues() override;

private:
    std::vector<double> R_values;
};

#endif // _RESISTOR_H_
