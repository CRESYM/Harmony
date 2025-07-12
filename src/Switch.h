#ifndef SWITCH_H
#define SWITCH_H

#include "Element.h"
#include <vector>

class Switch : public Element {
public:

    // Unified constructor for single-phase and three-phase systems
    Switch(const std::string& symbol, int pins, const std::vector<bool>& state);

    // override generic stamper
    void writeMNAmatrix(SymEngine::DenseMatrix&, std::unordered_map<Bus*, int>&, int, std::map<Element*, std::vector<RCP<const Basic>>>&) override;

    // getters
    bool getState(int phase = 0) const { return phaseState[phase]; }

    void printElementValues() override;

private:
    std::vector<bool> phaseState;           // true = closed, false = open
};

#endif // SWITCH_H

