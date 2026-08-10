#ifndef _LOAD_H_
#define _LOAD_H_

/**
 * @file Load.h
 * @brief Series R-L-C load model for AC and DC networks.
 */

#include "Load_base.h"

/**
 * @class Load
 * @brief Load with resistive, inductive, and capacitive components in series.
 * @ingroup source
 *
 * R, L, and C may be specified as three uniform values or as 3 × pins phase-specific values.
 */
class Load : public Load_base {
public:
    /**
     * @brief Construct a series R-L-C load.
     * @param symbol Element identifier (e.g. Load1).
     * @param location Network area or location string.
     * @param pins Number of pins (phases).
     * @param values Vector of 3 or 3 × pins values for R, L, and C.
     */
    Load(const std::string& symbol, const std::string& location, int pins, std::vector<double> values);


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
    void computePowerFlow(std::map<std::string, double>& busAC,
		std::map<std::string, double>& globalParams) const override;

private:
    // Values for resistance, inductance and capacitance
    std::vector<double> R;  // Resistance for each phase
    std::vector<double> L;  // Inductance for each phase
    std::vector<double> C;  // Capacitance for each phase

	// element_OPF_info is containing values for element_symbol, grid_area, rated_voltage_kv, R, L, C
};
#endif
