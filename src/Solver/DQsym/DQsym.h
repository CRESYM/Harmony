#ifndef _DQSYM_H_
#define _DQSYM_H_

#include "../../Constants.h"
#include "../Helper_Functions/Helper_Functions.h"


class Network;
class SubNetwork;
class Element;

/// Complete persistent state of one DSSS instance
struct DSSState
{
    MatrixXcd Ads, Bds, Cds, Dds;      ///< Cached switch-augmented matrices
    MatrixXcd x_old;                    ///< State from previous time step
    VectorXi  swVec, swVecOld;          ///< Current and previous switch vectors
    VectorXcd yswitch;
    int  nStates = 0;
    int  nInputs = 0;
    int  nOutputs = 0;
    int  nSwitches = 0;
    bool initialized = false;
};

/// Per-converter routing specification
struct ConverterRoute
{
    std::string name;                   ///< Must match key in converters map

    int  upGroupIndex = 0;             ///< DSS output group for upper-arm current (local)
    int  lowGroupIndex = 1;             ///< DSS output group for lower-arm current (local)
    bool invertUp = false;
    bool invertLow = true;

    /// Global input blocks that feed this converter's DSSS (B-matrix column order)
    std::vector<int> inputBlocks;

    /// One feedback path: converter output → global input block
    struct Feedback {
        int  signalIndex;               ///< Index in simulateTimeStep return vector
        int  targetBlock;               ///< Global input block index
        bool invert = false;
    };
    std::vector<Feedback> feedbacks;
};

/// Simulation configuration
struct Config
{
    double dt = 0.0;
    double t_start = 0.0;
    double t_end = 0.0;
    double f = 0.0;
    double omega = 0.0;

    int nKeep = 0;                      ///< DSS-level harmonics
    int nArm = 0;                       ///< Arm-level harmonics
    int nInputBlocks = 0;               ///< Total global input blocks

    Eigen::VectorXd swOnRes;
    Eigen::VectorXd swOffRes;
    Eigen::VectorXi swType;

    std::function<Eigen::VectorXi(int step, double t)> breakerFunction;
    std::function<std::vector<MatrixXcd>(int step, double t)> externalInputFunction;

    std::vector<ConverterRoute> converterRoutes;
};

/// Simulation output
struct DQsymResult
{
    std::vector<double> time;
    MatrixXi brkHistory;
    std::vector<MatrixXd> DSSabcHist;   ///< One Nx3 per DSS output group
    std::vector<MatrixXd> MMCabcHist;   ///< One Nx3 per converter output signal
};


// ===================================================================
//  DQsym class
// ===================================================================

class DQsym
{
public:
    DQsym() = default;
    ~DQsym() = default;

    // ---------------------------------------------------------------
    //  Network setup
    // ---------------------------------------------------------------
    void initialize(Network* net);
    void addConverter(const std::string& name, Element* conv) {
        converters[name] = conv;
    }

    // ---------------------------------------------------------------
    //  Simulation
    // ---------------------------------------------------------------
    DQsymResult run(Config& cfg);
    void reset() { dssStates_.clear(); hasRun_ = false; }

    // ---------------------------------------------------------------
    //  Results
    // ---------------------------------------------------------------
    void exportCSV(const std::string& filename) const;
    void plot() const;
    const DQsymResult& getResult() const;
    bool hasRun() const { return hasRun_; }

    // ---------------------------------------------------------------
    //  DSSS solver (state passed explicitly)
    // ---------------------------------------------------------------
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
        MatrixXcd& Ao, MatrixXcd& Bo,
        MatrixXcd& Co, MatrixXcd& Do);


private:
    // The single set of per-converter DSSS states
    std::unordered_map<std::string, DSSState> dssStates_;

    // Simulation bookkeeping
    bool        hasRun_ = false;
    DQsymResult result_;

    // Network references
    std::vector<std::string> ac_grid_names;
    std::vector<std::string> dc_grid_names;
    std::unordered_map<std::string, SubNetwork*> ac_grids;
    std::unordered_map<std::string, SubNetwork*> dc_grids;
    std::unordered_map<std::string, Element*>    converters;
};


#endif // _DQSYM_H_