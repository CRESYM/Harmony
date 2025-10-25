#ifndef MMC_H
#define MMC_H

#include "../Element.h"
#include "../../Include_control_blocks.h"

// Forward declarations
class Controller;
class Filter;

class MMC : public Element {
public:
    // Constructor 
    MMC(const std::string& symbol, const std::string& location,
        double omega, double activePower, double reactivePower,
        double angle, double acVoltage, double Pdc, double dcVoltage,
        double armInductance, double armResistance, double armCapacitance,
        int numSubmodules, double reactorInductance, double reactorResistance,
        double timeDelay);

    // Constructor to initialize MMC with the converter_params (from init_MMC)
    MMC(const std::string& symbol, const std::string& location, const std::vector<double>& converter_params);

    MMC(const std::string& symbol, const std::string& location, const std::vector<double>& converter_params, const std::vector<double>& controller_params);

    MMC(const std::string& symbol, const std::string& location, const std::vector<double>& converter_params,
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
	string getACarea() const { 
        auto pos = element_location.find('_');
        return element_location.substr(0, pos);
    } // Get AC area from location string
	string getDCarea() const { 
        auto pos = element_location.find('_');
        return element_location.substr(pos+1); 
    } // Get DC area from location string
    
    
    // Equilibrium point calculation
    void solveEquilibrium();
    Eigen::MatrixXd computeStateDerivatives(const Eigen::VectorXd& x, const Eigen::VectorXd& u);
    void computeABCD();

	// Y-parameter computation
    std::vector<std::vector<complex<double>>> compute_y_parameters(double frequency) override;

    // System analysis
    void checkStability() const;
    void printEigenvalues() const;

    // Override to print MMC-specific parameters
    virtual void printElementValues() override;

    void computePowerFlow(std::map<std::string, double>& data,
        std::map<std::string, double>& globalParams) const override
    {
        for (auto& [key, value] : element_OPF_info)
			data[key] = value; // Copy OPF info to branch data

		//data["bf"] = 0.0; // conductance of the filter
		//data["rf"] = 0.0; // Resistance of the filter
		//data["xf"] = 0.0; // Reactance of the filter
		data["xc"] = globalParams["omega"] * L_reactor / globalParams["Z_base"]; // Base voltage for AC
		data["rc"] = R_reactor / globalParams["Z_base"]; // Resistance of the phase reactor
        data["P_g"] = P / 1e6; // Setting of DC p-control value
        data["Q_g"] = Q / 1e6; // Setting of AC q-control value
        data["Vtar"] = V_dc / 1e3 / globalParams["DCbaseKV"]; // Seting of DC v-control value

		data["gridac"] = (int)element_location[2] - '0'; // AC grid number

        // DC side type_dc(1 = constant DC power control (i.e. active power), 2 = constant DC voltage control, 3 = DC droop control)
        if (controls.count("active_power")) {
            data["type_dc"] = 1;
		}
        else if (controls.count("dc_voltage")) {
			data["type_dc"] = 2;
		}
        else if (controls.count("droop")) {
			data["type_dc"] = 3;
        }
        
        // AC side control type_ac (1 = constant AC voltage control, 2 = constant reactive power control)
        if (controls.count("ac_voltage")) {
			data["type_ac"] = 1;
        }
        else if (controls.count("reactive_power")) {
            data["type_ac"] = 2;
        }
    }

private:
    double omega_0;  // Nominal frequency
    double P;        // Active power [W]
    double Q;        // Reactive power [VA]
    double P_dc;     // DC power [W]
    double P_min;    // Min active power output [W]
    double P_max;    // Max active power output [W]
    double Q_min;    // Min reactive power output [VA]
    double Q_max;    // Max reactive power output [VA]
    double theta;    // AC voltage angle [rad]
    double V_m;      // AC voltage amplitude [V]
    double V_dc;     // DC-bus voltage [V]
    double L_arm;    // Arm inductance [H]
    double R_arm;    // Arm resistance
    double C_arm;    // Capacitance per submodule [F]
    int N;           // Number of submodules per arm
    double L_reactor; // Inductance of the phase reactor [H]
    double R_reactor; // Resistance of the phase reactor [Ω]
    double t_delay;   // Time delay [s]
    
	// State variables
    int number_of_states = 12;
	int vdc_index = 0; // Index for DC voltage in state vector

    // System matrices
    Eigen::MatrixXd A_matrix, B_matrix, C_matrix, D_matrix;
	Eigen::MatrixXd Adelay, Bdelay, Cdelay, Ddelay; // Delay system matrices
	int pade_order = 2; // Order of Padé approximation for delays
    Eigen::VectorXd equilibrium_state;
    
    // Sub-elements
    std::vector<std::pair<std::string, std::shared_ptr<Element>>> subElements;

	std::map<std::string, Controller*> controls; // Map of existing controllers
	std::map<std::string, Filter*> filters;      // Map of existing filters   

	// List of controller and filter names, it can be changed only by developers
    const std::vector<std::string> controller_list = {
        "pll",  "dc_voltage", "active_power", "ac_voltage", "reactive_power", "energy", "zcc", "occ", "ccc", 
        "droop"
	}; // List of controller names
    const std::vector<std::string> filter_list = {
		"ac_voltage_dq", "ac_voltage", "active_power", "reactive_power", "dc_voltage"
	}; // List of filter names
};

#endif // MMC_H