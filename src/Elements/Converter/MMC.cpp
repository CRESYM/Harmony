#include "MMC.h"

#include "../../Control/Controller.h"
#include "../../Control/Filter.h"
#include "../../Control/Integrator.h" // Add this include to ensure Integrator is recognized

// Constructors
MMC::MMC(const std::string& symbol,
    double omega, double activePower, double reactivePower,
    double angle, double acVoltage, double dcVoltage,
    double armInductance, double armResistance, double armCapacitance,
    int numSubmodules, double reactorInductance, double reactorResistance,
    double timeDelay)
    : Element(symbol, 3, 1), // AC side - input pins; DC side - output pins
    omega_0(omega), P(activePower), Q(reactivePower), theta(angle), V_m(acVoltage), V_dc(dcVoltage),
    L_arm(armInductance), R_arm(armResistance), C_arm(armCapacitance),
    N(numSubmodules), L_reactor(reactorInductance), R_reactor(reactorResistance), t_delay(timeDelay)
{
    // Initialize active and reactive power limits for power flow calculations
    P_dc = P;
    P_min = 0.5 * P;
    P_max = 1.5 * P;
    Q_min = -P;
    Q_max = P;

    A_matrix = Eigen::MatrixXd::Zero(6, 6);
    B_matrix = Eigen::MatrixXd::Zero(6, 8);
    C_matrix = Eigen::MatrixXd::Identity(6, 6);
    D_matrix = Eigen::MatrixXd::Zero(6, 8);
}

MMC::MMC(const std::string& symbol, const std::vector<double>& converter_params)
    : Element(symbol, 3, 1), // AC side - input pins; DC side - output pins
    omega_0(converter_params[0]), P(converter_params[1]), Q(converter_params[2]),
    theta(converter_params[3]), V_m(converter_params[4]), V_dc(converter_params[5]),
    L_arm(converter_params[6]), R_arm(converter_params[7]), C_arm(converter_params[8]),
    N(static_cast<int>(converter_params[9])), L_reactor(converter_params[10]),
    R_reactor(converter_params[11]), t_delay(converter_params[12]) {

    // Initialize active and reactive power limits for power flow calculations
    P_dc = P;
    P_min = 0.5 * P;
    P_max = 1.5 * P;
    Q_min = -P;
    Q_max = P;

    // Initialize equilibrium state vector
    equilibrium_state = Eigen::VectorXd::Zero(6); // 6 dynamic states

    // Initialize system matrices to zero        
    A_matrix = Eigen::MatrixXd::Zero(6, 6);
    B_matrix = Eigen::MatrixXd::Zero(6, 8);
    C_matrix = Eigen::MatrixXd::Identity(6, 6);
    D_matrix = Eigen::MatrixXd::Zero(6, 8);
};

MMC::MMC(const std::string& symbol, const std::vector<double>& converter_params, const std::vector<double>& controller_params)
    : MMC(symbol, converter_params) // Call the constructor with converter_params
{
    // Initialize controllers and filters based on controller_params
    init_Controller(controller_params);
}

MMC::MMC(const std::string& symbol, const std::vector<double>& converter_params,
    const std::vector<double>& controller_params, const std::vector<double>& filter_params)
    : MMC(symbol, converter_params) // Call the constructor with converter_params
{
    // Initialize controllers based on controller_params
    init_Controller(controller_params);
    // Initialize filters based on filter_params
    init_Filter(filter_params);
}

// Initialize the controller(s) in MMC using provided parameters
void MMC::init_Controller(const std::vector<double>& controller_params) {
    for (int i = 0; i < controller_params.size(); ) {
        for (auto& controller_name : controller_list) {
            if (static_cast<bool>(controller_params[i])) {
                if ((i + 3) >= controller_params.size()) {
                    throw std::invalid_argument("Insufficient parameters for controller initialization.");
                }
                std::string controller_type = "PI"; // Default controller type
                std::vector<double> values = {controller_params[i+1], controller_params[i+2]};
                int number_of_values = static_cast<int>(controller_params[i+3]); // Default number of values for PI controller
                std::vector<double> refs;
                if ((i + 3 + number_of_values) < controller_params.size()) {
                    refs = std::vector<double>(controller_params.begin() + i + 4, controller_params.begin() + i + 4 + number_of_values);
                }
                else {
                    refs.resize(number_of_values, 0.0); // Initialize references to zero if not provided
                }
                number_of_states += number_of_values; // Update the number of states based on the number of values
                // Create a new controller and add it to the controls map
                controls[controller_name] = new Controller(controller_name, controller_type, values, number_of_values, refs);

                if (controller_name == "PLL")
                    control_blocks["pll"] = new Integrator();

                i += 4 + number_of_values;
            }
            else {
                i += 1; // Skip to the next controller
            }
        }    
    }
}


// Initialize the filter(s) in MMC using provided parameters
// This method assumes that the filter parameters are provided in a specific format
// first value is a boolean indicating whether the filter is enabled,
// followed by the filter type, order (i.e., 1st or 2nd), values.
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

void MMC::update_MMC(double Vm, double theta, double Pac, double Qac, double Vdc, double Pdc) {
    this->V_m = Vm;
    this->theta = theta;
    this->V_dc = Vdc;
    this->P = Pac;
    this->Q = Qac;
    this->P_dc = Pdc;

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

MatrixXd MMC::computeStateDerivatives(const Eigen::VectorXd& x, const Eigen::VectorXd& u) {
    double Leqac = L_arm / 2.0 + L_reactor;
    double Reqac = R_arm / 2.0 + R_reactor;
    double Ce = 6.0 * C_arm / N;

    // Extract state variables from the end of the state vector
    int i = x.size() - 12; // Assuming the last 12 elements are the state variables
    double iDelta_d = x(i), iDelta_q = x(i + 1), iSigma_d = x(i + 2), iSigma_q = x(i + 3), iSigma_z = x(i + 4);
    double vCDelta_d = x(i + 5), vCDelta_q = x(i + 6), vCDelta_Zd = x(i + 7), vCDelta_Zq = x(i + 8);
    double vCSigma_d = x(i + 9), vCSigma_q = x(i + 10), vCSigma_z = x(i + 11);

    // Constants for now
	double Vdc = (controls.count("dc_voltage")) ? u(0) : V_dc; // DC voltage
	double w = omega_0; // Angular frequency (rad/s)
	double Vgd = u(1); // d-axis voltage
	double Vgq = u(2); // q-axis voltage
    double Pac = (3 / 2) * (Vgd * iDelta_d + Vgq * iDelta_q);
    double Qac = (3 / 2) * (-Vgd * iDelta_q + Vgq * iDelta_d);
	double Vac_mag = 1.5 * sqrt(Vgd * Vgd + Vgq * Vgq); // AC voltage magnitude

    int number_of_states = x.size();
    Eigen::VectorXd F = Eigen::VectorXd::Zero(number_of_states);

    // Extract control reference voltages (user must assign these before this call)
    double vMDelta_d_ref = 0, vMDelta_q_ref = 0, vMDelta_Zd_ref = 0, vMDelta_Zq_ref = 0;
	double vMSigma_d_ref = 0, vMSigma_q_ref = 0, vMSigma_z_ref = 0;

    
    i = 0;
	// Placeholders for values used in control loops
    Eigen::VectorXd state_variables; // Placeholder for state variables
    Eigen::VectorXd x1 = Eigen::VectorXd(2);
    Eigen::VectorXd u1 = Eigen::VectorXd(2);
    Eigen::VectorXd c1 = Eigen::VectorXd(2);

	// LOOPS ARE ADDED IN FORMAT: outer loop that creates reference for inner loop and then inner loop
    // 
    // PLL
    // Define matrices for PLL
    Eigen::Matrix2d T_theta;
    Eigen::Matrix2d I_theta;
    Eigen::Matrix2d T_2theta;
    Eigen::Matrix2d I_2theta;
    T_theta.setIdentity();
    I_theta.setIdentity();
    T_2theta.setIdentity();
    I_2theta.setIdentity();

	// Adding PLL control loop
    if (controls.count("pll")) {
        double theta_c = x(i+1);
        double cos_theta = std::cos(theta_c);
        double sin_theta = std::sin(theta_c);

        // Transformation matrices
        T_theta << cos_theta, -sin_theta, sin_theta, cos_theta;
        I_theta << cos_theta, sin_theta, -sin_theta, cos_theta;

        double cos_2theta = std::cos(-2 * theta_c);
        double sin_2theta = std::sin(-2 * theta_c);

        T_2theta << cos_2theta, -sin_2theta, sin_2theta, cos_2theta;
        I_2theta << cos_2theta, sin_2theta, -sin_2theta, cos_2theta;

        Eigen::Vector2d Vg = T_theta * Eigen::Vector2d(u[1], u[2]);

        Vgd = Vg(0);
        Vgq = Vg(1);

		state_variables = controls["pll"]->define_differential_equations(x(i), Vgq, 0);
        // PLL control output
        F(i) = state_variables(i);
		double delta_omega = state_variables(1); // Reference frequency
        w = omega_0 + delta_omega;

        // Go through the integrator to get theta
		F(i + 1) = control_blocks["pll"]->define_differential_equations(delta_omega); // PLL output   

        i += 2;
    }
    else {
        Vgd = u(1);
        Vgq = u(2);
        w = omega_0;
    }

    // First to add filters
    if (filters.count("ac_voltage_dq")) {
        // Define x, u, and c for AC voltage dq filter (1st order)
        x1 << x(i), x(i+1); // Initialize x1 with the first two state variables
        u1 << Vgd, Vgq; // Initialize u1 with the first two state variables
        state_variables = filters["ac_voltage_dq"]->define_differential_equations(x1, u1);
        F(i) = state_variables(0); 
        F(i+1) = state_variables(1); 
		Vgd = state_variables(2); // Update Vgd from filter output
		Vgq = state_variables(3); // Update Vgq from filter output
		i += 2; // Move to next state variables
    }
    if (filters.count("active_power")) {
        // Define x, u, and c for active power (2nd order)
        x1 << x(i), x(i + 1); // Initialize x1 with the first two state variables
        state_variables = filters["active_power"]->define_differential_equations(x1, Pac);
        F(i) = state_variables(0);
        F(i + 1) = state_variables(1);
        Pac = state_variables(2); // Update Pac from filter output, and use in the following equations
        i += 2; // Move to next state variables
    }
    if (filters.count("reactive_power")) {
        // Define x, u, and c for active power (2nd order)
        x1 << x(i), x(i + 1); // Initialize x1 with the first two state variables
        state_variables = filters["reactive_power"]->define_differential_equations(x1, Qac);
        F(i) = state_variables(0);
        F(i + 1) = state_variables(1);
        Qac = state_variables(2); // Update Pac from filter output, and use in the following equations
        i += 2; // Move to next state variables
    }
    if (filters.count("dc_voltage")) {
        // Define x, u, and c for DC voltage (2nd order)
        x1 << x(i), x(i + 1); // Initialize x1 with the first two state variables
        state_variables = filters["dc_voltage"]->define_differential_equations(x1, Vdc);
        F(i) = state_variables(0);
        F(i + 1) = state_variables(1);
        Vdc = state_variables(2); // Update Vdc from filter output, and use in the following equations
        i += 2; // Move to next state variables
	}
	if (filters.count("ac_voltage")) { // NOT USED YET
        // Define x, u, and c for AC voltage (2nd order)
        x1 << x(i), x(i + 1); // Initialize x1 with the first two state variables
        state_variables = filters["ac_voltage"]->define_differential_equations(x1, Vac_mag);
        F(i) = state_variables(0);
        F(i + 1) = state_variables(1);
        Vac_mag = state_variables(2); // Update Vgd from filter output
        i += 2; // Move to next state variables
	}
     
    // OUTER LOOPS
	// Adding energy control loop
    if (controls.count("energy")) {
        double wSigmaz = 3 * (C_arm * (pow(vCDelta_d,2) + pow(vCDelta_q,2) + pow(vCDelta_Zd,2) 
            + pow(vCDelta_Zq,2) + pow(vCSigma_d,2) + pow(vCSigma_q,2) + 2 * pow(vCSigma_z,2))) / (2 * N);
		controls["energy"]->setReference(3.0 * C_arm * Vdc * Vdc / N, 0);

        state_variables = controls["energy"]->define_differential_equations(x(i), wSigmaz, Pac);
        F(i) = state_variables(0); // dxwSigmaz_dt = (wSigmaz_ref -  wSigmaz);
        double iSigma_z_ref = state_variables(1) / 3 / Vdc; 
		i += 1; // Move to next state variables

        if (controls.count("zcc"))
			controls["zcc"]->setReference({ iSigma_z_ref }); // Set reference for zero circulating current controller
        else
			throw std::runtime_error("Zero circulating current controller not available for energy control loop.");
    } 
    
	// NOTE: Only one of the following two loops will be executed, depending on the controller availability
	// Adding active power control loop
    if (controls.count("active_power")) {
        state_variables = controls["active_power"]->define_differential_equations(x(i), Pac, 0);
		F(i) = state_variables(0); // dxiPac_dt = (Pac_ref - Pac);
        double iDelta_d_ref = state_variables(1); // to check!!!
		i += 1; // Move to next state variables

		if (controls.count("occ"))
            controls["occ"]->setReference(iDelta_d_ref, 0); // Set reference for outer control loop
		else
			throw std::runtime_error("Outer current control loop is not available.");
    }
	// Adding DC voltage control loop
    else if (controls.count("dc_voltage")) { // to complete later, more difficult
		double Idc = P_dc / Vdc; // DC current
        state_variables = controls["dc_voltage"]->define_differential_equations(x(i), Vdc, P_dc);

		i += 2; // Move to next state variables
    }

    // NOTE: Only one of the following two loops will be executed, depending on the controller availability
	// Adding reactive power control loop
    if (controls.count("reactive_power")) {
        state_variables = controls["reactive_power"]->define_differential_equations(x(i), Qac, 0);
        F(i) = state_variables(0); // dxiQac_dt = (Qac_ref - Qac);
        double iDelta_q_ref = -state_variables(1);
        i += 1; // Move to next state variables

        if (controls.count("occ"))
            controls["occ"]->setReference(iDelta_q_ref, 1); // Set reference for outer control loop
        else
            throw std::runtime_error("Outer current control loop is not available.");
    }
	// Adding AC voltage control loop
    else if (controls.count("ac_voltage")) {
        state_variables = controls["ac_voltage"]->define_differential_equations(x(i), Vgd, 0);
		F(i) = state_variables(0); // dxiVgd_dt = (Vgd_ref - Vgd);
		double iDelta_q_ref = state_variables(1);
        if (controls.count("occ"))
            controls["occ"]->setReference(iDelta_q_ref, 1); // Set reference for outer control loop
        else
            throw std::runtime_error("Outer current control loop is not available.");
	}

	// Adding zero current control loop that gets reference from energy control loop, or given
    if (controls.count("zcc"))
    {
        state_variables = controls["zcc"]->define_differential_equations(x(i), iSigma_z, (-Vdc / 2));
        F(i) = state_variables(0); // dxiSigmaz_dt = (iSigmaz_ref -  iSigmaz);
        vMSigma_z_ref = -state_variables(1);
        i += 1; // Move to next state variables
    }

    // Adding inner control loops
    if (controls.count("occ")) {
		// Define x, u, and c for OCC controller
        x1 << x(i), x(i + 1); // Initialize x1 with the first two state variables
        u1 << iDelta_d, iDelta_q; // Initialize u1 with the first two state variables
        c1 << w * Leqac * iDelta_q + Vgd, -w * Leqac * iDelta_d + Vgq; // Initialize c1 with the voltage references
        state_variables = controls["occ"]->define_differential_equations(x1, u1, c1);
        F(i) = state_variables(0); // dxiDeltad_dt = (iDeltad_ref -  iDeltad);
        F(i + 1) = state_variables(1); // dxiDeltaq_dt = (iDeltaq_ref -  iDeltaq);
        vMDelta_d_ref = state_variables(2);
        vMDelta_q_ref = state_variables(3);
        i += 2; // Move to next state variables
    }

    if (controls.count("ccc")) {
        x1 << x(i), x(i + 1); // Initialize x1 with the first two state variables
        u1 << iSigma_d, iSigma_q; // Initialize u1 with the first two state variables
        c1 << -2 * w * L_arm * iSigma_q, 2 * w * L_arm * iSigma_d; // Initialize c1 with the voltage references
        state_variables = controls["ccc"]->define_differential_equations(x1, u1, c1);
        F(i) = state_variables(0); // dxiSigmad_dt = (iSigmad_ref - iSigmad);
        F(i + 1) = state_variables(1); // dxiSigmaq_dt = (iSigmaq_ref - iSigmaq);
        vMSigma_d_ref = -state_variables(2);
        vMSigma_q_ref = -state_variables(3);
        i += 2; // Move to next state variables
    }

    // Compute un-delayed modulation signals
    Eigen::VectorXd m_input(7);
    m_input << -2 * vMDelta_d_ref / Vdc, -2 * vMDelta_q_ref / Vdc, -2*vMDelta_Zd_ref / Vdc, -2*vMDelta_Zq_ref / V_dc, 2 * vMSigma_d_ref / Vdc, 2 * vMSigma_q_ref / Vdc, 2 * vMSigma_z_ref / Vdc;

    double mDelta_d = m_input(0);
    double mDelta_q = m_input(1);
    double mDelta_Zd = m_input(2);
    double mDelta_Zq = m_input(3);
    double mSigma_d = m_input(4);
    double mSigma_q = m_input(5);
    double mSigma_z = m_input(6);

    if (t_delay) {
        // Create delay matrices for 5 signals
        auto [AB, CD] = padeDelaySystemMulti(t_delay, 5);
        auto [Adelay, Bdelay] = AB;
        auto [Cdelay, Ddelay] = CD;

        // Apply update logic
        Eigen::VectorXd xdelay = x.segment(i, 15);             // 3rd-order Padé × 5 signals = 15 states
        F.segment(i, 15) = Adelay * xdelay + Bdelay * m_input; // m_input is 5×1
        Eigen::VectorXd mdelay = Cdelay * xdelay + Ddelay * m_input;
        i += 15; // Move to next state variables

        // Extract delayed signals
        mDelta_d = mdelay(0);
        mDelta_q = mdelay(1);
        mSigma_d = mdelay(2);
        mSigma_q = mdelay(3);
        mSigma_z = mdelay(4);
    }

    // Assuming state x = [iΔd, iΔq, iΣd, iΣq, iΣz, vCΔd, vCΔq, vCΔZd, vCΔZq, vCΣd, vCΣq, vCΣz]
    // Indices:      0     1     2     3     4      5       6       7        8       9       10      11
    // 
    // Reconstruct modulation voltages
	double vMDelta_d, vMDelta_q, vMDelta_Zd, vMDelta_Zq; // Modulation voltages for Δd, Δq, Zd, Zq
	double vMSigma_d, vMSigma_q, vMSigma_z; // Modulation voltages for Σd, Σq, Σz

    
   // Voltage vMDeltadqz
    vMDelta_d = (mDelta_q * vCSigma_q) / 4 - (mDelta_d * vCSigma_z) / 2 - (mDelta_d * vCSigma_d) / 4 - (mDelta_Zd * vCSigma_d) / 4 
        + (mDelta_Zq * vCSigma_q) / 4 - (mSigma_d * vCDelta_d) / 4 - (mSigma_z * vCDelta_d) / 2 + (mSigma_q * vCDelta_q) / 4 - (mSigma_d * vCDelta_Zd) / 4 + (mSigma_q * vCDelta_Zq) / 4;
    vMDelta_q = (mDelta_d * vCSigma_q) / 4 + (mDelta_q * vCSigma_d) / 4 - (mDelta_q * vCSigma_z) / 2 - (mDelta_Zd * vCSigma_q) / 4 
        - (mDelta_Zq * vCSigma_d) / 4 + (mSigma_d * vCDelta_q) / 4 + (mSigma_q * vCDelta_d) / 4 - (mSigma_z * vCDelta_q) / 2 - (mSigma_d * vCDelta_Zq) / 4 - (mSigma_q * vCDelta_Zd) / 4;
    vMDelta_Zd = -(mDelta_d * vCSigma_d) / 4 - (mDelta_q * vCSigma_q) / 4 - (mDelta_Zd * vCSigma_z) / 2 - (mSigma_d * vCDelta_d) / 4 - (mSigma_q * vCDelta_q) / 4 - (mSigma_z * vCDelta_Zd) / 2;
    vMDelta_Zq = (mDelta_d * vCSigma_q) / 4 - (mDelta_q * vCSigma_d) / 4 - (mDelta_Zq * vCSigma_z) / 2 - (mSigma_d * vCDelta_q) / 4 + (mSigma_q * vCDelta_d) / 4 - (mSigma_z * vCDelta_Zq) / 2;

    // Modulated Voltage vMSigmadqz
    vMSigma_d = (mDelta_d * vCDelta_d) / 4 - (mDelta_q * vCDelta_q) / 4 + (mDelta_d * vCDelta_Zd) / 4 + (mDelta_Zd * vCDelta_d) / 4 
        + (mDelta_q * vCDelta_Zq) / 4 + (mDelta_Zq * vCDelta_q) / 4 + (mSigma_d * vCSigma_z) / 2 + (mSigma_z * vCSigma_d) / 2;
    vMSigma_q = (mDelta_q * vCDelta_Zd) / 4 - (mDelta_q * vCDelta_d) / 4 - (mDelta_d * vCDelta_Zq) / 4 - (mDelta_d * vCDelta_q) / 4 
        + (mDelta_Zd * vCDelta_q) / 4 - (mDelta_Zq * vCDelta_d) / 4 + (mSigma_q * vCSigma_z) / 2 + (mSigma_z * vCSigma_q) / 2;
    vMSigma_z = (mDelta_d * vCDelta_d) / 4 + (mDelta_q * vCDelta_q) / 4 + (mDelta_Zd * vCDelta_Zd) / 4 + (mDelta_Zq * vCDelta_Zq) / 4 
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

    F(i++) = diDeltad_dt; F(i++) = diDeltaq_dt; F(i++) = diSigmad_dt; F(i++) = diSigmaq_dt; F(i++) = diSigmaz_dt;
    F(i++) = dvCDeltad_dt; F(i++) = dvCDeltaq_dt; F(i++) = dvCDeltaZd_dt; F(i++) = dvCDeltaZq_dt;
    F(i++) = dvCSigmad_dt; F(i++) = dvCSigmaq_dt; F(i++) = dvCSigmaz_dt;

    return F;
}

// Computes the Jacobian matrices A and B numerically using finite differences
void MMC::computeJacobian(const Eigen::VectorXd& x0, const Eigen::VectorXd& u0) {
    // Bind the member function computeStateDerivatives as a lambda
    auto f = [&](const Eigen::VectorXd& x, const Eigen::VectorXd& u) {
        return computeStateDerivatives(x, u);
        };

    // Compute both A = ∂f/∂x and B = ∂f/∂u
    std::pair<Eigen::MatrixXd, Eigen::MatrixXd> jacobians = computeJacobians(x0, u0, f);
    Eigen::MatrixXd A = jacobians.first;
    Eigen::MatrixXd B = jacobians.second;

    // Store in class variables
    A_matrix = A;
    B_matrix = B;

    int num_states = A_matrix.cols();
    C_matrix = Eigen::MatrixXd::Zero(3, num_states);

    C_matrix(1, 0) = 1; 
    C_matrix(2, 1) = 1;  

    if (controls.count("dc")) {
        C_matrix(0, 4) = 3;  // corresponds to C[1,5] in Julia
    }
	else { // to repair the C matrix
        //C_matrix(0, vdc_position - 1) = 1; 
    }

    D_matrix = Eigen::MatrixXd::Zero(3, 3);
}

// Solves f(x,u) = 0 for a steady-state operating point x, using Newton-Raphson
void MMC::solveEquilibrium() {
    const int n = number_of_states;

    // Initial guess
    Eigen::VectorXd x0 = 0.001 * Eigen::VectorXd::Ones(n);
    x0(n - 1) = V_dc;

    // Define input vector u (DC voltage and AC voltages)
    Eigen::VectorXd u(3);
    u << V_dc, V_m* cos(omega_0), -V_m * sin(omega_0);

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



Eigen::MatrixXcd MMC::compute_y_parameters_num(double omega) {
    // s: Laplace variable (jω)
	std::complex<double> s_num = std::complex<double>(0, omega);
    const int n = A_matrix.rows();
    Eigen::MatrixXcd I = Eigen::MatrixXcd::Identity(n, n);
    Eigen::MatrixXcd A_s = s_num * I - A_matrix.cast<std::complex<double>>();
    Eigen::MatrixXcd inv_A_s = A_s.inverse();

    Eigen::MatrixXcd B_c = B_matrix.cast<std::complex<double>>();
    Eigen::MatrixXcd C_c = C_matrix.cast<std::complex<double>>();
    Eigen::MatrixXcd D_c = D_matrix.cast<std::complex<double>>();

    return C_c * inv_A_s * B_c + D_c;
}

// System analysis
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

void MMC::printEigenvalues() const {
    Eigen::EigenSolver<Eigen::MatrixXd> es(A_matrix);
    std::cout << "Eigenvalues:\n" << es.eigenvalues() << "\n";
}

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
        std::cout << "  Controller: " << controllerName << "\n";
        controller->printValues(); // Print controller values
    }
}