/**
 * @file Integrator.cpp
 * @brief Implementation of Integrator control block (state derivative equals input error).
 */
#include "Integrator.h"

// For scalar input: dx/dt = error
double Integrator::define_differential_equations(const double error) {
    // The derivative of the integrator state is the error signal
    return error;
}

// For vector input: dx/dt = error vector
Eigen::VectorXd Integrator::define_differential_equations(const Eigen::VectorXd& error) {
    // The derivative of the integrator state is the error signal vector
    return error;
}