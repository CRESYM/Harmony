#ifndef SIMPLE_MMC_H
#define SIMPLE_MMC_H


#include "Converter.h"
#include "../../Solver/DQsym/DQsym.h"
#include "../../Include_control_blocks.h"

//add on the 21/04
class Simple_MMC : public Converter
{
public:
    using MatrixXcd = Eigen::MatrixXcd;
    using VectorXcd = Eigen::VectorXcd;
    using VectorXd = Eigen::VectorXd;
    using VectorXi = Eigen::VectorXi;

    enum class ArmType { Upper, Lower };

    struct StepResult {  MatrixXcd VoutUp; MatrixXcd VoutLow;
    };

    static MatrixXcd makeArmControlCoeffs(int nCols, ArmType armType);

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
    StepResult simulateTimeStep(
        double Ts,
        double C,
        const MatrixXcd& Iup,
        const MatrixXcd& Ilow,
        int nArm,
        int nKeep
    );

  
private:
    int number_of_states = 6;

    double L_arm = 0.0;
    double R_arm = 0.0;
    double C_arm = 0.0;
    int N = 0;

    DQsym dq_;

    // Persistent internal harmonic memory
    MatrixXcd Uup_;
    MatrixXcd Ulow_;

    MatrixXcd ZupOld_;
    MatrixXcd XupOld_;
    MatrixXcd ZlowOld_;
    MatrixXcd XlowOld_;

    MatrixXcd lastVcUp_;
    MatrixXcd lastVcLow_;
    MatrixXcd lastVoutUp_;
    MatrixXcd lastVoutLow_;

    // Tracks currently allocated harmonic sizes
    int currentNArm_ = -1;
    int currentNKeep_ = -1;
};

#endif // SIMPLE_MMC_H


//kept as a ref to be removed later, the above code is the updated version with the new simulateTimeStep function and the new StepResult struct. 
// The old code is commented out below for reference.

//#ifndef SIMPLE_MMC_H
//#define SIMPLE_MMC_H
//
//#include "Converter.h"
//#include "../../Solver/DQsym/DQsym.h"
//#include "../../Include_control_blocks.h"
//
//class Simple_MMC : public Converter
//{
//public:
//    // 🔹 Make enum PUBLIC
//    enum class ArmType { Upper, Lower };
//
//    // 🔹 Make function PUBLIC
//    static MatrixXcd makeArmControlCoeffs(int nCols, ArmType armType);
//
//    // Constructor
//    Simple_MMC(const std::string& symbol, const std::string& location,
//        double omega, double activePower, double reactivePower,
//        double angle, double acVoltage, double Pdc, double dcVoltage,
//        double armInductance, double armResistance, double armCapacitance,
//        int numSubmodules, double reactorInductance, double reactorResistance);
//
//    // Constructor from vector
//    Simple_MMC(const std::string& symbol, const std::string& location,
//        const std::vector<double>& converter_params);
//
//    void computeABCD() override;
//
//    VectorXd simulateTimeStep(const Eigen::VectorXd& initial_state, double dt) override;
//
//    struct StepResult {
//        MatrixXcd y_mmc;
//        MatrixXcd i_up;
//        MatrixXcd i_low;
//        MatrixXcd vc_up;
//        MatrixXcd vc_low;
//        MatrixXcd vout_up;
//        MatrixXcd vout_low;
//    };
//
//    StepResult step(const MatrixXcd& u1,
//        const MatrixXcd& u2,
//        const VectorXi& brkVec);
//
//private:
//    VectorXcd x0;
//
//    double L_arm = 0.0;
//    double R_arm = 0.0;
//    double C_arm = 0.0;
//    int N = 0;
//
//    int number_of_states = 6;
//};
//
//#endif // SIMPLE_MMC_H