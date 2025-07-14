#include "Controller.h"


// updateParameters: Update controller parameters (if dynamic change is needed)
void Controller::updateParameters(double newKp, double newKi, double newZeta, double newBandwidth) {
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


Eigen::VectorXd Controller::define_differential_equations(const Eigen::VectorXd& x, const Eigen::VectorXd& u, const Eigen::VectorXd& c) {
	// The function takes state-variables x and inputs u to define the differential equations for the controller.
    // And the constant to define the reference output signal.
	Eigen::MatrixXd dx_dt(reference.size(), 1); // Placeholder for the derivative of state variables
	Eigen::MatrixXd output(reference.size(), 1); // Placeholder for the output of the controller
    for (int i = 0; i < reference.size(); ++i) {
        // Example: Proportional-Integral control equation
        // dx/dt = Kp * (reference[i] - x[i]) + Ki * u[i];
        // Here, we would typically compute the derivative of the state variable.
        // This is a placeholder for the actual implementation.

		dx_dt(i) = (reference[i] - u(i));
		output(i) = Kp * dx_dt(i) + Ki * x(i) + c(i);
	}

	// Merge the output into the state vector
	Eigen::VectorXd state_derivative = Eigen::VectorXd::Zero(x.size() + output.size());
	state_derivative << dx_dt, output;

    // Return the state derivative
	return state_derivative;
}

Eigen::VectorXd Controller::define_differential_equations(const double x, const double u, const double c) {
    // The function takes state-variables x and inputs u to define the differential equations for the controller.
    // And the constant to define the reference output signal.
    double dx_dt; // Placeholder for the derivative of state variables
    double output; // Placeholder for the output of the controller
   
    dx_dt = (reference[0] - u);
    output = Kp * dx_dt + Ki * x + c;

    // Merge the output into the state vector
    Eigen::VectorXd state_derivative = Eigen::VectorXd::Zero(2);
    state_derivative << dx_dt, output;

    // Return the state derivative
    return state_derivative;
}