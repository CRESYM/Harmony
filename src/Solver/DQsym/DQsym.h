#ifndef _DQSYM_H_
#define _DQSYM_H_

#include "../../Constants.h"
#include "../Helper_Functions/Helper_Functions.h"

class Network;
class SubNetwork;
class Element;
class Bus;

// ===================================================================
//  Persistent DSSS state
// ===================================================================
struct DSSState
{
    MatrixXcd Ads, Bds, Cds, Dds;
    MatrixXcd x_old;
    VectorXi  swVec, swVecOld;
    VectorXcd yswitch;
    int  nStates = 0, nInputs = 0, nOutputs = 0, nSwitches = 0;
    bool initialized = false;
};

// ===================================================================
//  Simulation configuration
// ===================================================================
struct Config
{
    double dt = 0.0, t_start = 0.0, t_end = 0.0;
    double f = 0.0, omega = 0.0;
    int nKeep = 1;

    // Switch parameters
    Eigen::VectorXd swOnRes, swOffRes;
    Eigen::VectorXi swType;

    std::function<Eigen::VectorXi(int step, double t)> breakerFunction;

    /// Input function: returns (nu x nKeep) complex matrix per step.
    /// nu = B.cols() from StateSpaceModel (passed as argument).
    std::function<MatrixXcd(int step, double t, int nu, int nKeep)> inputFunction;

    /// Which buses to observe (for C/D in StateSpaceModel)
    std::vector<Bus*> outputBuses;
};

// ===================================================================
//  Simulation result
// ===================================================================
struct DQsymResult
{
    std::vector<double> time;
    MatrixXi brkHistory;
    std::vector<MatrixXd> DSSabcHist;   ///< One Nx3 per state group (state-space order)
};

// ===================================================================
//  DQsym solver
// ===================================================================
class DQsym
{
public:
    DQsym() = default;
    ~DQsym() = default;

    void DQsym::initialize(Network* net)
    {
        net_ = net;
    }

    /// Assembles state-space from MNA, discretizes, runs DSSS loop
    DQsymResult run(Config& cfg);
    void reset() { dssState_ = DSSState{}; hasRun_ = false; }

    void exportCSV(const std::string& filename) const;
    void plot() const;
    const DQsymResult& getResult() const;
    bool hasRun() const { return hasRun_; }

    // DSSS core
    MatrixXcd DSSS(DSSState& state,
        const MatrixXcd& Ad, const MatrixXcd& Bd,
        const MatrixXcd& Cd, const MatrixXcd& Dd,
        const VectorXd& swOnRes, const VectorXd& swOffRes,
        const VectorXi& swType, const VectorXi& brkVec,
        const MatrixXcd& u, const VectorXcd& xo,
        double dt = 2e-5, double f0 = 50.0);

    void buildMatricesForState(const MatrixXcd& A0, const MatrixXcd& B0,
        const MatrixXcd& C0, const MatrixXcd& D0,
        const VectorXi& swVec, const VectorXi& swType,
        const VectorXd& swOnRes, const VectorXd& swOffRes,
        MatrixXcd& Ao, MatrixXcd& Bo, MatrixXcd& Co, MatrixXcd& Do);

private:
    DSSState dssState_;
    bool hasRun_ = false;
    DQsymResult result_;

    Network* net_ = nullptr;
};

#endif // _DQSYM_H_
