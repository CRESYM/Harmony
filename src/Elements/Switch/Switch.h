#ifndef SWITCH_H
#define SWITCH_H

#include "../Element.h"

class Switch : public Element {
public:

    // Unified constructor for single-phase and three-phase systems
    Switch(const std::string& symbol, const std::string& location, int pins, const std::vector<bool>& state);

    // override generic stamper
    void writeMNAmatrix(SymEngine::DenseMatrix&, std::unordered_map<Bus*, int>&, int, std::map<Element*, std::vector<RCP<const Basic>>>&) override;

	// change the state of the switch
    void setOpen() { 
        for (auto& state : phaseState) {
            state = false; // Set all phases to open
        }
	}
    void setClosed() {
        for (auto& state : phaseState) {
            state = true; // Set all phases to closed
        }
    }
    void setState(int phase, bool state) {
        if (phase >= 0 && phase < phaseState.size()) {
            phaseState[phase] = state; // Set specific phase state
        }
	}
    void setState(bool state) {
        for (int phase; phase < phaseState.size(); phase++) {
            phaseState[phase] = state; // Set specific phase state
        }
    }

    // getters
    bool getState(int phase = 0) const { return phaseState[phase]; }

    void printElementValues() override;

private:
    std::vector<bool> phaseState;           // true = closed, false = open
};

#endif // SWITCH_H

