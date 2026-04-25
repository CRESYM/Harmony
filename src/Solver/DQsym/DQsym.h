#ifndef _DQSYM_H_
#define _DQSYM_H_

#include "../../Constants.h"
#include "../Helper_Functions/Helper_Functions.h"

class Network; class SubNetwork; class Element;

struct DSSState {
    MatrixXcd Ads, Bds, Cds, Dds;
    MatrixXcd x_old;
    VectorXi  swVec, swVecOld;
    VectorXcd yswitch;
    int  nStates = 0, nInputs = 0, nOutputs = 0, nSwitches = 0;
    bool initialized = false;
};

struct Config {
    double dt = 0.0, t_start = 0.0, t_end = 0.0;
    double f = 0.0, omega = 0.0;
    int nKeep = 1;
    Eigen::VectorXd swOnRes, swOffRes;
    Eigen::VectorXi swType;
    std::function<Eigen::VectorXi(int step, double t)> breakerFunction;
    std::vector<Bus*> outputBuses;
};

struct DQsymResult {
    std::vector<double> time;
    MatrixXi brkHistory;
    std::vector<MatrixXd> DSSabcHist;
};

class DQsym {
public:
    DQsym() = default;
    void initialize(Network* net);
    DQsymResult run(Config& cfg);
    void reset() { dssState_ = DSSState{}; hasRun_ = false; }
    void exportCSV(const std::string& filename) const;
    void plot() const;
    const DQsymResult& getResult() const;
    bool hasRun() const { return hasRun_; }

    MatrixXcd DSSS(DSSState&, const MatrixXcd&, const MatrixXcd&,
        const MatrixXcd&, const MatrixXcd&,
        const VectorXd&, const VectorXd&, const VectorXi&, const VectorXi&,
        const MatrixXcd&, const VectorXcd&, double dt = 2e-5, double f0 = 50.0);

    void buildMatricesForState(const MatrixXcd&, const MatrixXcd&,
        const MatrixXcd&, const MatrixXcd&,
        const VectorXi&, const VectorXi&, const VectorXd&, const VectorXd&,
        MatrixXcd&, MatrixXcd&, MatrixXcd&, MatrixXcd&);

private:
    DSSState dssState_;
    bool hasRun_ = false;
    DQsymResult result_;
    Network* net_ = nullptr;
    std::vector<std::string> ac_grid_names, dc_grid_names;
    std::unordered_map<std::string, SubNetwork*> ac_grids, dc_grids;
    std::unordered_map<std::string, Element*> converters;
};

#endif
