#include "MMC.h"
#include "Controller.h"
#include "Filter.h"


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
                controls[controller_name] = new Controller(controller_name, controller_type, values, number_of_values, refs);
                i += 4 + number_of_values;
            }
            else {
                i += 1; // Skip to the next controller
			}
		}	
	}
}

// Initialize the filter(s) in MMC using provided parameters
void MMC::init_Filter(const std::vector<double>& filter_params) {
    for (int i = 0; i < filter_params.size(); ) {
        for (auto& filter_name : filter_list) {
            if (static_cast<bool>(filter_params[i])) {
                if ((i + 4) >= filter_params.size()) {
                    throw std::invalid_argument("Insufficient parameters for filter initialization.");
                }
                std::string filter_type = "LP"; // Default controller type
                std::vector<double> values = { filter_params[i + 1], filter_params[i + 2], filter_params[i+3]};
                int number_of_values = static_cast<int>(filter_params[i + 4]); // Default number of values for PI controller
                filters[filter_name] = new Filter(filter_name, filter_type, values, number_of_values);
                i += 4;
            }
            else {
                i += 1; // Skip to the next controller
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

Eigen::MatrixXd MMC::computeStateDerivatives(const Eigen::VectorXd& x, const Eigen::VectorXd& u) {
    double Leqac = L_arm / 2.0 + L_reactor;
    double Reqac = R_arm / 2.0 + R_reactor;
    double Ce = 6.0 * C_arm / N;

    // Constants for now
	double Vdc = V_dc; // DC voltage
	double w = omega_0; // Angular frequency (rad/s)
	double Vgd = V_m * std::cos(theta); // d-axis voltage
	double Vgq = -V_m * std::sin(theta); // q-axis voltage

    int number_of_states = x.size();
    Eigen::VectorXd F = Eigen::VectorXd::Zero(number_of_states);

	// Extract state variables from the end of the state vector
	int i = x.size() - 12; // Assuming the last 12 elements are the state variables
    double iDelta_d = x(i), iDelta_q = x(i + 1), iSigma_d = x(i + 2), iSigma_q = x(i + 3), iSigma_z = x(i + 4);
    double vCDelta_d = x(i + 5), vCDelta_q = x(i + 6), vCDelta_Zd = x(i + 7), vCDelta_Zq = x(i + 8);
    double vCSigma_d = x(i + 9), vCSigma_q = x(i + 10), vCSigma_z = x(i + 11);

    // Extract control reference voltages (user must assign these before this call)
    double vMDelta_d_ref = 0, vMDelta_q_ref = 0, vMDelta_Zd_ref = 0, vMDelta_Zq_ref = 0;
	double vMSigma_d_ref = 0, vMSigma_q_ref = 0, vMSigma_z_ref = 0;

    //Eigen::VectorXd xdelay = x.segment(index, 15); // 3rd order Padé for 5 signals

    //// Delay update equations
    //F.segment(index, 15) = Adelay * xdelay + Bdelay * m_input;
    //Eigen::VectorXd mdelay = Cdelay * xdelay + Ddelay * m_input;

    //// Extract delayed modulation signals
    //double mDelta_d = mdelay(0);
    //double mDelta_q = mdelay(1);
    //double mSigma_d = mdelay(2);
    //double mSigma_q = mdelay(3);
    //double mSigma_z = mdelay(4);

    i = 0;
	// Placeholders for values used in control loops
    Eigen::VectorXd state_variables; // Placeholder for state variables
    Eigen::VectorXd x1 = Eigen::VectorXd(2);
    Eigen::VectorXd u1 = Eigen::VectorXd(2);
    Eigen::VectorXd c1 = Eigen::VectorXd(2);
	double x2 = 0; // Placeholder for second state variable

	// LOOPS ARE ADDED IN FORMAT: outer loop that creates reference for inner loop and then inner loop
    // 
    // OUTER LOOPS
	// Adding energy control loop
    if (controls.count("energy")) {
        double Pac = (3 / 2) * (Vgd * iDelta_d + Vgq * iDelta_q);
        double wSigmaz = 3 * (C_arm * (pow(vCDelta_d,2) + pow(vCDelta_q,2) + pow(vCDelta_Zd,2) 
            + pow(vCDelta_Zq,2) + pow(vCSigma_d,2) + pow(vCSigma_q,2) + 2 * pow(vCSigma_z,2))) / (2 * N);
		controls["energy"]->setReference(3.0 * C_arm * Vdc * Vdc / N, 0);
		//std::cout << controls["energy"]->getReference() << endl;

		x2 = x(i); // integral of energy error
        state_variables = controls["energy"]->define_differential_equations(x(i), wSigmaz, Pac);
        F(i) = state_variables(0);
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
        double Pac = (3 / 2) * (Vgd * iDelta_d + Vgq * iDelta_q);
        x2 = x(i); // integral of active power error
        u1 << Pac; // Pac
        c1 << 0; // Vdc
        state_variables = controls["active_power"]->define_differential_equations(x1, u1, c1);
		F(i) = state_variables(0);
		double iDelta_d_ref = state_variables(1);
		i += 1; // Move to next state variables

		if (controls.count("occ"))
            controls["occ"]->setReference(iDelta_d_ref, 0); // Set reference for outer control loop
		else
			throw std::runtime_error("Outer current control loop is not available.");
    }
	// Adding DC voltage control loop
    else if (controls.count("dc_voltage")) {
		double Idc = P_dc / Vdc; // DC current
        double Vdc = x(i); // DC voltage
        x2 = x(i); // integral of DC voltage error
        u1 << Vdc; // Vdc
        c1 << P_dc; // Pdc
        state_variables = controls["dc_voltage"]->define_differential_equations(x1, u1, c1);

		i += 2; // Move to next state variables
    }

	// Adding reactive power control loop
    if (controls.count("reactive_power")) {
        double Qac = (3 / 2) * (Vgd * iDelta_q - Vgq * iDelta_d);
        x2 = x(i); // integral of reactive power error
        state_variables = controls["reactive_power"]->define_differential_equations(x2, Qac, 0);
        F(i) = state_variables(0);
        double iDelta_q_ref = -state_variables(1);
        i += 1; // Move to next state variables

        if (controls.count("occ"))
            controls["occ"]->setReference(iDelta_q_ref, 1); // Set reference for outer control loop
        else
            throw std::runtime_error("Outer current control loop is not available.");
    }
	// Adding AC voltage control loop
    else if (controls.count("ac_voltage")) {
        x2 = x(i); // integral of AC voltage error
        u1 << Vgd, Vgq; // Vgd, Vgq
        c1 << P, Q; // P, Q
        state_variables = controls["ac_voltage"]->define_differential_equations(x1, u1, c1);
	}

	// Adding zero current control loop that gets reference from energy control loop, or given
    if (controls.count("zcc"))
    {
        double x2 = x(i);
        state_variables = controls["zcc"]->define_differential_equations(x2, iSigma_z, (-Vdc / 2));
        F(i) = state_variables(0);
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
        F(i) = state_variables(0);
        F(i + 1) = state_variables(1);
        vMDelta_d_ref = state_variables(2);
        vMDelta_q_ref = state_variables(3);
        i += 2; // Move to next state variables
    }

    if (controls.count("ccc")) {
        x1 << x(i), x(i + 1); // Initialize x1 with the first two state variables
        u1 << iSigma_d, iSigma_q; // Initialize u1 with the first two state variables
        c1 << -2 * w * L_arm * iSigma_q, 2 * w * L_arm * iSigma_d; // Initialize c1 with the voltage references
        state_variables = controls["ccc"]->define_differential_equations(x1, u1, c1);
        F(i) = state_variables(0);
        F(i + 1) = state_variables(1);
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

// Computes Jacobian matrices A = ∂f/∂x and B = ∂f/∂u numerically
// x0, u0: Operating point vectors for state and input
Eigen::MatrixXd MMC::computeJacobianNumerically(const Eigen::VectorXd& x0, const Eigen::VectorXd& u0) {
    const double eps = 1e-3;
    const int n = x0.size();
    const int m = u0.size();

    Eigen::MatrixXd A = Eigen::MatrixXd::Zero(n, n);
    Eigen::MatrixXd B = Eigen::MatrixXd::Zero(n, m);
    
    Eigen::VectorXd f0 = computeStateDerivatives(x0, u0); // f(x0, u0)

    // Compute ∂f/∂x
    for (int i = 0; i < n; ++i) {
        Eigen::VectorXd x_pert = x0;
        x_pert(i) += eps;
        A.col(i) = (computeStateDerivatives(x_pert, u0) - f0) / eps;
    }

    for (int i = 0; i < m; ++i) {
        Eigen::VectorXd u_pert = u0;
        u_pert(i) += eps;
        B.col(i) = (computeStateDerivatives(x0, u_pert) - f0) / eps;
    }

    A_matrix = A;
    B_matrix = B;

    return A;
}

// Solves f(x,u) = 0 for a steady-state operating point x, using Newton-Raphson
void MMC::solveEquilibrium() {
    const int max_iter = 1000;
    const double tol = 1e-9;

    const int n = number_of_states;  // Only the dynamic states
    Eigen::VectorXd x = 0.001 * Eigen::VectorXd::Ones(n);
	x(number_of_states - 1) = V_dc; // Set the last state to zero (e.g., DC voltage)

    // Operating point input voltages
    Eigen::VectorXd u(1);
    u << 0;

    for (int iter = 0; iter < max_iter; ++iter) {
        Eigen::VectorXd f_val = computeStateDerivatives(x, u);
        double norm_f = f_val.norm();
        if (norm_f < tol) {
            std::cout << "Equilibrium found in " << iter << " iterations. Residual = " << norm_f << "\n";
            break;
        }

        // Compute numerical Jacobian df/dx
        Eigen::MatrixXd J = Eigen::MatrixXd::Zero(n, n);
        double eps = 1e-9;
        for (int i = 0; i < n; ++i) {
            Eigen::VectorXd x_eps = x;
            x_eps(i) += eps;
            J.col(i) = (computeStateDerivatives(x_eps, u) - f_val) / eps;
        }

        // Solve for Newton step
        Eigen::VectorXd dx = J.fullPivLu().solve(-f_val);
        x += dx;

        if (dx.norm() < tol) {
            std::cout << "Converged (Newton step small) in " << iter << " iterations.\n";
            break;
        }

        if (iter == max_iter - 1) {
            std::cerr << "WARNING: Equilibrium solver did not converge.\n";
        }
    }

    equilibrium_state = Eigen::VectorXd::Zero(number_of_states);  // match 13-d state
    equilibrium_state.head(number_of_states) = x;  // store equilibrium
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