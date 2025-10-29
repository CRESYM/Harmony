#ifndef MMC_H
#define MMC_H

#include "Converter.h"
#include "../../Include_control_blocks.h"

// Forward declarations
class Controller;
class Filter;

class MMC : public Converter {
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
       
    
    // Equilibrium point calculation
    virtual void solveEquilibrium() override;
    virtual Eigen::MatrixXd computeStateDerivatives(const Eigen::VectorXd& x, const Eigen::VectorXd& u) override;
    virtual void computeABCD() override;

	// Y-parameter computation
    std::vector<std::vector<complex<double>>> compute_y_parameters(double frequency) override;
        
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
    double L_arm;    // Arm inductance [H]
    double R_arm;    // Arm resistance
    double C_arm;    // Capacitance per submodule [F]
    int N;           // Number of submodules per arm
    
    
	// State variables
    int number_of_states = 12;
	int vdc_index = 0; // Index for DC voltage in state vector

    
};

#endif // MMC_H