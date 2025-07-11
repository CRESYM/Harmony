#ifndef AC_SOURCE_H
#define AC_SOURCE_H

#include "Element.h"

/*
AC source is modeled as ideal AC voltage source with added series impedance. Its parameters are
AC are series impedance value. It also can be created as single, three phase, etc. and thus, 
pins number is added also as its input.
Additionally, after initial creation needs to be added power flow data:
voltage magnitude and phase shift, active and reactive powers, and their max and min values.
*/
class AC_source : public Element {
public:
    // Constructor
    AC_source(const std::string& symbol, int pins, DenseMatrix Z);

    // Destructor
    ~AC_source();

    //void addPowerFlowData();

    //void writeMatrixSymbolic(SymEngine::DenseMatrix& mat,
    //    const std::unordered_map<Bus*, int>& busIndex) override;

    void writeMNAmatrixNumeric(
        Eigen::MatrixXd& A, Eigen::MatrixXd& E, Eigen::MatrixXd& B,
        int num_equations,
        int index,
        const std::unordered_map<Bus*, int>& busIndex,
        const std::unordered_map<Element*, int>& currentSourceIndex,
        const std::unordered_map<Element*, int>& stateVarIndex) override;

    // Function to print AC source values
    void printElementValues();

private:
    // Properties
    DenseMatrix Z; // Source series impedance [Omega]
    double V;      // Voltage amplitude [kV]
    double theta;  // Phase shift [radians]

    // Properties used for power flow
    double P;      // Active power output [MW]
    double Q;      // Reactive power output [MVAr]
    double P_min;  // Min active power output [MW]
    double P_max;  // Max active power output [MW]
    double Q_min;  // Min reactive power output [MVA]
    double Q_max;  // Max reactive power output [MVA]

    std::vector<std::complex<double>> Y; // Y parameters for the source
};

#endif

