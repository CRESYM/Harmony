#ifndef SIMPLE_MMC_H
#define SIMPLE_MMC_H


#include "Converter.h"
#include "../../Solver/DQsym/DQsym.h"
#include "../../Include_control_blocks.h"

class Simple_MMC : public Converter
{
public:
    // Constructor
    Simple_MMC(const std::string& symbol, const std::string& location,
        double omega, double activePower, double reactivePower,
        double angle, double acVoltage, double Pdc, double dcVoltage,
        double armInductance, double armResistance, double armCapacitance,
        int numSubmodules, double reactorInductance, double reactorResistance);

    // Constructor from vector
    Simple_MMC(const std::string& symbol, const std::string& location,
        const std::vector<double>& converter_params);

    void computeABCD() override;

    // One MMC arm-voltage time step
    vector<MatrixXcd> simulateTimeStep(const vector<MatrixXcd>& input, double Ts, int nKeep1, int nKeep2) override;

  
private:
    int number_of_states = 6;

    double L_arm = 0.0;
    double R_arm = 0.0;
    double C_arm = 0.0;
    int N = 0;


    MatrixXcd ZupOld_;
    MatrixXcd XupOld_;
    MatrixXcd ZlowOld_;
    MatrixXcd XlowOld_;
};

#endif // SIMPLE_MMC_H
