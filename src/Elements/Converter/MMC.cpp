#include "MMC.h"

#include "../../Include_control_blocks.h"

// -----------------------------------------------------------------------------
// State Vector Ordering
// -----------------------------------------------------------------------------
/**
 * @file MMC.cpp
 * @brief Implementation of the Modular Multilevel Converter (MMC) class.
 *
 * @details
 * The state vector 'x' in MMC is ordered as follows (from end to start):
 * [iDelta_d, iDelta_q, iSigma_d, iSigma_q, iSigma_z, vCDelta_d, vCDelta_q, vCDelta_Zd, vCDelta_Zq, vCSigma_d, vCSigma_q, vCSigma_z]
 * where:
 *   - iDelta_d, iDelta_q   : AC differential currents (d/q axis)
 *   - iSigma_d, iSigma_q   : Circulating currents (d/q axis)
 *   - iSigma_z             : Zero-sequence circulating current
 *   - vCDelta_d, vCDelta_q : AC capacitor voltages (d/q axis)
 *   - vCDelta_Zd, vCDelta_Zq : Zero-sequence capacitor voltages (d/q axis)
 *   - vCSigma_d, vCSigma_q : Circulating capacitor voltages (d/q axis)
 *   - vCSigma_z            : Zero-sequence circulating capacitor voltage
 *
 * Additional states (if present) are prepended for controllers, filters, PLL, and delay blocks.
 * The index 'i' is set to (number_of_states - 12) to access the main MMC states above.
 * Controller/filter/PLL/delay states are accessed at lower indices (i.e., x[0] ... x[i-1]).
 */
// -----------------------------------------------------------------------------

/**
 * @brief MMC constructor with explicit parameters.
 * @param symbol Element symbol/name.
 * @param omega Nominal angular frequency (rad/s).
 * @param activePower Active power (W).
 * @param reactivePower Reactive power (VAR).
 * @param angle Initial phase angle (rad).
 * @param acVoltage AC voltage amplitude (V).
 * @param dcVoltage DC voltage (V).
 * @param armInductance Arm inductance (H).
 * @param armResistance Arm resistance (Ohm).
 * @param armCapacitance Arm capacitance (F).
 * @param numSubmodules Number of submodules.
 * @param reactorInductance Reactor inductance (H).
 * @param reactorResistance Reactor resistance (Ohm).
 * @param timeDelay Modulation time delay (s).
 */
MMC::MMC(const std::string& symbol,
    double omega, double activePower, double reactivePower,
    double angle, double acVoltage, double Pdc, double dcVoltage,
    double armInductance, double armResistance, double armCapacitance,
    int numSubmodules, double reactorInductance, double reactorResistance,
    double timeDelay)
    : Element(symbol, 3, 1), // AC side - input pins; DC side - output pins
    omega_0(omega), P(activePower), Q(reactivePower), theta(angle), V_m(acVoltage), V_dc(dcVoltage),
    L_arm(armInductance), R_arm(armResistance), C_arm(armCapacitance),
    N(numSubmodules), L_reactor(reactorInductance), R_reactor(reactorResistance), t_delay(timeDelay),
    P_dc(Pdc)
{
    // Initialize active and reactive power limits for power flow calculations
    P_min = 0.5 * P;
    P_max = 1.5 * P;
    Q_min = -P;
    Q_max = P;

    A_matrix = Eigen::MatrixXd::Zero(3, 3);
    B_matrix = Eigen::MatrixXd::Zero(3, 3);
    C_matrix = Eigen::MatrixXd::Identity(3, 3);
    D_matrix = Eigen::MatrixXd::Zero(3, 3);

    if (t_delay != 0) {
        number_of_states += 5 * pade_order; // Add states for delay system
        Adelay = Eigen::MatrixXd::Zero(5 * pade_order, 5 * pade_order);
        Bdelay = Eigen::MatrixXd::Zero(5 * pade_order, 1); // Assuming one input for delay system
        Cdelay = Eigen::MatrixXd::Zero(5, 5 * pade_order); // Assuming one output for delay system
        Ddelay = Eigen::MatrixXd::Zero(5, 5); // Assuming one output for delay system
        cout << "Adding " << 5 * pade_order << " states for delay system with order " << pade_order << endl;
        if (pade_order == 2) {
            padeDelaySystemMulti2(t_delay, Adelay, Bdelay, Cdelay, Ddelay, 5);
        }
        else if (pade_order == 3) {
            padeDelaySystemMulti3(t_delay, Adelay, Bdelay, Cdelay, Ddelay, 5);
        }
        else {
            throw std::invalid_argument("Unsupported Padé order for delay system. Only 2nd and 3rd orders are supported.");
        }
    }

	Y_matrix.resize(3, 3);
}

/**
 * @brief MMC constructor with converter parameter vector.
 * @param symbol Element symbol/name.
 * @param converter_params Vector of converter parameters.
 */
MMC::MMC(const std::string& symbol, const std::vector<double>& converter_params)
    : Element(symbol, 3, 1), // AC side - input pins; DC side - output pins
    omega_0(converter_params[0]), P(converter_params[1]), Q(converter_params[2]),
    theta(converter_params[3]), V_m(converter_params[4]), P_dc(converter_params[5]), V_dc(converter_params[6]),
    L_arm(converter_params[7]), R_arm(converter_params[8]), C_arm(converter_params[9]),
    N(static_cast<int>(converter_params[10])), L_reactor(converter_params[11]),
    R_reactor(converter_params[12]), t_delay(converter_params[13]) {

    // Initialize active and reactive power limits for power flow calculations
    P_min = 0.5 * P;
    P_max = 1.5 * P;
    Q_min = -P;
    Q_max = P;

    // Initialize equilibrium state vector
    equilibrium_state = Eigen::VectorXd::Zero(6); // 6 dynamic states

    // Initialize system matrices to zero        
    A_matrix = Eigen::MatrixXd::Zero(3, 3);
    B_matrix = Eigen::MatrixXd::Zero(3, 3);
    C_matrix = Eigen::MatrixXd::Identity(3, 3);
    D_matrix = Eigen::MatrixXd::Zero(3, 3);

    if (t_delay != 0) {
        number_of_states += 5 * pade_order; // Add states for delay system
        Adelay = Eigen::MatrixXd::Zero(5 * pade_order, 5 * pade_order);
        Bdelay = Eigen::MatrixXd::Zero(5 * pade_order, 1); // Assuming one input for delay system
        Cdelay = Eigen::MatrixXd::Zero(5, 5 * pade_order); // Assuming one output for delay system
        Ddelay = Eigen::MatrixXd::Zero(5, 5); // Assuming one output for delay system
        cout << "Adding " << 5 * pade_order << " states for delay system with order " << pade_order << endl;
        if (pade_order == 2) {
            padeDelaySystemMulti2(t_delay, Adelay, Bdelay, Cdelay, Ddelay, 5);
			cout << "Using 2nd order Padé approximation for delay system." << endl;
			cout << Adelay << endl;
			cout << Bdelay << endl;
			cout << Cdelay << endl;
			cout << Ddelay << endl;
        }
        else if (pade_order == 3) {
            padeDelaySystemMulti3(t_delay, Adelay, Bdelay, Cdelay, Ddelay, 5);
        }
        else {
            throw std::invalid_argument("Unsupported Padé order for delay system. Only 2nd and 3rd orders are supported.");
        }
    }

    Y_matrix.resize(3, 3);
    //cout << "MMC initialized with " << number_of_states << " states." << endl;
};

/**
 * @brief MMC constructor with converter and controller parameter vectors.
 * @param symbol Element symbol/name.
 * @param converter_params Vector of converter parameters.
 * @param controller_params Vector of controller parameters.
 */
MMC::MMC(const std::string& symbol, const std::vector<double>& converter_params, const std::vector<double>& controller_params)
    : MMC(symbol, converter_params) // Call the constructor with converter_params
{
    // Initialize controllers and filters based on controller_params
    init_Controller(controller_params);
}

/**
 * @brief MMC constructor with converter, controller, and filter parameter vectors.
 * @param symbol Element symbol/name.
 * @param converter_params Vector of converter parameters.
 * @param controller_params Vector of controller parameters.
 * @param filter_params Vector of filter parameters.
 */
MMC::MMC(const std::string& symbol, const std::vector<double>& converter_params,
    const std::vector<double>& controller_params, const std::vector<double>& filter_params)
    : MMC(symbol, converter_params) // Call the constructor with converter_params
{
    // Initialize controllers based on controller_params
    init_Controller(controller_params);
    // Initialize filters based on filter_params
    init_Filter(filter_params);
}

/**
 * @brief Initialize the controller(s) in MMC using provided parameters.
 * @param controller_params Vector of controller parameters.
 */
void MMC::init_Controller(const std::vector<double>& controller_params) {
    for (int i = 0; i < controller_params.size(); ) {
        for (auto& controller_name : controller_list) {
			if (static_cast<bool>(controller_params[i])) { // If the controller is active
                if ((i + 3) >= controller_params.size()) {
                    throw std::invalid_argument("Insufficient parameters for controller initialization.");
                }
                // First read the controller type
				int controller_type = static_cast<int>(controller_params[++i]); // Default controller type is PI (value 0), P (value 1)
				int number_of_values; // Dimension of the controller output
               
                if (controller_type == 0) { // PI controller
					// Check if there are enough parameters for PI controller; at least 4 are expected for Kp, Ki, number_of_values, and references
                    if ((i + 3) >= controller_params.size()) {
                        throw std::invalid_argument("Insufficient parameters for PI controller initialization.");
                    }
                    std::vector<double> values = { controller_params[++i], controller_params[++i] };
					number_of_values = static_cast<int>(controller_params[++i]);
                    //cout << number_of_values << i << endl;
                    std::vector<double> refs;
                    if ((i + number_of_values) < controller_params.size()) {
                        refs = std::vector<double>(controller_params.begin() + i + 1, controller_params.begin() + i + 1 + number_of_values);
                        i++;
                    }
                    else {
                        refs.resize(number_of_values, 0.0); // Initialize references to zero if not provided
                    }
                    controls[controller_name] = new ProportionalIntegralController(controller_name, values, number_of_values, refs);
                }
                else if (controller_type == 1) { // P controller
					// Check if there are enough parameters for P controller; at least 3 are expected for Kp, number_of_values, and references
                    if ((i + 2) >= controller_params.size()) {
                        throw std::invalid_argument("Insufficient parameters for P controller initialization.");
                    }
					std::vector<double> values = { controller_params[++i] };
					number_of_values = static_cast<int>(controller_params[++i]);
                    cout << number_of_values << " " << i << endl;
                    std::vector<double> refs;
                    if ((i + number_of_values) < controller_params.size()) {
                        refs = std::vector<double>(controller_params.begin() + i + 1, controller_params.begin() + i + 1 + number_of_values);
                        i++;
                    }
                    else {
                        refs.resize(number_of_values, 0.0); // Initialize references to zero if not provided
                    }
                    controls[controller_name] = new ProportionalController(controller_name, values, number_of_values, refs);

                }
                else {
                    throw std::invalid_argument("Unsupported controller type. Only P (1) and PI (0) controllers are supported.");
				}
               
				// Update the number of states based on the controller type and number of values
                if (controller_name == "dc_voltage") {
                    if (t_delay != 0) {
                        vdc_index = number_of_states - 12 - 5 * pade_order; // Update vdc_index 
                    }
                    else {
                        vdc_index = number_of_states - 12; // Update vdc_index 
					}
                    number_of_states += number_of_values;
				}
				else if (controller_name == "pll") {
					number_of_states += 2; // PLL has 2 states (frequency and phase)
                }
                else if (controller_name == "droop") {
					number_of_states += 0; // Droop controller does not add states

					// Add reverse droop value for K
                    double Kdroop = (controls["droop"]->getParameters())[0];
                    controls["droop"]->setParameters({ 1.0 / Kdroop });
                }
                else 
                    number_of_states += number_of_values; // Update the number of states based on the number of values
                
                i += number_of_values;
            }
            else {
                i += 1; // Skip to the next controller
            }
        }    
    }

	// Check validity of controller parameters
    if (controls.count("dc_voltage") && controls.count("active_power"))
		throw std::invalid_argument("DC voltage and active power controllers cannot be used together in MMC.");
	else if (controls.count("droop") && (controls.count("active_power") || controls.count("dc_voltage")))
        throw std::invalid_argument("Droop controller, and DC voltage and/or active power controllers cannot be used together in MMC.");
	if (controls.count("ac_voltage") && controls.count("reactive_power"))
		throw std::invalid_argument("AC voltage and reactive power controllers cannot be used together in MMC.");
}

/**
 * @brief Initialize the filter(s) in MMC using provided parameters.
 * @param filter_params Vector of filter parameters, conaining filter names and their parameters.
 * @param filter_list List of filter name to initialize.
 * @param filter_size Size of the filter (default is 1). Only used for AC voltage dq filter.
 * @param filter_order Order of the filter (default is 2). Only used for AC voltage dq filter.
 * @details filter_params should contain the following structure: 0 if the filer is not used,
 * 1 if the filter is used, followed by the filter parameters (3 double values for each filter, 
 * representing Kp, Ki, and T). For the first order fiter, only T is used, and Kp and Ki are set to 0 in the main.cpp.
 */
void MMC::init_Filter(const std::vector<double>& filter_params) {
    for (int i = 0; i < filter_params.size(); ) {
        for (auto& filter_name : filter_list) {
            if (static_cast<bool>(filter_params[i])) {
                if ((i + 3) >= filter_params.size()) {
                    throw std::invalid_argument("Insufficient parameters for filter initialization.");
                }
                std::string filter_type = "LP"; // Default controller type
				int filter_order = 2; // Default filter size
                vector<double> values = { filter_params[i + 1], filter_params[i + 2], filter_params[i + 3]};
				int filter_size = 1; // Default filter size

                if (filter_name == "ac_voltage_dq") {
					filter_size = 2; // AC voltage dq filter has two inputs
					filter_order = 1; // AC voltage dq filter is a first-order filter
                }

                filters[filter_name] = new Filter(filter_name, filter_type, filter_order, values, filter_size);

                number_of_states += filter_size*filter_order; // Update the number of states based on the number of values
                i += 4;
            }
            else {
                i += 1; // Skip to the next filter
            }
        }
    }
}

/**
 * @brief Update MMC operating point and controller references.
 * @param Vm AC voltage magnitude.
 * @param theta AC voltage phase angle.
 * @param Pac Active power.
 * @param Qac Reactive power.
 * @param Vdc DC voltage.
 * @param Pdc DC power.
 */
void MMC::update_MMC(double Vm, double theta, double Pac, double Qac, double Vdc, double Pdc) {
    V_m = Vm;
    this->theta = theta;
    V_dc = Vdc;
    P = Pac;
    Q = Qac;
    P_dc = Pdc;

    const double Vgd = Vm * std::cos(theta);
    const double Vgq = -Vm * std::sin(theta);
    const double denom = Vgd * Vgd + Vgq * Vgq;

    if (denom < 1e-6) {
        throw std::runtime_error("Voltage magnitude too small for dq transformation.");
    }

    const double Id = (2.0 / 3.0) * (Vgd * P + Vgq * Q) / denom;
    const double Iq = (2.0 / 3.0) * (Vgq * P - Vgd * Q) / denom;

    // Set OCC controller reference (dq current)
    if (controls.count("occ")) {
        controls["occ"]->setReference({ Id, Iq });
    }

    // DC voltage control has priority over active power
    if (controls.count("dc_voltage")) {
        controls["dc_voltage"]->setReference({ Vdc });
    }
    else if (controls.count("active_power")) {
        controls["active_power"]->setReference({ Pac });
    }

    // Reactive power control has priority over AC voltage magnitude
    if (controls.count("reactive_power")) {
        controls["reactive_power"]->setReference({ Qac });
    }
    else if (controls.count("ac_voltage")) {
        controls["ac_voltage"]->setReference({ Vgd, Vgq });
    }

    // Zero circulating current control
    if (controls.count("zcc")) {
        if (Vdc > 1e-3) {  // Prevent division by zero
            controls["zcc"]->setReference({ (3.0 * Vgd * Id) / (6.0 * Vdc) });
        }
        else {
            controls["zcc"]->setReference({ 0.0 });
        }
    }

    // Energy controller
    if (controls.count("energy")) {
        controls["energy"]->setReference({ 3.0 * C_arm * Vdc * Vdc / N });
    }
}

/**
 * @brief Compute the state derivatives for the MMC system.
 * @param x State vector.
 * @param u Input vector.
 * @return MatrixXd State derivatives vector.
 *
 * @details
 * The state vector ordering is described at the top of this file.
 * Controller, filter, PLL, and delay states are prepended to the main MMC states.
 */
MatrixXd MMC::computeStateDerivatives(const Eigen::VectorXd& x, const Eigen::VectorXd& u) {
    // --- Precompute constants for MMC model ---
    const double Leqac = L_arm / 2.0 + L_reactor;
    const double Reqac = R_arm / 2.0 + R_reactor;
    const double Ce = 6.0 * C_arm / N;
    Eigen::VectorXd F = Eigen::VectorXd::Zero(number_of_states);

    // Extract state variables from the end of the state vector
    int i = number_of_states - 12;
    double iDelta_d = x(i), iDelta_q = x(i + 1), iSigma_z = x(i + 2), iSigma_d = x(i + 3), iSigma_q = x(i + 4);
    double vCDelta_d = x(i + 5), vCDelta_q = x(i + 6), vCDelta_Zd = x(i + 7), vCDelta_Zq = x(i + 8);
    double vCSigma_d = x(i + 9), vCSigma_q = x(i + 10), vCSigma_z = x(i + 11);

    // Constants for now
    double Vdc = (controls.count("dc_voltage")) ? x(vdc_index) : u(0);
    double w = omega_0;
    double Vgd = u(1);
    double Vgq = u(2);
    double Pac = 1.5 * (Vgd * iDelta_d + Vgq * iDelta_q);
    double Qac = 1.5 * (-Vgd * iDelta_q + Vgq * iDelta_d);
    double Vac_mag = 1.5 * sqrt(Vgd * Vgd + Vgq * Vgq);

    // Extract control reference voltages (user must assign these before this call)
    double vMDelta_d_ref = 0, vMDelta_q_ref = 0, vMDelta_Zd_ref = 0, vMDelta_Zq_ref = 0;
    double vMSigma_d_ref = 0, vMSigma_q_ref = 0, vMSigma_z_ref = Vdc / 2;

    // Preallocate and reuse Eigen vectors
    Eigen::VectorXd state_variables;
    Eigen::Vector2d x1, u1, c1;

    // Precompute transformation matrices only if PLL is present
    Eigen::Matrix2d T_theta = Eigen::Matrix2d::Identity();
    Eigen::Matrix2d I_theta = Eigen::Matrix2d::Identity();
    Eigen::Matrix2d T_2theta = Eigen::Matrix2d::Identity();
    Eigen::Matrix2d I_2theta = Eigen::Matrix2d::Identity();

    i = 0;
    if (controls.count("pll")) {
        double theta_c = x(i + 1);
        const double cos_theta = std::cos(theta_c);
        const double sin_theta = std::sin(theta_c); 
        T_theta << cos_theta, -sin_theta, sin_theta, cos_theta;
        I_theta << cos_theta, sin_theta, -sin_theta, cos_theta;

        const double cos_2theta = std::cos(-2 * theta_c);
        const double sin_2theta = std::sin(-2 * theta_c);
        T_2theta << cos_2theta, -sin_2theta, sin_2theta, cos_2theta;
        I_2theta << cos_2theta, sin_2theta, -sin_2theta, cos_2theta;

        Eigen::Vector2d Vg = T_theta * Eigen::Vector2d(u[1], u[2]);
        Vgd = Vg(0);
        Vgq = Vg(1);

        state_variables = controls["pll"]->define_equations(x(i), Vgq, 0);
        F(i) = state_variables(0);
        double delta_omega = state_variables(1);
        w = omega_0 + delta_omega;
        F(i + 1) = delta_omega; //control_blocks["pll"]->define_differential_equations(delta_omega);
        i += 2;
    }

    // Apply dq transformations (reuse matrices)
    Eigen::Vector2d i_delta_vec = T_theta * Eigen::Vector2d(iDelta_d, iDelta_q);
    Eigen::Vector2d i_sigma_vec = T_2theta * Eigen::Vector2d(iSigma_d, iSigma_q);
    iDelta_d = i_delta_vec(0);
    iDelta_q = i_delta_vec(1);
    iSigma_d = i_sigma_vec(0);
    iSigma_q = i_sigma_vec(1);

    // Filter and controller blocks (cache existence)
    bool has_ac_voltage_dq_filter = filters.count("ac_voltage_dq");
    bool has_active_power_filter = filters.count("active_power");
    bool has_reactive_power_filter = filters.count("reactive_power");
    bool has_dc_voltage_filter = filters.count("dc_voltage");
    bool has_ac_voltage_filter = filters.count("ac_voltage");

    if (has_ac_voltage_dq_filter) {
        x1 << x(i), x(i + 1);
        u1 << Vgd, Vgq;
        state_variables = filters["ac_voltage_dq"]->define_differential_equations(x1, u1);
        F(i) = state_variables(0);
        F(i + 1) = state_variables(1);
        Vgd = state_variables(2);
        Vgq = state_variables(3);
        i += 2;
    }
    if (has_active_power_filter) {
        x1 << x(i), x(i + 1);
        state_variables = filters["active_power"]->define_differential_equations(x1, Pac);
        F(i) = state_variables(0);
        F(i + 1) = state_variables(1);
        Pac = state_variables(2);
        i += 2;
    }
    if (has_reactive_power_filter) {
        x1 << x(i), x(i + 1);
        state_variables = filters["reactive_power"]->define_differential_equations(x1, Qac);
        F(i) = state_variables(0);
        F(i + 1) = state_variables(1);
        Qac = state_variables(2);
        i += 2;
    }
    if (has_dc_voltage_filter) {
        x1 << x(i), x(i + 1);
        state_variables = filters["dc_voltage"]->define_differential_equations(x1, Vdc);
        F(i) = state_variables(0);
        F(i + 1) = state_variables(1);
        Vdc = state_variables(2);
        i += 2;
    }
    if (has_ac_voltage_filter) {
        x1 << x(i), x(i + 1);
        state_variables = filters["ac_voltage"]->define_differential_equations(x1, Vac_mag);
        F(i) = state_variables(0);
        F(i + 1) = state_variables(1);
        Vac_mag = state_variables(2);
        i += 2;
    }

    // Outer control loops (cache existence)
    bool has_active_power_ctrl = controls.count("active_power");
    bool has_dc_voltage_ctrl = controls.count("dc_voltage");
	bool has_droop_ctrl = controls.count("droop");
    bool has_occ = controls.count("occ");
    
    if (has_active_power_ctrl) {
        state_variables = controls["active_power"]->define_equations(x(i), Pac, 0);
        F(i) = state_variables(0);
        double iDelta_d_ref = state_variables(1);
        i += 1;
        if (has_occ) controls["occ"]->setReference(iDelta_d_ref, 0);
    } else if (has_dc_voltage_ctrl) {
        double Idc = P_dc / Vdc; // u(0);
		x1 << 0, x(i+1);
        u1 << (-Idc + 3.0 * iSigma_z) / (1.0 * Ce), x(i);
		c1 << 0, 0; // No additional control inputs for dc_voltage
        state_variables = controls["dc_voltage"]->define_equations(x1, u1, c1);
		F(i) = state_variables(0);
        F(i + 1) = state_variables(1);
        double iDelta_d_ref = -state_variables(3);
        if (has_occ) controls["occ"]->setReference(iDelta_d_ref, 0);
        i += 2;
    }
    else if (has_droop_ctrl) {
        x1 << Vdc, 0;
		c1 << 0, 0; // No additional control inputs for droop
        VectorXd output = controls["droop"]->define_equations(c1, x1, c1);
		double iDelta_d_ref = (output(0) + output(1) / (controls["droop"]->getParameters())[0]) / Vgd;
    }

    bool has_reactive_power_ctrl = controls.count("reactive_power");
    bool has_ac_voltage_ctrl = controls.count("ac_voltage");

    if (has_reactive_power_ctrl) {
        state_variables = controls["reactive_power"]->define_equations(x(i), Qac, 0);
        F(i) = state_variables(0);
        double iDelta_q_ref = -state_variables(1);
        i += 1;
        if (has_occ) controls["occ"]->setReference(iDelta_q_ref, 1);
    } else if (has_ac_voltage_ctrl) {
        state_variables = controls["ac_voltage"]->define_equations(x(i), Vgd, 0);
        F(i) = state_variables(0);
        double iDelta_q_ref = state_variables(1);
        i += 1;
        if (has_occ) controls["occ"]->setReference(iDelta_q_ref, 1);
    }

    if (controls.count("energy")) {
        double wSigmaz = 3 * (C_arm * (pow(vCDelta_d, 2) + pow(vCDelta_q, 2) + pow(vCDelta_Zd, 2)
            + pow(vCDelta_Zq, 2) + pow(vCSigma_d, 2) + pow(vCSigma_q, 2) + 2 * pow(vCSigma_z, 2))) / (2.0 * N);
        controls["energy"]->setReference(3.0 * C_arm * V_dc * V_dc / N, 0);
        state_variables = controls["energy"]->define_equations(x(i), wSigmaz, Pac);
        F(i) = state_variables(0);
        double iSigma_z_ref = state_variables(1) / 3.0 / Vdc;
        i += 1;
        if (controls.count("zcc")) controls["zcc"]->setReference({ iSigma_z_ref });
    }

    if (controls.count("zcc")) {
        state_variables = controls["zcc"]->define_equations(x(i), iSigma_z, (-Vdc / 2));
        F(i) = state_variables(0);
        vMSigma_z_ref = -state_variables(1);
        i += 1;
    }

    if (has_occ) {
        vector<double> refs = controls["occ"]->getReference();
        if (!has_active_power_ctrl || !has_dc_voltage_ctrl) {
            Eigen::Vector2d i_delta_ref_vec = T_theta * Eigen::Vector2d(refs[0], refs[1]);
            double iDelta_d_ref = i_delta_ref_vec(0);
            double iDelta_q_ref = i_delta_ref_vec(1);
            controls["occ"]->setReference({ iDelta_d_ref, iDelta_q_ref });
        }
        x1 << x(i), x(i + 1);
        u1 << iDelta_d, iDelta_q;
        c1 << w * Leqac * iDelta_q + Vgd, -w * Leqac * iDelta_d + Vgq;
        state_variables = controls["occ"]->define_equations(x1, u1, c1);
        F(i) = state_variables(0);
        F(i + 1) = state_variables(1);
        vMDelta_d_ref = state_variables(2);
        vMDelta_q_ref = state_variables(3);
        Eigen::Vector2d vM_ref_c(vMDelta_d_ref, vMDelta_q_ref);
        Eigen::Vector2d vM_ref = I_theta * vM_ref_c;
        vMDelta_d_ref = vM_ref(0);
        vMDelta_q_ref = vM_ref(1);
        controls["occ"]->setReference(refs);
        i += 2;
    }

    if (controls.count("ccc")) {
        vector<double> refs = controls["ccc"]->getReference();
        if (!controls.count("reactive_power") || !controls.count("ac_voltage")) {
            Eigen::Vector2d i_sigma_ref_vec = T_theta * Eigen::Vector2d(refs[0], refs[1]);
            double iSigma_d_ref = i_sigma_ref_vec(0);
            double iSigma_q_ref = i_sigma_ref_vec(1);
            controls["ccc"]->setReference({ iSigma_d_ref, iSigma_q_ref });
        }
        x1 << x(i), x(i + 1);
        u1 << iSigma_d, iSigma_q;
        c1 << -2 * w * L_arm * iSigma_q, 2 * w * L_arm * iSigma_d;
        state_variables = controls["ccc"]->define_equations(x1, u1, c1);
        F(i) = state_variables(0);
        F(i + 1) = state_variables(1);
        vMSigma_d_ref = -state_variables(2);
        vMSigma_q_ref = -state_variables(3);
        Eigen::Vector2d vM_sigma_ref_c(vMSigma_d_ref, vMSigma_q_ref);
        Eigen::Vector2d vM_sigma_ref = I_2theta * vM_sigma_ref_c;
        vMSigma_d_ref = vM_sigma_ref(0);
        vMSigma_q_ref = vM_sigma_ref(1);
        controls["ccc"]->setReference(refs);
        i += 2;
    }

    // Compute un-delayed modulation signals
    Eigen::VectorXd m_input(7);
    m_input << -2 * vMDelta_d_ref / Vdc, -2 * vMDelta_q_ref / Vdc, -2 * vMDelta_Zd_ref / Vdc, -2 * vMDelta_Zq_ref / Vdc,
               2 * vMSigma_d_ref / Vdc, 2 * vMSigma_q_ref / Vdc, 2 * vMSigma_z_ref / Vdc;

    double mDelta_d = m_input(0);
    double mDelta_q = m_input(1);
    double mDelta_Zd = m_input(2);
    double mDelta_Zq = m_input(3);
    double mSigma_d = m_input(4);
    double mSigma_q = m_input(5);
    double mSigma_z = m_input(6);

    if (t_delay) {
		Eigen::VectorXd m(5);
		m << mDelta_d, mDelta_q, mSigma_d, mSigma_q, mSigma_z;
        Eigen::VectorXd xdelay = x.segment(i, 5*pade_order);
        F.segment(i, 5*pade_order) = Adelay * xdelay + Bdelay * m;
        Eigen::VectorXd mdelay = Cdelay * xdelay + Ddelay * m;
        i += 5*pade_order;
        mDelta_d = mdelay(0);
        mDelta_q = mdelay(1);
        mSigma_d = mdelay(2);
        mSigma_q = mdelay(3);
        mSigma_z = mdelay(4);
    }

    // Modulation voltages
    double vMDelta_d = (mDelta_q * vCSigma_q) / 4 - (mDelta_d * vCSigma_z) / 2 - (mDelta_d * vCSigma_d) / 4 - (mDelta_Zd * vCSigma_d) / 4
        + (mDelta_Zq * vCSigma_q) / 4 - (mSigma_d * vCDelta_d) / 4 - (mSigma_z * vCDelta_d) / 2 + (mSigma_q * vCDelta_q) / 4 - (mSigma_d * vCDelta_Zd) / 4 + (mSigma_q * vCDelta_Zq) / 4;
    double vMDelta_q = (mDelta_d * vCSigma_q) / 4 + (mDelta_q * vCSigma_d) / 4 - (mDelta_q * vCSigma_z) / 2 - (mDelta_Zd * vCSigma_q) / 4
        - (mDelta_Zq * vCSigma_d) / 4 + (mSigma_d * vCDelta_q) / 4 + (mSigma_q * vCDelta_d) / 4 - (mSigma_z * vCDelta_q) / 2 - (mSigma_d * vCDelta_Zq) / 4 - (mSigma_q * vCDelta_Zd) / 4;
    double vMDelta_Zd = -(mDelta_d * vCSigma_d) / 4 - (mDelta_q * vCSigma_q) / 4 - (mDelta_Zd * vCSigma_z) / 2 - (mSigma_d * vCDelta_d) / 4 - (mSigma_q * vCDelta_q) / 4 - (mSigma_z * vCDelta_Zd) / 2;
    double vMDelta_Zq = (mDelta_d * vCSigma_q) / 4 - (mDelta_q * vCSigma_d) / 4 - (mDelta_Zq * vCSigma_z) / 2 - (mSigma_d * vCDelta_q) / 4 + (mSigma_q * vCDelta_d) / 4 - (mSigma_z * vCDelta_Zq) / 2;

    double vMSigma_d = (mDelta_d * vCDelta_d) / 4 - (mDelta_q * vCDelta_q) / 4 + (mDelta_d * vCDelta_Zd) / 4 + (mDelta_Zd * vCDelta_d) / 4
        + (mDelta_q * vCDelta_Zq) / 4 + (mDelta_Zq * vCDelta_q) / 4 + (mSigma_d * vCSigma_z) / 2 + (mSigma_z * vCSigma_d) / 2;
    double vMSigma_q = (mDelta_q * vCDelta_Zd) / 4 - (mDelta_q * vCDelta_d) / 4 - (mDelta_d * vCDelta_Zq) / 4 - (mDelta_d * vCDelta_q) / 4
        + (mDelta_Zd * vCDelta_q) / 4 - (mDelta_Zq * vCDelta_d) / 4 + (mSigma_q * vCSigma_z) / 2 + (mSigma_z * vCSigma_q) / 2;
    double vMSigma_z = (mDelta_d * vCDelta_d) / 4 + (mDelta_q * vCDelta_q) / 4 + (mDelta_Zd * vCDelta_Zd) / 4 + (mDelta_Zq * vCDelta_Zq) / 4
        + (mSigma_d * vCSigma_d) / 4 + (mSigma_q * vCSigma_q) / 4 + (mSigma_z * vCSigma_z) / 2;


    // AC Current dynamics
    double diDeltad_dt = -(Vgd - vMDelta_d + Reqac * iDelta_d + Leqac * iDelta_q * w) / Leqac;
    double diDeltaq_dt = -(Vgq - vMDelta_q + Reqac * iDelta_q - Leqac * iDelta_d * w) / Leqac;
    
    
    // Common - mode Current dynamics
    double diSigmad_dt = -(vMSigma_d + R_arm * iSigma_d - 2 * L_arm * iSigma_q * w) / L_arm;
    double diSigmaq_dt = -(vMSigma_q + R_arm * iSigma_q + 2 * L_arm * iSigma_d * w) / L_arm;
    double diSigmaz_dt = -(vMSigma_z - Vdc / 2 + R_arm * iSigma_z) / L_arm;

	// Capacitor voltage dynamics
    double dvCSigmad_dt = (N * (iSigma_d * mSigma_z + iSigma_z * mSigma_d + iDelta_d * (mDelta_d / 4 + mDelta_Zd / 4) 
        - iDelta_q * (mDelta_q / 4 - mDelta_Zq / 4) + (4 * C_arm * vCSigma_q * w) / N)) / (2 * C_arm);
    double dvCSigmaq_dt = -(N * (iDelta_q * (mDelta_d / 4 - mDelta_Zd / 4) - iSigma_z * mSigma_q - iSigma_q * mSigma_z 
        + iDelta_d * (mDelta_q / 4 + mDelta_Zq / 4) + (4 * C_arm * vCSigma_d * w) / N)) / (2 * C_arm);
    double dvCSigmaz_dt = (N * (iDelta_d * mDelta_d + iDelta_q * mDelta_q + 2 * iSigma_d * mSigma_d + 2 * iSigma_q * mSigma_q + 4 * iSigma_z * mSigma_z)) / (8 * C_arm);

    double dvCDeltad_dt = (N * (iSigma_z * mDelta_d - (iDelta_q * mSigma_q) / 4 + iSigma_d * (mDelta_d / 2 + mDelta_Zd / 2) - iSigma_q * (mDelta_q / 2 + mDelta_Zq / 2) 
        + iDelta_d * (mSigma_d / 4 + mSigma_z / 2) - (2 * C_arm * vCDelta_q * w) / N)) / (2 * C_arm);
    double dvCDeltaq_dt = -(N * ((iDelta_d * mSigma_q) / 4 - iSigma_z * mDelta_q + iSigma_q * (mDelta_d / 2 - mDelta_Zd / 2) + iSigma_d * (mDelta_q / 2 - mDelta_Zq / 2) 
        + iDelta_q * (mSigma_d / 4 - mSigma_z / 2) - (2 * C_arm * vCDelta_d * w) / N)) / (2 * C_arm);
    double dvCDeltaZd_dt = (N * (iDelta_d * mSigma_d + 2 * iSigma_d * mDelta_d + iDelta_q * mSigma_q + 2 * iSigma_q * mDelta_q + 4 * iSigma_z * mDelta_Zd)) / (8 * C_arm) - 3 * vCDelta_Zq * w;
    double dvCDeltaZq_dt = 3 * vCDelta_Zd * w + (N * (iDelta_q * mSigma_d - iDelta_d * mSigma_q + 2 * iSigma_d * mDelta_q - 2 * iSigma_q * mDelta_d + 4 * iSigma_z * mDelta_Zq)) / (8 * C_arm);

    F(i++) = diDeltad_dt; F(i++) = diDeltaq_dt; F(i++) = diSigmaz_dt; F(i++) = diSigmad_dt; F(i++) = diSigmaq_dt;
    F(i++) = dvCDeltad_dt; F(i++) = dvCDeltaq_dt; F(i++) = dvCDeltaZd_dt; F(i++) = dvCDeltaZq_dt;
    F(i++) = dvCSigmad_dt; F(i++) = dvCSigmaq_dt; F(i++) = dvCSigmaz_dt;

    for (int i = 0; i < F.size(); ++i) {
    if (std::isnan(F(i)) || std::isinf(F(i))) {
        throw std::runtime_error("State derivative contains NaN or Inf.");
    }
}

    return F;
}

/**
 * @brief Numerically compute the Jacobian matrices A and B using finite differences.
 * @param x0 Operating point state vector.
 * @param u0 Operating point input vector.
 */
void MMC::computeABCD() {
    const Eigen::VectorXd& x0 = equilibrium_state;
    Eigen::VectorXd& u0 = VectorXd(3);
    // Define input vector u0 (DC voltage and AC voltages)
    if (controls.count("dc_voltage")) {
        u0 << P_dc / V_dc, V_m* cos(omega_0), V_m* sin(omega_0);
    }
    else {
        u0 << V_dc, V_m* cos(omega_0), V_m* sin(omega_0);
    }

    // Bind the member function computeStateDerivatives as a lambda
    auto f = [&](const Eigen::VectorXd& x, const Eigen::VectorXd& u) {
        return computeStateDerivatives(x, u);
        };

    // Compute both A = ∂f/∂x and B = ∂f/∂u
    std::pair<Eigen::MatrixXd, Eigen::MatrixXd> jacobians = computeJacobians(x0, u0, f);

    // Store in class variables
    A_matrix = jacobians.first; 
    B_matrix = jacobians.second;

    int n = A_matrix.cols();
    C_matrix = Eigen::MatrixXd::Zero(3,n);

	C_matrix(1, n - 12) = 1; // location of iDelta_d
	C_matrix(2, n - 11) = 1; // location of iDelta_q

    if (!controls.count("dc")) {
		C_matrix(0, n - 10) = 3; // location of iSigma_z
    }
	else { // to repair the C matrix
		C_matrix(0, vdc_index) = 1; // location of Vdc
    }

    D_matrix = Eigen::MatrixXd::Zero(3, 3);

    /*DenseMatrix I;
    SymEngine::eye(I, 3);
    DenseMatrix A_s;
    mul_dense_scalar(A_s, s, I);
    add_dense_dense(A_s, A_s, eigenToSymEngineDenseMatrix(-A_matrix));
    DenseMatrix inv_A_s = A_s.inv();

    DenseMatrix B_c = eigenToSymEngineDenseMatrix(B_matrix);
    DenseMatrix C_c = eigenToSymEngineDenseMatrix(C_matrix);
    DenseMatrix D_c = eigenToSymEngineDenseMatrix(D_matrix);
	mul_dense_dense(Y_matrix, C_c, inv_A_s);
	mul_dense_dense(Y_matrix, Y_matrix, B_c);
	add_dense_dense(Y_matrix, Y_matrix, D_c);

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < n; ++j) {
            Y_matrix.set(i, j, Y.get(i, j));
        }
	}

    Y_matrix.set(1, 1, neg(Y_matrix.get(1, 1))); // Correct the sign for the second row, second column
    Y_matrix.set(2, 2, neg(Y_matrix.get(2, 2))); // Correct the sign for the third row, third column

    if (controls.count("dc")) {
		RCP<const Basic> value = real_double(-C_arm * (6.0 / N));
        Y_matrix.set(0, 0, mul(real_double(2.0), (Y_matrix.get(0, 0), mul(s, value)))); // Correct the sign for the first row, first column
	} */
}

/**
 * @brief Solve for the steady-state operating point x using Newton-Raphson.
 */
void MMC::solveEquilibrium() {
    const int n = number_of_states;

    // Initial guess
    Eigen::VectorXd x0 = 0.01 * Eigen::VectorXd::Ones(n);
    const double Vgd = V_m * std::cos(theta);
    const double Vgq = -V_m * std::sin(theta);
    const double denom = Vgd * Vgd + Vgq * Vgq;

    if (denom < 1e-6) {
        throw std::runtime_error("Voltage magnitude too small for dq transformation.");
    }

    const double Id = (2.0 / 3.0) * (Vgd * P + Vgq * Q) / denom;
    const double Iq = (2.0 / 3.0) * (Vgq * P - Vgd * Q) / denom;

	x0(n - 12) = Id; // iDelta_d
	x0(n - 11) = Iq; // iDelta_q
	x0(n - 10) = P_dc / 3.0 / V_dc; // iSigma_z
	x0(n - 9) = 0; // iSigma_d
	x0(n - 8) = 0; // iSigma_q
    x0(n - 1) = V_dc;
	

    // Define input vector u (DC voltage and AC voltages)
    Eigen::VectorXd u(3);
    if (controls.count("dc_voltage")) {
        u << P_dc/V_dc, V_m* cos(omega_0), V_m* sin(omega_0);
    } 
    else {
        u << V_dc, V_m * cos(omega_0), V_m * sin(omega_0); 
	}

    // Wrap member function as a lambda
    DerivFunc f = [&](const Eigen::VectorXd& x, const Eigen::VectorXd& u) {
        return computeStateDerivatives(x, u);
        };

    // Call external equilibrium solver
    Eigen::VectorXd x_eq = findEquilibrium(x0, u, f);

    // Store result
    equilibrium_state = Eigen::VectorXd::Zero(number_of_states);
    equilibrium_state.head(number_of_states) = x_eq;
}

/**
 * @brief Compute the numerical admittance (Y) parameters for the MMC.
 * @param omega Angular frequency (rad/s).
 * @return Eigen::MatrixXcd Admittance matrix.
 */
std::vector<std::vector<complex<double>>> MMC::compute_y_parameters(double omega) {
    // s: Laplace variable (jω)
	std::complex<double> s_num = std::complex<double>(0, omega);
    const int n = A_matrix.rows();
    Eigen::MatrixXcd I = Eigen::MatrixXcd::Identity(n, n);
    Eigen::MatrixXcd A_s = s_num * I - A_matrix.cast<std::complex<double>>();
    Eigen::MatrixXcd inv_A_s = A_s.inverse();

    Eigen::MatrixXcd B_c = B_matrix.cast<std::complex<double>>();
    Eigen::MatrixXcd C_c = C_matrix.cast<std::complex<double>>();
    Eigen::MatrixXcd D_c = D_matrix.cast<std::complex<double>>();
	Eigen::MatrixXcd Y = C_c * inv_A_s * B_c + D_c;

	Y(1, 1) = -Y(1, 1); // Correct the sign for the second row, second column
	Y(2, 2) = -Y(2, 2); // Correct the sign for the third row, third column

    if (controls.count("dc")) {
        Y(0, 0) = 2.0 * (Y(0, 0) - s_num *  C_arm * (6.0/N)); // Correct the sign for the first row, first column
    }

    std::vector<std::vector<complex<double>>> Y_val_exact(Y_matrix.nrows());
    for (int i = 0; i < Y_matrix.nrows(); ++i) {
        Y_val_exact[i].resize(Y_matrix.ncols());
        for (int j = 0; j < Y_matrix.ncols(); ++j) {
            Y_val_exact[i][j] = std::complex<double>(Y(i, j).real(), Y(i, j).imag());
        }
	}

    return Y_val_exact;
}

/**
 * @brief Check system stability by evaluating eigenvalues of the A matrix.
 */
void MMC::checkStability() const {
    Eigen::EigenSolver<Eigen::MatrixXd> es(A_matrix);

    bool stable = true;
    for (int i = 0; i < es.eigenvalues().size(); ++i) {
        if (es.eigenvalues()(i).real() > 0) {
            stable = false;
            break;
        }
    }

    if (stable) {
        std::cout << "System is STABLE around this operating point.\n";
    }
    else if (es.eigenvalues().real().maxCoeff() > 0) {
        std::cout << "System is UNSTABLE around this operating point.\n";
    }
    else {
        std::cout << "System is MARGINALLY STABLE or needs further analysis.\n";
    }
}

/**
 * @brief Print eigenvalues of the A matrix.
 */
void MMC::printEigenvalues() const {
    Eigen::EigenSolver<Eigen::MatrixXd> es(A_matrix);
    std::cout << "Eigenvalues:\n" << es.eigenvalues() << "\n";
}

/**
 * @brief Print MMC element and controller parameter values.
 */
void MMC::printElementValues() {
    Element::printElementInfo();
    std::cout << "MMC Parameters:\n"
        << "  Active Power (P): " << P / 1e6 << " MW\n"
        << "  Reactive Power (Q): " << Q / 1e6 << " MVA\n"
        << "  DC Power (P_dc): " << P_dc / 1e6 << " MW\n"
        << "  AC Voltage Amplitude (V_m): " << V_m / 1e3 << " kV\n"
        << "  DC Voltage (V_dc): " << V_dc / 1e3 << " kV\n"
        << "  Nominal Frequency (omega_0): " << omega_0 << " rad/s\n"
        << "  Arm Inductance (L_arm): " << L_arm << " H\n"
        << "  Arm Resistance (R_arm): " << R_arm << " Omega\n"
        << "  Capacitance per Submodule (C_arm): " << C_arm << " F\n"
        << "  Number of Submodules (N): " << N << "\n"
        << "  Reactor Inductance (L_reactor): " << L_reactor << " H\n"
        << "  Reactor Resistance (R_reactor): " << R_reactor << " Omega\n"
        << "  Time Delay (t_delay): " << t_delay << " s\n";
    for (const auto& pair : controls) {
        const std::string& controllerName = pair.first;
        Controller* controller = pair.second;
        cout << endl;
        std::cout << "  Controller: " << controllerName << "\n";
        controller->printValues(); // Print controller values
    }
}