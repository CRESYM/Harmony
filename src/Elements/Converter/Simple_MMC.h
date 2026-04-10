#ifndef SIMPLE_MMC_H
#define SIMPLE_MMC_H


#include "../../Solver/DQsym/DQsym.h"

#include "Converter.h"
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

    // Constructor to initialize MMC with the converter_params (from init_MMC)
    Simple_MMC(const std::string& symbol, const std::string& location, const std::vector<double>& converter_params);

    //Simple_MMC(const std::string& symbol, const std::string& location, const std::vector<double>& converter_params, const std::vector<double>& controller_params);

    //Simple_MMC(const std::string& symbol, const std::string& location, const std::vector<double>& converter_params,
    //    const std::vector<double>& controller_params, const std::vector<double>& filter_params);


    virtual VectorXd simulateTimeStep(const Eigen::VectorXd& initial_state, double dt) override;

private:
	// System matrices already defined in Converter class as: A_matrix, B_matrix, C_matrix, D_matrix

    
    // Initial state
    VectorXcd x0;


    double L_arm;    // Arm inductance [H]
    double R_arm;    // Arm resistance
    double C_arm;    // Capacitance per submodule [F]
    int N;           // Number of submodules per arm


    // State variables
    int number_of_states = 6;
   
    //// Internal control coefficient matrices
    //MatrixXcd Uup_;
    //MatrixXcd Ulow_;

    //// Internal arm integrator memory
    //MatrixXcd ZupOld_;
    //MatrixXcd XupOld_;
    //MatrixXcd ZlowOld_;
    //MatrixXcd XlowOld_;

    //// Feedback signals used as next MMC inputs
    //MatrixXcd VoutUpForMMC_;
    //MatrixXcd VoutLowForMMC_;

   
};

#endif // SIMPLE_MMC_H