#include "Filter.h"

void Filter::updateParameters(double newTimeConstant, double newZeta, double newBandwidth) {
    timeConstant = newTimeConstant;
    zeta = newZeta;
    bandwidth = newBandwidth;
}

VectorXd Filter::define_differential_equations(const Eigen::VectorXd& x, const Eigen::VectorXd& u) {
    if (filter_order == 1) {
        // First-order filter differential equation
        VectorXd dx_dt(1 * filter_size), output(1 * filter_size);
        for (int i = 0; i < filter_size; ++i) {
            dx_dt(i) = u(i) - x(i) / timeConstant; // Example: dx/dt = (input - output) / T
			output(i) = x(i) / timeConstant; // Example: output = damping * dx/dt + state
		}
        // Merge the output into the state vector
        VectorXd state_derivative = Eigen::VectorXd::Zero(dx_dt.size() + output.size());
        state_derivative << dx_dt, output;

        return state_derivative;
    } 
    else if (filter_order == 2) {
        // Second-order filter differential equation
        VectorXd dx_dt(2 * filter_size), output(filter_size);
        for (int i = 0; i < filter_size; ++i) {
			dx_dt(i) = -bandwidth * x(i+1) - 2 * zeta * bandwidth * x(i) + u(i); // Example: dx/dt = -ω_n * x - ζ * u
            dx_dt(i + 1) = x(i);
            output(i) = pow(bandwidth,2) * x(i+1); // Example: output = damping * dx/dt + state
        }
        // Merge the output into the state vector
        VectorXd state_derivative = Eigen::VectorXd::Zero(dx_dt.size() + output.size());
        state_derivative << dx_dt, output;

        return state_derivative;
    } else {
        throw std::invalid_argument("Unsupported filter order. Only 1st and 2nd order filters are implemented.");
	}
}

VectorXd Filter::define_differential_equations(const VectorXd& x, const double u) {
	// This can only occur when there is a single input to the filter, i.e. filter_size = 1
    // and the filter order is 2.
    if (filter_order != 2) {
        throw std::invalid_argument("This method is only applicable for 2nd order filters with a single input.");
	}
    else if (filter_size != 1) {
        throw std::invalid_argument("This method is only applicable for filters with a single input.");
    }
    // Second-order filter differential equation
    VectorXd dx_dt(2), output(1);
    dx_dt(0) = -bandwidth * x(1) - 2 * zeta * bandwidth * x(0) + u; // Example: dx/dt = -ω_n * x - ζ * u
    dx_dt(1) = x(0);
    output(0) = pow(bandwidth, 2) * x(1); // Example: output = damping * dx/dt + state
    
    // Merge the output into the state vector
    VectorXd state_derivative = Eigen::VectorXd::Zero(dx_dt.size() + output.size());
    state_derivative << dx_dt, output;

    return state_derivative;

}

VectorXd Filter::define_differential_equations(const double x, const double u) {
    // This can only occur when there is a single input to the filter, i.e. filter_size = 1
    // and the filter order is 1.
    if (filter_order != 1) {
        throw std::invalid_argument("This method is only applicable for 2nd order filters with a single input.");
    }
    else if (filter_size != 1) {
        throw std::invalid_argument("This method is only applicable for filters with a single input.");
    }
    // First-order filter differential equation
    VectorXd dx_dt(1 * filter_size), output(1 * filter_size);
    dx_dt(0) = u - x / timeConstant; // Example: dx/dt = (input - output) / T
    output(0) = x / timeConstant; // Example: output = damping * dx/dt + state
    
    // Merge the output into the state vector
    VectorXd state_derivative = Eigen::VectorXd::Zero(dx_dt.size() + output.size());
    state_derivative << dx_dt, output;

    return state_derivative;
    
}