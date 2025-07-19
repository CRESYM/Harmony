#ifndef GENERATOR_H
#define GENERATOR_H

#include "../Element.h"

class Generator : public Element {
public:
    // Constructor
    Generator(const std::string& symbol, int pins, const std::vector<double>& values);
 

    // Destructor
    ~Generator() {}

    // Power flow computations for AC and DC networks
    void computePowerFlowAC(std::map<std::string, double>& branchData,
        std::map<std::string, double>& globalParams) const override;

    void computePowerFlowDC(std::map<std::string, double>& branchDCData,
        std::map<std::string, double>& globalParams) const override;

private:
    double R_f = 1.0;  // Default value
    double L_f = 0.01; // Default value
    double X_d = 1.0;  // Default value
    double T_f = 0.1;  // Default value

};

#endif

