#ifndef MMC_H
#define MMC_H

#include "../Element.h"
//#include "Filter.h"
//#include "Controller.h"

// Forward declarations
class Controller;
class Filter;
class ControlBlock;
class Integrator;

class MMC : public Element {
public:
    // Constructor 
    MMC(const std::string& symbol,
        double omega, double activePower, double reactivePower,
        double angle, double acVoltage, double dcVoltage,
        double armInductance, double armResistance, double armCapacitance,
        int numSubmodules, double reactorInductance, double reactorResistance,
        double timeDelay);

    // Constructor to initialize MMC with the converter_params (from init_MMC)
    MMC(const std::string& symbol, const std::vector<double>& converter_params);

    MMC(const std::string& symbol, const std::vector<double>& converter_params, const std::vector<double>& controller_params);

    MMC(const std::string& symbol, const std::vector<double>& converter_params,
        const std::vector<double>& controller_params, const std::vector<double>& filter_params);

    // Initialization methods
    void init_Controller(const std::vector<double>& converter_params);// Method to initialize controllers and Filters in MMC
    void init_Filter(const std::vector<double>& converter_params);
    void update_MMC(double Vm, double theta, double Pac, double Qac, double Vdc, double Pdc);

    // Destructor
    ~MMC() {};
    
    
   
    
    //getter
    Eigen::MatrixXd getA() const { return A_matrix; }
    Eigen::MatrixXd getB() const { return B_matrix; }
    Eigen::MatrixXd getC() const { return C_matrix; }
    Eigen::MatrixXd getD() const { return D_matrix; }
    Eigen::VectorXd getEquilibriumState() const { return equilibrium_state; }

    
    
    // Equilibrium point calculation
    void solveEquilibrium();
    Eigen::MatrixXd computeStateDerivatives(const Eigen::VectorXd& x, const Eigen::VectorXd& u);
    void computeJacobian(const Eigen::VectorXd& x0, const Eigen::VectorXd& u0);

    Eigen::MatrixXcd compute_y_parameters_num(double omega) override;

    // System analysis
    void checkStability() const;
    void printEigenvalues() const;

    // Override to print MMC-specific parameters
    virtual void printElementValues() override;

private:
    double omega_0;  // Nominal frequency
    double P;        // Active power [MW]
    double Q;        // Reactive power [MVA]
    double P_dc;     // DC power [kW]
    double P_min;    // Min active power output [MW]
    double P_max;    // Max active power output [MW]
    double Q_min;    // Min reactive power output [MVA]
    double Q_max;    // Max reactive power output [MVA]
    double theta;    // AC voltage angle [rad]
    double V_m;      // AC voltage amplitude [kV]
    double V_dc;     // DC-bus voltage [kV]
    double L_arm;    // Arm inductance [H]
    double R_arm;    // Arm resistance
    double C_arm;    // Capacitance per submodule [F]
    int N;           // Number of submodules per arm
    double L_reactor; // Inductance of the phase reactor [H]
    double R_reactor; // Resistance of the phase reactor [Ω]
    double t_delay;   // Time delay [s]
    
	// State variables
    int number_of_states = 12;
   
    // System matrices
    Eigen::MatrixXd A_matrix, B_matrix, C_matrix, D_matrix;
    Eigen::VectorXd equilibrium_state;
    
    // Sub-elements
    std::vector<std::pair<std::string, std::shared_ptr<Element>>> subElements;

	std::map<std::string, Controller*> controls; // Map of existing controllers
	std::map<std::string, Filter*> filters;      // Map of existing filters   
	std::map<std::string, Integrator*> control_blocks; // Map of control blocks

	// List of controller and filter names, it can be changed only by developers
    const std::vector<std::string> controller_list = {
        "pll",  "dc_voltage", "active_power", "ac_voltage", "reactive_power", "energy", "zcc", "occ", "ccc"
	}; // List of controller names
    const std::vector<std::string> filter_list = {
		"AC_voltage_dq", "active_power", "reactive_power", "dc_voltage", "ac_voltage"
	}; // List of filter names
};

#endif // MMC_H