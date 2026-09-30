#ifndef MMC_H
#define MMC_H

/**
 * @file MMC.h
 * @brief Modular Multilevel Converter (MMC) state-space and MNA model.
 */

#include "Converter.h"
#include "core/Include_control_blocks.h"

// Forward declarations
class Controller;
class Filter;

/**
 * @class MMC
 * @brief Modular Multilevel Converter with arm dynamics and control loops.
 * @ingroup converter
 *
 * Park (report eq. 4.54): vd = Vm cos θ, vq = −Vm sin θ.
 * Pac = 3/2 (vd id + vq iq),  Qac = 3/2 (vq id − vd iq),  Pdc = 3 vdc iΣz.
 * Signs: Pac>0 AC export, Qac>0 ⇒ iq<0 at vq=0, Pdc>0 DC import.
 * OPF (MatACDC): P_G=Pac, Q_G=Qac in MW; solver uses pn=−P_G, qs=−Q_G;
 * make_OPF writes back Pac=−ps, Qac=−qs, Pdc=−pn.
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

    void update_MMC(double Vm, double theta, double Pac, double Qac, double Vdc, double Pdc);

    /** @brief True when GFM outer-loop states are enabled. */
    bool hasGfm() const { return gfm_index_ >= 0 && controls.count("gfm") > 0; }

    /** @brief Return current GFM droops (Kdroop_P, Kdroop_Q). */
    std::pair<double, double> getGfmDroops() const;

    /** @brief Set GFM droops (Kdroop_P, Kdroop_Q); no-op if GFM disabled. */
    void setGfmDroops(double Kdroop_P, double Kdroop_Q);

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

        // Match the OPF converter equivalent to the MMC plant: no AC filter /
        // transformer, and rc+jxc = R_eq + jω L_eq with L_eq = L_arm/2 + L_reactor.
        // MatACDC row defaults (bf=0.0887 pu, LossA=1.103 MW, xc=0.164 pu) would
        // otherwise produce a PCC (V,P,Q,Pdc) that is not an MMC equilibrium.
        if (!element_OPF_info.count("bf"))
            data["bf"] = 0.0;
        if (!element_OPF_info.count("rtf"))
            data["rtf"] = 0.0;
        if (!element_OPF_info.count("xtf"))
            data["xtf"] = 0.0;
        if (!element_OPF_info.count("LossA"))
            data["LossA"] = 0.0;
        if (!element_OPF_info.count("LossB"))
            data["LossB"] = 0.0;
        if (!element_OPF_info.count("LossCrec"))
            data["LossCrec"] = 0.0;
        if (!element_OPF_info.count("LossCinv"))
            data["LossCinv"] = 0.0;
        if (!element_OPF_info.count("xc"))
            data["xc"] = globalParams["omega"] * L_eq / globalParams["ACZbase"];
        if (!element_OPF_info.count("rc"))
            data["rc"] = R_eq / globalParams["ACZbase"];
        // MatACDC P_G/Q_G: OPF enforces pn = -P_G and qs = -Q_G.
        // P_G/Q_G are MMC machine powers in MW (report Park Q = 1.5(Vq Id − Vd Iq)).
        data["P_g"] = P / 1e6;
        data["Q_g"] = Q / 1e6;
        data["Vtar"] = V_dc / 1e3 / globalParams["DCbaseKV"]; // Seting of DC v-control value

		data["gridac"] = (int)element_location[2] - '0'; // AC grid number

        // DC side type_dc(1 = constant DC power control (i.e. active power), 2 = constant DC voltage control, 3 = DC droop control)
        if (controls.count("active_power")) {
            if (!element_OPF_info.count("type_dc"))
                data["type_dc"] = 1;
		}
        else if (controls.count("dc_voltage")) {
            if (!element_OPF_info.count("type_dc"))
			data["type_dc"] = 2;
		}
        else if (controls.count("droop")) {
            if (!element_OPF_info.count("type_dc"))
			data["type_dc"] = 3;
        }
        
        // AC side type_ac matches Powerflow_solver / MatACDC: 1 = Q, 2 = Vac.
        if (controls.count("ac_voltage")) {
            if (!element_OPF_info.count("type_ac"))
			data["type_ac"] = 2;
        }
        else if (controls.count("reactive_power")) {
            if (!element_OPF_info.count("type_ac"))
                data["type_ac"] = 1;
        }

        if (element_OPF_info.count("type_dc"))
            data["type_dc"] = element_OPF_info.at("type_dc");
        if (element_OPF_info.count("type_ac"))
            data["type_ac"] = element_OPF_info.at("type_ac");
    }

    // State-space model manipulation - generic MNA stamping 
    void writeMNAmatrix(SymEngine::DenseMatrix&, std::unordered_map<Bus*, int>&, int,
        std::map<Element*, std::vector<RCP<const Basic>>>&) override;

    std::vector<RCP<const Basic>> getVirtualInputSymbols() const override;

    void simulateInputStep(
        const std::vector<MatrixXcd>& states, int nKeep,
        std::vector<MatrixXcd>& out) const override;

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

    /** @brief Write the 12 plant slots of @p x from DQsym harmonic groups (shared with stepControllers). */
    void fillPlantFromHarmonics(Eigen::VectorXd& x,
        const std::vector<Eigen::MatrixXcd>& states) const;
    // added18/5]=== END DQsym closed-loop control ===

    // added18/5=== BEGIN DQsym closed-loop control (members) ===
    Eigen::VectorXd x_ctrl_dqsym_;          // persistent controller integrator states
    Eigen::MatrixXcd mD_dqsym_;             // current Δ-modulation (set by stepControllers)
    Eigen::MatrixXcd mS_dqsym_;             // current Σ-modulation
    bool dqsym_initialized_ = false;        // first-call init flag
    mutable MatrixXcd dq_prod_a_, dq_prod_b_;  // dq_multiply scratch (simulateInputStep)

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
	int vdc_index = 0; // Index of the DC-voltage PI integrator when dc_voltage is enabled
	int gfm_index_ = -1; // Start index of GFM states (theta, Pac_f, Qac_f); -1 if disabled
	double gfm_E_ref_ = 0.0; // GFM internal voltage magnitude reference
	/// When true, GFM residuals use power-normalized form for KINSOL conditioning.
	bool gfm_scale_eq_residual_ = false;
	/// When true (equilibrium solve only), replace the Vdc-integrator residual
	/// Ki(v*−v) with P_dc/V_dc − 3 i^Σ_z. With V_dc as a port, v=v* so the
	/// dynamic residual is identically zero and does not pin ξ or i_d.
	bool vdc_eq_idc_residual_ = false;

    // add18/5=== BEGIN plant state count (captured at construction, before non-plant states added) ===
    int n_plant_states_ = 12;   // will be overwritten in constructor with actual value
    // add18/5=== END plant state count ===

    // Open-loop feedforward used when outer controllers (occ/zcc) are absent.
    bool open_loop_modulation_ = false;
    double ol_vMDelta_d_ref_ = 0.0;
    double ol_vMDelta_q_ref_ = 0.0;
    double ol_vMSigma_z_ref_ = 0.0;
    Eigen::VectorXd equilibrium_guess_;

    void computeOpenLoopArmRefs(
        double Id, double Iq, double Vdc, double iSigma_z,
        double& vMDelta_d, double& vMDelta_q, double& vMSigma_z) const;
    void initializeDelayStates(
        Eigen::VectorXd& x0, double Vdc,
        double vMDelta_d, double vMDelta_q, double vMSigma_z) const;
    void seedPlantStateGuess(
        Eigen::VectorXd& x0, double Id, double Iq, double iSigma_z) const;

    void init_Controller(const std::vector<double>& converter_params);
    void init_Filter(const std::vector<double>& converter_params);
    
};

#endif // MMC_H