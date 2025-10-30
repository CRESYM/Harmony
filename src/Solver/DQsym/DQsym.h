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

	// Discrete state-space solver for dynamic phasors
	MatrixXcd DQsym::DSSS(const MatrixXcd& Ad, const MatrixXcd& Bd,
		const MatrixXcd& Cd, const MatrixXcd& Dd, const VectorXd& swOnRes, const VectorXd& swOffRes,
		const VectorXi& swType, const VectorXi& brkVec, const MatrixXcd& u, const VectorXcd& xo,
		double dt, double f0);

	// Build state-space matrices for current switch state
	void buildMatricesForState(const MatrixXcd& A0, const MatrixXcd& B0,
		const MatrixXcd& C0, const MatrixXcd& D0, const VectorXi& swVec, const VectorXi& swType,
		const VectorXd& swOnRes, const VectorXd& swOffRes, MatrixXcd& Ao, MatrixXcd& Bo,
		MatrixXcd& Co, MatrixXcd& Do);

	void convertToPhasor(const MatrixXcd& A, const MatrixXcd& B,
		const MatrixXcd& C, const MatrixXcd& D, MatrixXcd& Adc, MatrixXcd& Bdc,
		MatrixXcd& Cdc, MatrixXcd& Ddc);

private:
	// Persistent variables, obtained using state-space matrices solver
	MatrixXcd Ads, Bds, Cds, Dds;
	MatrixXcd x_old;
	VectorXi swVec, swVecOld;
	int nSwitches, nStates, nOutputs, nInputs;
	VectorXcd yswitch;
	bool initialized = false;
};


#endif // _DQSYM_H_