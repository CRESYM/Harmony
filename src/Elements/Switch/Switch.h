#ifndef SWITCH_H
#define SWITCH_H

/**
 * @file Switch.h
 * @brief Multi-phase switch element for open/closed branch control.
 */

#include "../Element.h"

/**
 * @class Switch
 * @brief Single- or multi-phase switch with per-phase open/closed state.
 * @ingroup elements
 */
class Switch : public Element {
public:

    /**
     * @brief Construct a switch with an initial per-phase state vector.
     * @param symbol Element identifier.
     * @param location Network area or location string.
     * @param pins Number of pins (phases).
     * @param state Per-phase state (true = closed, false = open).
     */
    Switch(const std::string& symbol, const std::string& location, int pins, const std::vector<bool>& state);

    // override generic stamper
    void writeMNAmatrix(SymEngine::DenseMatrix&, std::unordered_map<Bus*, int>&, int, std::map<Element*, std::vector<RCP<const Basic>>>&) override;

	// change the state of the switch
    void setOpen() {
        phaseState.assign(phaseState.size(), false); // Set all phases to open
	}
    void setClosed() {
        phaseState.assign(phaseState.size(), true); // Set all phases to closed
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

