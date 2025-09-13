#include "ProportionalIntegral_controller.h"

/**
 * @brief Update controller parameters (if dynamic change is needed).
 *
 * @param newKp New proportional gain.
 * @param newKi New integral gain.
 * @param newZeta New damping ratio.
 * @param newBandwidth New bandwidth (Hz).
 */
void ProportionalIntegralController::updateParameters(double newKp, double newKi, double newZeta, double newBandwidth) {
    Kp = newKp;
    Ki = newKi;
    zeta = newZeta;
    bandwidth = newBandwidth;
    std::cout << "Controller parameters updated: \n"
        << "  New Proportional Gain (Kp): " << Kp << "\n"
        << "  New Integral Gain (Ki): " << Ki << "\n"
        << "  New Damping Ratio (ζ): " << zeta << "\n"
        << "  New Bandwidth: " << bandwidth << " Hz\n";
}

/**
 * @brief Defines the differential equations for a vector PI controller.
 * 
 * This function computes the error between the reference and input, updates the integrator state,
 * and calculates the PI controller output for each channel. The returned vector concatenates the
 * state derivatives and controller outputs.
 * 
 * @param x Integrator state vector.
 * @param u Input (measured) vector.
 * @param c Constant/reference output signal vector.
 * @return Eigen::VectorXd Concatenated vector of state derivatives and controller outputs.
 */
Eigen::VectorXd ProportionalIntegralController::define_equations(const Eigen::VectorXd& x, const Eigen::VectorXd& u, const Eigen::VectorXd& c) {
    Eigen::MatrixXd dx_dt(reference.size(), 1); // Error signal (state derivative)
    Eigen::MatrixXd output(reference.size(), 1); // PI controller output
    for (int i = 0; i < reference.size(); ++i) {
        dx_dt(i) = (reference[i] - u(i));
        output(i) = Kp * dx_dt(i) + Ki * x(i) + c(i);
    }
    Eigen::VectorXd state_derivative = Eigen::VectorXd::Zero(x.size() + output.size());
    state_derivative << dx_dt, output;
    return state_derivative;
}

/**
 * @brief Defines the differential equations for a scalar PI controller.
 * 
 * This function computes the error between the reference and input, updates the integrator state,
 * and calculates the PI controller output. The returned vector contains the state derivative and controller output.
 * 
 * @param x Integrator state (scalar).
 * @param u Input (measured) value.
 * @param c Constant/reference output signal.
 * @return Eigen::VectorXd Vector containing state derivative and controller output.
 */
Eigen::VectorXd ProportionalIntegralController::define_equations(const double x, const double u, const double c) {
    double dx_dt = (reference[0] - u);
    double output = Kp * dx_dt + Ki * x + c;
    Eigen::VectorXd state_derivative = Eigen::VectorXd::Zero(2);
    state_derivative << dx_dt, output;
    return state_derivative;
}