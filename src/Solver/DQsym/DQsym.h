#ifndef _DQSYM_H_	
#define _DQSYM_H_

#include "../../Constants.h"

class DQsym
{
public:
	DQsym() = default;
	~DQsym() = default;

	// Mathematical operations on dynamic phasors
	MatrixXcd add(const MatrixXcd& a, const MatrixXcd& b);
	MatrixXcd subtract(const MatrixXcd& a, const MatrixXcd& b);
	MatrixXcd integrate(MatrixXcd& Zpnz_old, MatrixXcd& Xpnz_old, const MatrixXcd& Xpnz,
		double dt, double w);
	MatrixXcd multiply(const MatrixXcd& x_coef1_in, const MatrixXcd& y_coef1_in);
};


#endif // _DQSYM_H_