#ifndef SWITCH_H
#define SWITCH_H

#include "Element.h"
#include <vector>

class Switch : public Element {
public:

    // Unified constructor for single-phase and three-phase systems
    Switch(const std::string& symbol,
        const std::vector<Bus*>& node1s,
        const std::vector<Bus*>& node2s,
        const std::vector<bool>& phaseState = { true });

    // override generic stamper
    void writeMNAmatrix(SymEngine::DenseMatrix& A,
        int num_equations,
        int index,
        const SymEngine::RCP<const SymEngine::Basic>& value,
        const std::unordered_map<Bus*, int>& busIndex) override;

    //Symbolic admittance matrix stamping 
    //void writeMatrixSymbolic(SymEngine::DenseMatrix& Y,
    //    const std::unordered_map<Bus*, int>& busIndex);

    // getters
    bool getState(int phase = 0) const { return phaseState[phase]; }

    void printElementValues() override;

private:
    std::vector<bool> phaseState;           // true = closed, false = open
};

#endif // SWITCH_H

