#ifndef GENERATOR_H
#define GENERATOR_H

#include "Element.h"
#include <array>

class Generator : public Element {
public:
    // Constructor
    Generator(const std::string& symbol, int pins, const std::vector<double>& values);

    // Destructor
    ~Generator() {}

    //// Set voltage for each phase OPF
    //void set_voltage(int phase, std::complex<double> voltage) {
    //    if (phase >= 0 && phase < voltage_values.size()) {
    //        voltage_values[phase] = voltage;
    //    }
    //    else {
    //        throw std::out_of_range("Invalid phase index");
    //    }
    //}

    //// Compute the power flow for the generator given the voltage on each phase
    //std::complex<double> compute_power_flow(int phase);
    //
    //// Compute the impedance for the generator based on given parameters
    //std::complex<double> get_impedance(int phase) const;

    // Power flow computations for AC and DC networks
    void computePowerFlowAC(std::map<std::string, std::map<std::string, double>>& branchData,
        std::map<std::string, double>& globalParams) const override;

    void computePowerFlowDC(std::map<std::string, std::map<std::string, double>>& branchDCData,
        std::map<std::string, double>& globalParams) const override;

private:
    double R_f = 1.0;  // Default value
    double L_f = 0.01; // Default value
    double X_d = 1.0;  // Default value
    double T_f = 0.1;  // Default value

    //std::vector<std::complex<double>> voltage_values;  // Voltages for each phase

};

#endif

