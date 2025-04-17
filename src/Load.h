#ifndef LOAD
#define LOAD

#include "Element.h"

/*
Creates load with resistive, inductive and capacitive components in series. Its constructor 
gets information about pin number and furthermore, R, L, C values. These values can be given 
as single value per each component R, L, C, and thus, input 3 values as vector. Or there can be
a separate value per each component R, L, C per pin/phase and thus, it gets 3 * pins input 
values.
*/
class Load : public Element {
public:
    /*
    * Constructor: Load
    *
    * Constructs a Load element with the given symbol (name), number of pins (phases), and values for
    * resistance (R), inductance (L), and capacitance (C). The values vector can contain either 3 values
    * for uniform R, L, C across phases, or 3 * pins values for phase-specific R, L, and C.
    *
    * Parameters:
    * - symbol: Symbolic identifier for the load element (e.g., Load1, Load2)
    * - pins: Number of input/output pins (phases)
    * - values: Vector of values containing either 3 entries or 3 * pins entries for R, L, and C.
    */
    Load(const std::string& symbol, int pins, std::vector<double> values);

    // Destructor
    ~Load() {}
    
    // Access functions for resistance, inductance, and capacitance values for each phase
    double getResistance(int phase) const {
        if (phase >= 0 && phase < R.size()) {
            return R[phase];
        }
        throw std::out_of_range("Invalid phase index");
    }

    double getInductance(int phase) const {
        if (phase >= 0 && phase < L.size()) {
            return L[phase];
        }
        throw std::out_of_range("Invalid phase index");
    }

    double getCapacitance(int phase) const {
        if (phase >= 0 && phase < C.size()) {
            return C[phase];
        }
        throw std::out_of_range("Invalid phase index");
    }

    // Power flow calculations (AC and DC)
    void computePowerFlowAC(std::map<std::string, std::map<std::string, double>>& branchData,
        std::map<std::string, double>& globalParams) const override;

    void computePowerFlowDC(std::map<std::string, std::map<std::string, double>>& branchDCData,
        std::map<std::string, double>& globalParams) const override;

private:
    // Values for resistance, inductance and capacitance
    std::vector<double> R;  // Resistance for each phase
    std::vector<double> L;  // Inductance for each phase
    std::vector<double> C;  // Capacitance for each phase

};
#endif
