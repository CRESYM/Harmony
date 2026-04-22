#ifndef _DQSYM_H_
#define _DQSYM_H_

#include "../../Constants.h"
#include "../Helper_Functions/Helper_Functions.h"


class Network;
class SubNetwork;
class Element;

struct FeedbackRoute
{
    std::string sourceConverter;   // which converter to read from
    int sourceSignalIndex = 0;     // index in output vector

    int targetBlockIndex = 0;      // where it goes in DSSS input

    bool invert = false;
};

struct Config
{
    double dt = 0.0;
    double t_start = 0.0;
    double t_end = 0.0;

    double f = 0.0;  // system frequency
    double omega = 0.0; // system angular frequency

    double C = 0.0; // delete, inside MMC

    int nKeep = 0; // truncation, used for internal MMC calcultion -> delete, kept inside MMC
    int nArm = 0; // delete, inside MMC
    int nInputBlocks = 0;   // DSSS input blocks, each block must be 3 x nKeep 

    int upGroupIndex = 0; // 
    int lowGroupIndex = 0;
    bool invertUpFeedback = true;
    bool invertLowFeedback = true;

    // Switch parameters
    Eigen::VectorXd swOnRes;
    Eigen::VectorXd swOffRes;
    Eigen::VectorXi swType;

    std::function<Eigen::VectorXi(int step, double t)> breakerFunction;
    std::function<std::vector<MatrixXcd>(int step, double t)> externalInputFunction;

    std::vector<FeedbackRoute> feedbackRoutes;
};

struct DQsymResult
{
    vector<double> time; // Time vector for each simulation step
    MatrixXi brkHistory; // Switch state history (0 for off, 1 for on)

    vector<MatrixXd> DSSabcHist; // History of DSSS outputs in abc format for each group (3 columns per group)
    vector<MatrixXd> MMCabcHist; // History of MMC-related variables in abc format for each group (3 columns per group)
    // Can be the vector of matrices UupAbcHist, UlowAbcHist, IupAbcHist, IlowAbcHist, VcUpAbcHist, 
    // VcLowAbcHist, VoutUpAbcHist, VoutLowAbcHist
};

class DQsym
{
public:
    DQsym() = default;
    ~DQsym() = default;

	// Initialize the DQsym object with the given network and parameters
    void initialize(Network* net);

    // Reset internal persistent DSSS state
    void reset();

    // Discrete state-space solver for dynamic phasors
    MatrixXcd DSSS(const MatrixXcd& Ad, const MatrixXcd& Bd,
        const MatrixXcd& Cd, const MatrixXcd& Dd,
        const VectorXd& swOnRes, const VectorXd& swOffRes,
        const VectorXi& swType, const VectorXi& brkVec,
        const MatrixXcd& u, const VectorXcd& xo,
        double dt = 2e-5, double f0 = 50.0);

    // Build state-space matrices for current switch state
    void buildMatricesForState(const MatrixXcd& A0, const MatrixXcd& B0,
        const MatrixXcd& C0, const MatrixXcd& D0,
        const VectorXi& swVec, const VectorXi& swType,
        const VectorXd& swOnRes, const VectorXd& swOffRes,
        MatrixXcd& Ao, MatrixXcd& Bo,
        MatrixXcd& Co, MatrixXcd& Do);
   

	// Run the full DSSS simulation and store results in DQsymResult
    DQsymResult run(Config& cfg, MatrixXd Ad, MatrixXd Bd, MatrixXd Cd, MatrixXd Dd);

	// Reading and plotting results
    void exportCSV(const std::string& filename) const;
    void plot() const;
    const DQsymResult& getResult() const;
	bool hasRun() const { return hasRun_; }

private:
    // Persistent variables used by DSSS
    
    MatrixXcd x_old;
    VectorXi swVec, swVecOld;

    VectorXcd yswitch;


    int nSwitches = 0;
    int nStates = 0;
    int nOutputs = 0;
    int nInputs = 0;

    bool initialized = false;

	bool hasRun_ = false; // Flag to track if DSSS has been run at least once
	DQsymResult result_; // Store results for later retrieval

	// Reference to the network being analyzed
	double f0; // Fundamental frequency for dynamic phasor transformation

    // Names for identification
    std::vector<std::string> ac_grid_names;
    std::vector<std::string> dc_grid_names;

    // Core hierarchical system representation
    std::unordered_map<std::string, SubNetwork*> ac_grids;  // AC grids as subnetworks
    std::unordered_map<std::string, SubNetwork*> dc_grids;  // DC grids as subnetworks
    std::unordered_map<std::string, Element*> converters; // Converter subnetworks
};


#endif // _DQSYM_H_