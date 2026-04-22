#ifndef _DQSYM_CONVERSION_FUNCTIONS_H_
#define _DQSYM_CONVERSION_FUNCTIONS_H_

#include "../../Constants.h"

struct ABCResult
{
    std::vector<double> t;
    MatrixXd Xabc;
};

// Mathematical operations on dynamic phasors
extern MatrixXcd dq_add(const MatrixXcd& a, const MatrixXcd& b);
extern MatrixXcd dq_subtract(const MatrixXcd& a, const MatrixXcd& b);
extern MatrixXcd dq_integrate(MatrixXcd& Zpnz_old, MatrixXcd& Xpnz_old, const MatrixXcd& Xpnz,
    double dt, double w);
extern MatrixXcd dq_multiply(const MatrixXcd& x_coef1_in, const MatrixXcd& y_coef1_in);

// Convert abc-stacked matrices into phasor / sequence domain
extern void convertToPhasor(const MatrixXcd& A, const MatrixXcd& B,
    const MatrixXcd& C, const MatrixXcd& D,
    MatrixXcd& Adc, MatrixXcd& Bdc,
    MatrixXcd& Cdc, MatrixXcd& Ddc);

//Truncation for DQsym
extern MatrixXcd truncateHarmonics(const MatrixXcd& X, int nColsToKeep);


// dqn -> abc reconstruction for one 3xH coefficient block at one angle
extern Vector3d dqn2abc_at_time(const MatrixXcd& Xdcpnz_c, double theta);

// Convert all 3-row output groups of Y at one angle theta
extern std::vector<Vector3d> dqn2abc_groups_at_time(const MatrixXcd& Y, double theta);

// Simulate abc reconstruction over time for one 3xH coefficient block
extern ABCResult simulate_dqn2abc(const MatrixXcd& Xdcpnz_c,
    double freq_hz, double t0, double t1, double Ts);

#endif // _DQSYM_CONVERSION_FUNCTIONS_H_
