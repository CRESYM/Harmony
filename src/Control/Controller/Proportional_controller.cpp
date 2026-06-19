/**
 * @file Proportional_controller.cpp
 * @brief Implementation of Proportional (P) controller.
 */
#include "Proportional_controller.h"

/**
 * @brief Defines the equations for a vector P controller.
 *
 * This function computes the error between the reference and input.
 * The returned vector concatenates the states and controller outputs.
 *
 * @param x Input state vector.
 * @param u Input (measured) vector. Left empty for proportional controller.
 * @param c Constant/reference output signal vector.
 * @return Eigen::VectorXd Concatenated vector of states and controller outputs.
 */
Eigen::VectorXd ProportionalController::define_equations(const Eigen::VectorXd& x, const Eigen::VectorXd& u, const Eigen::VectorXd& c) {
    Eigen::MatrixXd dx(reference.size(), 1); // Error signal (state derivative)
    Eigen::MatrixXd output(reference.size(), 1); // P controller output
    for (int i = 0; i < reference.size(); ++i) {
        dx(i) = (reference[i] - u(i));
        output(i) = K * dx(i) + c(i);
    }
    Eigen::VectorXd state = Eigen::VectorXd::Zero(x.size() + output.size());
    state << dx, output;
    return state;
}

/**
 * @brief Defines the equations for a scalar P controller.
 *
 * This function computes the error between the reference and input. 
 * The returned vector contains the state derivative and controller output.
 *
 * @param x Integrator state (scalar).
 * @param u Input (measured) value. Not used in proportional controller.
 * @param c Constant/reference output signal.
 * @return Eigen::VectorXd Vector containing states and controller output.
 */
Eigen::VectorXd ProportionalController::define_equations(const double x, const double u, const double c) {
    double dx = (reference[0] - u);
    double output = K * dx + c;
    Eigen::VectorXd state = Eigen::VectorXd::Zero(2);
    state << dx, output;
    return state;
}