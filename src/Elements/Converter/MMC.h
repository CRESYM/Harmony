#ifndef MMC_H
#define MMC_H

/**
 * @file MMC.h
 * @brief Modular Multilevel Converter (MMC) state-space and MNA model.
 */

#include "Converter.h"
#include "../../Include_control_blocks.h"

// Forward declarations
class Controller;
class Filter;

/**
 * @class MMC
 * @brief Modular Multilevel Converter with arm dynamics and control loops.
 * @ingroup converter
 */
class MMC : public Converter {
public:
    /**
     * @brief Construct an MMC from individual scalar parameters.
     * @param symbol Element identifier.
     * @param location Network area string encoding AC and DC areas.
     * @param omega Nominal angular frequency (rad/s).
     * @param activePower Active power setpoint (W).
     * @param reactivePower Reactive power setpoint (VAR).
     * @param angle AC voltage angle (rad).
     * @param acVoltage AC voltage amplitude (V).
     * @param Pdc DC power (W).
     * @param dcVoltage DC bus voltage (V).
     * @param armInductance Arm inductance (H).
     * @param armResistance Arm resistance (Ω).
     * @param armCapacitance Submodule capacitance (F).
     * @param numSubmodules Number of submodules per arm.
     * @param reactorInductance Phase reactor inductance (H).
     * @param reactorResistance Phase reactor resistance (Ω).
     * @param timeDelay Control time delay (s).
     */
    MMC(const std::string& symbol, const std::string& location,
        double omega, double activePower, double reactivePower,
        double angle, double acVoltage, double Pdc, double dcVoltage,
        double armInductance, double armResistance, double armCapacitance,
        int numSubmodules, double reactorInductance, double reactorResistance,
        double timeDelay);

    /**
     * @brief Construct an MMC from a converter parameter vector.
     * @param symbol Element identifier.
     * @param location Network area string encoding AC and DC areas.
     * @param converter_params Packed converter parameters from init_MMC.
     */
    MMC(const std::string& symbol, const std::string& location, const std::vector<double>& converter_params);

    /**
     * @brief Construct an MMC with converter and controller parameters.
     * @param symbol Element identifier.
     * @param location Network area string encoding AC and DC areas.
     * @param converter_params Packed converter parameters.
     * @param controller_params Controller gain and setpoint parameters.
     */
    MMC(const std::string& symbol, const std::string& location, const std::vector<double>& converter_params, const std::vector<double>& controller_params);

    /**
     * @brief Construct an MMC with converter, controller, and filter parameters.
     * @param symbol Element identifier.
     * @param location Network area string encoding AC and DC areas.
     * @param converter_params Packed converter parameters.
     * @param controller_params Controller gain and setpoint parameters.
     * @param filter_params Filter time-constant and gain parameters.
     */
    MMC(const std::string& symbol, const std::string& location, const std::vector<double>& converter_params,
        const std::vector<double>& controller_params, const std::vector<double>& filter_params);

    // Initialization methods
    void init_Controller(const std::vector<double>& converter_params);// Method to initialize controllers and Filters in MMC
    void init_Filter(const std::vector<double>& converter_params);
    void update_MMC(double Vm, double theta, double Pac, double Qac, double Vdc, double Pdc);

    // Destructor — control blocks are freed by Converter base class
    ~MMC() override = default;
       
    
    // Equilibrium point calculation
    virtual void solveEquilibrium() override;
    virtual Eigen::MatrixXd computeStateDerivatives(const Eigen::VectorXd& x, const Eigen::VectorXd& u) override;
    virtual void computeABCD() override;
    /// Exact 12×12 plant Jacobian (modulation treated as fixed parameters)
    Eigen::MatrixXd computePlantJacobian(
        double w,
        double mDd, double mDq, double mDZd, double mDZq,
        double mSd, double mSq, double mSz) const;

    /// computeABCD variant: exact plant block + numerical controller block
    void computeABCD_analytical();

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
		data["xc"] = globalParams["omega"] * L_reactor / globalParams["ACZbase"]; // Base voltage for AC
		data["rc"] = R_reactor / globalParams["ACZbase"]; // Resistance of the phase reactor
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

    // One MMC arm-voltage time step
    //vector<MatrixXcd> simulateTimeStep(const vector<MatrixXcd>& input, double Ts, int nKeep1, int nKeep2) override;

    // State-space model manipulation - generic MNA stamping 
    void writeMNAmatrix(SymEngine::DenseMatrix&, std::unordered_map<Bus*, int>&, int,
        std::map<Element*, std::vector<RCP<const Basic>>>&) override;

    std::vector<RCP<const Basic>> getVirtualInputSymbols() const override;

    std::vector<MatrixXcd> simulateInputStep(
        const std::vector<MatrixXcd>& states, int nKeep) const override;

    int getNumberOfInternalStates() const override { return number_of_states; }

    //add18/5
    // 
    // // === BEGIN DQsym: expose plant-only state count ===
    int getNumberOfPlantStates() const override {
        /*std::cout << "[MMC::getNumberOfPlantStates] returning " << n_plant_states_ << "\n"; */
        return n_plant_states_; }
    // === END DQsym: expose plant-only state count ===

    //add18/5[

    // added18/5=== BEGIN DQsym closed-loop control (public interface) ===
    void stepControllers(double dt,
        const std::vector<Eigen::MatrixXcd>& states,
        const Eigen::Vector2d& Vg_dq);
    // added18/5]=== END DQsym closed-loop control ===

    // added18/5=== BEGIN DQsym closed-loop control (members) ===
    Eigen::VectorXd x_ctrl_dqsym_;          // persistent controller integrator states
    Eigen::MatrixXcd mD_dqsym_;             // current Δ-modulation (set by stepControllers)
    Eigen::MatrixXcd mS_dqsym_;             // current Σ-modulation
    bool dqsym_initialized_ = false;        // first-call init flag

    // Modulation references exposed by computeStateDerivatives (side-channel output).
    // Written every call; read only by stepControllers.
    mutable double last_vMDelta_d_ref_ = 0.0;
    mutable double last_vMDelta_q_ref_ = 0.0;
    mutable double last_vMSigma_d_ref_ = 0.0;
    mutable double last_vMSigma_q_ref_ = 0.0;
    mutable double last_vMSigma_z_ref_ = 0.0;
    // added18/5=== END DQsym closed-loop control ===



    map_basic_basic getParameterSubstitutions() const override;

private:
    double L_arm;    // Arm inductance [H]
    double R_arm;    // Arm resistance
    double C_arm;    // Capacitance per submodule [F]
    int N;           // Number of submodules per arm

    // Helper values
    double L_eq = 0.0, R_eq = 0.0, m_1 = 1.0;    
    
	// State variables
    int number_of_states = 12;
	int vdc_index = 0; // Index for DC voltage in state vector

    // add18/5=== BEGIN plant state count (captured at construction, before non-plant states added) ===
    int n_plant_states_ = 12;   // will be overwritten in constructor with actual value
    // add18/5=== END plant state count ===

    
};

#endif // MMC_H