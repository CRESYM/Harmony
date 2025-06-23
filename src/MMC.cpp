#include "MMC.h"
#include "Controller.h"
#include "Filter.h"
#include <memory>
#include <stdexcept>
#include <vector>
#include <Eigen/Dense>
#include <cmath>
#include <iostream>

// Static method to initialize the MMC with converter parameters
MMC MMC::init_MMC(const std::vector<double>& converter_params) {
    return MMC(
        "MMC", 100, 10,                            // symbol, inputPins, outputPins
        converter_params[0],                       // omega
        converter_params[1],                       // activePower
        converter_params[2],                       // reactivePower
        converter_params[3],                       // dcPower
        converter_params[4],                       // minActivePower
        converter_params[5],                       // maxActivePower
        converter_params[6],                       // minReactivePower
        converter_params[7],                       // maxReactivePower
        converter_params[8],                       // angle
        converter_params[9],                       // acVoltage
        converter_params[10],                      // dcVoltage
        converter_params[11],                      // armInductance
        converter_params[12],                      // armResistance
        converter_params[13],                      // armCapacitance
        static_cast<int>(converter_params[14]),    // numSubmodules
        converter_params[15],                      // reactorInductance
        converter_params[16],                      // reactorResistance
        converter_params[17]                       // timeDelay
    );  // Return the fully initialized MMC instance
}

// Initialize the controller(s) in MMC using provided parameters
void MMC::init_Controller(const std::vector<double>& converter_params) {
    // Ensure that there are enough parameters for Controller
    if (converter_params.size() < 27) {
        throw std::invalid_argument("Insufficient converter parameters for Controller initialization.");
    }
    //Add a default controller using parameters
    auto controller = std::make_shared<Controller>(
        "Controller1",                         // Symbol for controller
        static_cast<int>(converter_params[18]), // Pins
        converter_params[19],                  // Proportional Gain (Kp)
        converter_params[20],                  // Integral Gain (Ki)
        converter_params[21],                  // Damping Ratio (zeta)
        converter_params[22],                  // Bandwidth
        std::vector<double>(converter_params.begin() + 23, converter_params.end())  // Reference vector (using the rest of the values)
    );
    // Add the controller to the 'controls' map
    //controls["Controller1"] = controller;
    controls[controller->getElementSymbol()] = controller;  // Add filter to the map
}

// Initialize the filter(s) in MMC using provided parameters
void MMC::init_Filter(const std::vector<double>& converter_params) {
    if (converter_params.size() < 27) {
        throw std::invalid_argument("Insufficient converter parameters for Filter initialization.");
    }
    // Add a default filter using parameters
    auto filter = std::make_shared<Filter>(
        "Filter1",                            // Symbol for filter
        static_cast<int>(converter_params[23]), // Pins (example value, adjust accordingly)
        converter_params[24],                  // Time Constant (T)
        converter_params[25],                  // Damping Ratio (zeta)
        converter_params[26]                   // Bandwidth
    );

    //filters["Filter1"] = filter;  // Add filter to the map
    filters[filter->getElementSymbol()] = filter;  // Add filter to the map
}
void MMC::update_MMC(double Vm, double theta, double Pac, double Qac, double Vdc, double Pdc) {
    this->V_m = Vm;
    this->theta = theta;
    this->V_dc = Vdc;
    this->P = Pac;
    this->Q = Qac;
    this->P_dc = Pdc;
}

void MMC::setRarmPositive(double val) { R_arm = val; }
void MMC::setRarmNegative(double val) { Rn_custom = val; }
void MMC::setRarmMutual(double val) { Rm_custom = val; }

void MMC::setSecondHarmonicInjection(double A, double B, double ph1, double ph2, double ph3) {
    A_harm = A;
    B_harm = B;
    phi1 = ph1;
    phi2 = ph2;
    phi3 = ph3;
}

Eigen::MatrixXd MMC::computeStateDerivatives(const Eigen::VectorXd& x, const Eigen::VectorXd& u) {
    assert(x.size() == 6 && u.size() == 8);
    
    const double L = L_arm;
    const double Rp = R_arm;
    const double Rn = R_arm;
    const double Rm = 10.0;

    double t = 5.0; // Operating time (5s) 
    const double w = omega_0;

    // Extract states variable
    const double ip1 = x(0), ip2 = x(1), ip3 = x(2);   //positive arm
    const double in1 = x(3), in2 = x(4), in3 = x(5);   //negative arm

    // 2nd harmonic injection modulation
    const double s1 = std::sin(2 * w * t + phi1);
    const double s2 = std::sin(2 * w * t + phi2);
    const double s3 = std::sin(2 * w * t - phi3); 

    // Inject harmonics into upper and lower arm currents
    const double ip1_mod = ip1 + A_harm * s1;
    const double ip2_mod = ip2 + A_harm * s2;
    const double ip3_mod = ip3 + A_harm * s3;
    const double in1_mod = in1 + B_harm * s1;
    const double in2_mod = in2 + B_harm * s2;
    const double in3_mod = in3 + B_harm * s3; 

    // Extract voltage inputs
    const double VD1 = u(0), VD2 = u(1);
    const double VS1 = u(2), VS2 = u(3), VS3 = u(4);
    const double VS4 = u(5), VS5 = u(6), VS6 = u(7);
    
    // Compute differential equations (nonlinear)
    Eigen::VectorXd dx(6);
    dx(0) = (VD1 - VS1 - ip1_mod * Rp + ip1_mod * Rm + in1_mod * Rn) / L;
    dx(1) = (VD1 - VS2 - ip2_mod * Rp + ip2_mod * Rm + in2_mod * Rn) / L;
    dx(2) = (VD1 - VS3 - ip3_mod * Rp + ip3_mod * Rm + in3_mod * Rn) / L;
    dx(3) = (VS4 - VD2 - in1_mod * Rn - in1_mod * Rm - ip1_mod * Rm) / L;
    dx(4) = (VS5 - VD2 - in2_mod * Rn - in2_mod * Rm - ip2_mod * Rm) / L;
    dx(5) = (VS6 - VD2 - in3_mod * Rn - in3_mod * Rm - ip3_mod * Rm) / L;

    return dx;
}

Eigen::VectorXd MMC::computeStateDerivativesLinear(const Eigen::VectorXd& x, const Eigen::VectorXd& u) {
    return A_matrix * x + B_matrix * u;
}

// Jacobian calculations
void MMC::computeJacobianLinear() {
    double L = L_arm;
    const double Rp = R_arm;
    const double Rn = Rn_custom;
    const double Rm = Rm_custom;

    // A matrix entries
    double A11 = -(Rp + Rm) / L;
    double A14 = -Rm / L;
    double A12 = -(Rm + Rn) / L;

    A_matrix << A11, 0, 0, A14, 0, 0,
        0, A11, 0, 0, A14, 0,
        0, 0, A11, 0, 0, A14,
        A14, 0, 0, A12, 0, 0,
        0, A14, 0, 0, A12, 0,
        0, 0, A14, 0, 0, A12;

    // B matrix 
    double Bval = 1.0 / L;
    B_matrix << Bval, 0.0, -Bval, 0.0, 0.0, 0.0, 0.0, 0.0,
                Bval, 0.0, 0.0, -Bval, 0.0, 0.0, 0.0, 0.0,
                Bval, 0.0, 0.0, 0.0, -Bval, 0.0, 0.0, 0.0,
                0.0, -Bval, 0.0, 0.0, 0.0, Bval, 0.0, 0.0,
                0.0, -Bval, 0.0, 0.0, 0.0, 0.0, Bval, 0.0,
                0.0, -Bval, 0.0, 0.0, 0.0, 0.0, 0.0, Bval;
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

//void MMC::computeJacobian() {
//    A_matrix = Eigen::MatrixXd::Zero(13, 13);
//    B_matrix = Eigen::MatrixXd::Zero(13, 3);
//}

Eigen::MatrixXcd MMC::computeAdmittanceMatrix(std::complex<double> s) {
    // s: Laplace variable (jω)
    const int n = A_matrix.rows();
    Eigen::MatrixXcd I = Eigen::MatrixXcd::Identity(n, n);
    Eigen::MatrixXcd A_s = s * I - A_matrix.cast<std::complex<double>>();
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