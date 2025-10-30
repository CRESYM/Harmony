#ifndef _INTEGRATOR_H_
#define _INTEGRATOR_H_

#include "Control_block.h"

class Integrator : public ControlBlock {
public:
	Integrator()  {} // Default constructor

	Eigen::VectorXd define_differential_equations(const Eigen::VectorXd& x);
	double define_differential_equations(const double x);

	~Integrator() = default; // Default destructor

private:
	
};

#endif
