#ifndef _INTEGRATOR_H_
#define _INTEGRATOR_H_

#include "Control_block.h"

/**
 * @file Integrator.h
 * @brief Integrator control block (state derivative equals input error).
 * @ingroup control
 */

/**
 * @brief Pure integrator: @f$ \dot{x} = u @f$ for scalar or vector signals.
 */
class Integrator : public ControlBlock {
public:
	Integrator()  {} // Default constructor

	/**
	 * @brief Vector integrator dynamics.
	 * @param x Error (or input) vector.
	 * @return State derivative vector equal to @p x.
	 */
	Eigen::VectorXd define_differential_equations(const Eigen::VectorXd& x);

	/**
	 * @brief Scalar integrator dynamics.
	 * @param x Error (or input) value.
	 * @return State derivative equal to @p x.
	 */
	double define_differential_equations(const double x);

	~Integrator() = default; // Default destructor

private:
	
};

#endif
