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
                controls[controller_name] = new Controller(controller_name, controller_type, values, number_of_values);
                i += 4;
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

void MMC::init_MMC() {
    // Handle initial references for OCC and CCC
    //if (controls.count("occ")) {
    //    Controller* occ = controls["occ"];
    //    if (occ->ref.size() == 1 && occ.ref(0) == 0) {
    //        occ.ref = Eigen::Vector2d(Id, Iq);
    //    }
    //}

    //if (c.controls.count("ccc")) {
    //    Controller& ccc = c.controls["ccc"];
    //    if (ccc.ref.size() == 1 && ccc.ref(0) == 0) {
    //        ccc.ref = Eigen::Vector2d(0.0, 0.0);
    //    }
    //}
}

void MMC::update_MMC(double Vm, double theta, double Pac, double Qac, double Vdc, double Pdc) {
    this->V_m = Vm;
    this->theta = theta;
    this->V_dc = Vdc;
    this->P = Pac;
    this->Q = Qac;
    this->P_dc = Pdc;
}

Eigen::MatrixXd MMC::computeStateDerivatives(const Eigen::VectorXd& x0, const Eigen::VectorXd& u) {
    double Le = L_arm / 2.0 + L_reactor;
    double Re = R_arm / 2.0 + R_reactor;
    double Ce = 6.0 * C_arm / N;

    double Vgd = V_m * cos(theta);
    double Vgq = -V_m * sin(theta);

    double Id = (2.0 / 3.0) * (Vgd * P + Vgq * Q) / (Vgd * Vgd + Vgq * Vgq);
    double Iq = (2.0 / 3.0) * (Vgq * P - Vgd * Q) / (Vgd * Vgd + Vgq * Vgq);
    //assert(x.size() == 6 && u.size() == 8);

    //// Extract states variable
    //const double ip1 = x(0), ip2 = x(1), ip3 = x(2);   //positive arm
    //const double in1 = x(3), in2 = x(4), in3 = x(5);   //negative arm

    //// 2nd harmonic injection modulation
    //const double s1 = std::sin(2 * w * t + phi1);
    //const double s2 = std::sin(2 * w * t + phi2);
    //const double s3 = std::sin(2 * w * t - phi3); 

    //// Inject harmonics into upper and lower arm currents
    //const double ip1_mod = ip1 + A_harm * s1;
    //const double ip2_mod = ip2 + A_harm * s2;
    //const double ip3_mod = ip3 + A_harm * s3;
    //const double in1_mod = in1 + B_harm * s1;
    //const double in2_mod = in2 + B_harm * s2;
    //const double in3_mod = in3 + B_harm * s3; 

    //// Extract voltage inputs
    //const double VD1 = u(0), VD2 = u(1);
    //const double VS1 = u(2), VS2 = u(3), VS3 = u(4);
    //const double VS4 = u(5), VS5 = u(6), VS6 = u(7);
    //
    //// Compute differential equations (nonlinear)
    //Eigen::VectorXd dx(6);
    //dx(0) = (VD1 - VS1 - ip1_mod * Rp + ip1_mod * Rm + in1_mod * Rn) / L;
    //dx(1) = (VD1 - VS2 - ip2_mod * Rp + ip2_mod * Rm + in2_mod * Rn) / L;
    //dx(2) = (VD1 - VS3 - ip3_mod * Rp + ip3_mod * Rm + in3_mod * Rn) / L;
    //dx(3) = (VS4 - VD2 - in1_mod * Rn - in1_mod * Rm - ip1_mod * Rm) / L;
    //dx(4) = (VS5 - VD2 - in2_mod * Rn - in2_mod * Rm - ip2_mod * Rm) / L;
    //dx(5) = (VS6 - VD2 - in3_mod * Rn - in3_mod * Rm - ip3_mod * Rm) / L;

    Eigen::VectorXd dx(3);
    dx << 0, 0, 0;

    return dx;
}

Eigen::VectorXd MMC::computeStateDerivativesLinear(const Eigen::VectorXd& x, const Eigen::VectorXd& u) {
    return A_matrix * x + B_matrix * u;
}

// Computes Jacobian matrices A = ∂f/∂x and B = ∂f/∂u numerically
// x0, u0: Operating point vectors for state and input
Eigen::MatrixXd MMC::computeJacobianNumerically(const Eigen::VectorXd& x0, const Eigen::VectorXd& u0) {
    const double eps = 1e-6;
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
    const int max_iter = 50;
    const double tol = 1e-8;
    double t = 5.0;

    const int n = 6;  // Only the dynamic states
    Eigen::VectorXd x = Eigen::VectorXd::Zero(n);

    // Operating point input voltages
    Eigen::VectorXd u(8);
    u << 0, 0, 400, 400, -400, 400, -400, 400;

    for (int iter = 0; iter < max_iter; ++iter) {
        Eigen::VectorXd f_val = computeStateDerivatives(x, u);
        double norm_f = f_val.norm();
        if (norm_f < tol) {
            std::cout << "Equilibrium found in " << iter << " iterations. Residual = " << norm_f << "\n";
            break;
        }

        // Compute numerical Jacobian df/dx
        Eigen::MatrixXd J = Eigen::MatrixXd::Zero(n, n);
        double eps = 1e-6;
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

    equilibrium_state = Eigen::VectorXd::Zero(13);  // match 13-d state
    equilibrium_state.head(6) = x;  // store 6-state equilibrium

    // equilibrium_state = Eigen::VectorXd::Zero(13);
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