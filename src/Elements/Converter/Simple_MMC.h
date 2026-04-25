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

    // State-space model manipulation - generic MNA stamping 
    void writeMNAmatrix(SymEngine::DenseMatrix&, std::unordered_map<Bus*, int>&, int, std::map<Element*, std::vector<RCP<const Basic>>>&) override;

    int getNumberOfInternalStates() const override { return number_of_states; }
    map_basic_basic getParameterSubstitutions() const override;
  
private:
    int number_of_states = 12;

    double L_arm = 0.0;
    double R_arm = 0.0;
    double C_arm = 0.0;
    int N = 0;

    // Helper values
	double L_eq = 0.0, R_eq = 0.0, m_1 = 1.0;

    vector<MatrixXcd> Zold;
    vector<MatrixXcd> Xold;
};

#endif // SIMPLE_MMC_H
