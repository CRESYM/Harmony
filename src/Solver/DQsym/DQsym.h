#ifndef _DQSYM_H_
#define _DQSYM_H_

#include "../../Constants.h"
#include <vector>

struct ABCResult
{
    std::vector<double> t;
    MatrixXd Xabc;
};

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
    MatrixXcd DSSS(const MatrixXcd& Ad, const MatrixXcd& Bd,
        const MatrixXcd& Cd, const MatrixXcd& Dd,
        const VectorXd& swOnRes, const VectorXd& swOffRes,
        const VectorXi& swType, const VectorXi& brkVec,
        const MatrixXcd& u, const VectorXcd& xo,
        double dt = 2e-5, double f0 = 50.0);

    // Reset internal persistent DSSS state
    void reset();

    // Build state-space matrices for current switch state
    void buildMatricesForState(const MatrixXcd& A0, const MatrixXcd& B0,
        const MatrixXcd& C0, const MatrixXcd& D0,
        const VectorXi& swVec, const VectorXi& swType,
        const VectorXd& swOnRes, const VectorXd& swOffRes,
        MatrixXcd& Ao, MatrixXcd& Bo,
        MatrixXcd& Co, MatrixXcd& Do);

    // Convert abc-stacked matrices into phasor / sequence domain
    void convertToPhasor(const MatrixXcd& A, const MatrixXcd& B,
        const MatrixXcd& C, const MatrixXcd& D,
        MatrixXcd& Adc, MatrixXcd& Bdc,
        MatrixXcd& Cdc, MatrixXcd& Ddc);

    // dqn -> abc reconstruction for one 3xH coefficient block at one angle
    Vector3d dqn2abc_at_time(const MatrixXcd& Xdcpnz_c, double theta);

    // Convert all 3-row output groups of Y at one angle theta
    std::vector<Vector3d> dqn2abc_groups_at_time(const MatrixXcd& Y, double theta);

    // Simulate abc reconstruction over time for one 3xH coefficient block
    ABCResult simulate_dqn2abc(const MatrixXcd& Xdcpnz_c,
        double freq_hz, double t0, double t1, double Ts);

private:
    // Persistent variables used by DSSS
    MatrixXcd Ads, Bds, Cds, Dds;
    MatrixXcd x_old;
    VectorXi swVec, swVecOld;
  
    VectorXcd yswitch;


    int nSwitches = 0;
    int nStates = 0;
    int nOutputs = 0;
    int nInputs = 0;

    bool initialized = false;
};


#endif // _DQSYM_H_