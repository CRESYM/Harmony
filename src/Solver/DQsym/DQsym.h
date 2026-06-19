#ifndef _DQSYM_H_
#define _DQSYM_H_

/**
 * @file DQsym.h
 * @brief Dynamic phasor (DQsym) time-domain solver with switch handling.
 *
 * Discretizes the network state-space model in the phasor domain and
 * advances it step-by-step using the DSSS (Discrete-time State-Space
 * Solver) algorithm, supporting breaker state changes during simulation.
 */

#include "../../Constants.h"
#include "../../Bus.h"
#include "../Helper_Functions/Helper_Functions.h"

class Network; class SubNetwork; class Element;

/**
 * @brief Persistent state for the DSSS phasor-domain integrator.
 *
 * Holds the current switch-modified discrete-time matrices and the
 * previous-step state/switch vectors used to detect topology changes.
 */
struct DSSState {
    MatrixXcd Ads, Bds, Cds, Dds;
    MatrixXcd x_old;
    VectorXi  swVec, swVecOld;
    VectorXcd yswitch;
    int  nStates = 0, nInputs = 0, nOutputs = 0, nSwitches = 0;
    bool initialized = false;
};

/**
 * @brief Simulation configuration for a DQsym run.
 */
struct Config {
    double dt = 0.0, t_start = 0.0, t_end = 0.0;
    double f = 0.0, omega = 0.0;
    int nKeep = 1;
    Eigen::VectorXd swOnRes, swOffRes;
    Eigen::VectorXi swType;
    std::function<Eigen::VectorXi(int step, double t)> breakerFunction;
    std::vector<Bus*> outputBuses;
};

/**
 * @brief Time-domain results produced by a DQsym simulation.
 */
struct DQsymResult {
    std::vector<double> time;
    MatrixXi brkHistory;
    std::vector<MatrixXd> DSSabcHist;
};

/**
 * @class DQsym
 * @brief Orchestrates phasor-domain dynamic simulation of an AC/DC network.
 *
 * Initializes area/subnetwork metadata from a Network, builds discrete-time
 * state-space matrices, and runs the DSSS integrator with optional breaker
 * switching. Results can be exported to CSV or plotted interactively.
 */
class DQsym {
public:
    /** @brief Default constructor. */
    DQsym() = default;

    /**
     * @brief Loads network topology and area metadata for simulation.
     * @param net Pointer to the network to simulate.
     */
    void initialize(Network* net);

    /**
     * @brief Runs the full DQsym time-domain simulation.
     * @param cfg Simulation parameters (time range, step size, breakers, outputs).
     * @return Time histories of states, breaker positions, and abc waveforms.
     */
    DQsymResult run(Config& cfg);

    /** @brief Resets internal DSSS state so a fresh run can be started. */
    void reset() { dssState_ = DSSState{}; hasRun_ = false; }

    /**
     * @brief Writes simulation results to a CSV file.
     * @param filename Output file path.
     */
    void exportCSV(const std::string& filename) const;

    /** @brief Opens an interactive plot of the most recent simulation results. */
    void plot() const;

    /**
     * @brief Returns the result of the last completed simulation.
     * @return Const reference to stored DQsymResult.
     */
    const DQsymResult& getResult() const;

    /**
     * @brief Returns whether at least one simulation has completed.
     * @return True after run() succeeds at least once.
     */
    bool hasRun() const { return hasRun_; }

    /**
     * @brief Advances the discrete-time phasor-domain state-space system one step at a time.
     *
     * Rebuilds switch-modified matrices whenever the breaker configuration changes.
     * Operates directly on the supplied @p st, updating it in place.
     *
     * @param st Persistent DSSS state (matrices, previous x, switch vectors).
     * @param Ad Discrete-time state matrix.
     * @param Bd Discrete-time input matrix.
     * @param Cd Discrete-time output matrix.
     * @param Dd Discrete-time feed-through matrix.
     * @param swOnRes ON-resistance for each switch.
     * @param swOffRes OFF-resistance for each switch.
     * @param swType Switch type identifier per element pin.
     * @param brkVec Current breaker (open/closed) state vector.
     * @param u Input matrix over the simulation horizon (n_inputs × T).
     * @param xo Initial state vector.
     * @param dt Time step (default 2e-5 s).
     * @param f0 Fundamental frequency in Hz (default 50 Hz).
     * @return Output matrix (n_outputs × T).
     */
    MatrixXcd DSSS(DSSState&, const MatrixXcd&, const MatrixXcd&,
        const MatrixXcd&, const MatrixXcd&,
        const VectorXd&, const VectorXd&, const VectorXi&, const VectorXi&,
        const MatrixXcd&, const VectorXcd&, double dt = 2e-5, double f0 = 50.0);

    /**
     * @brief Builds switch-modified discrete-time matrices for the current breaker state.
     * @param Ad Base discrete-time state matrix (phasor domain).
     * @param Bd Base discrete-time input matrix (phasor domain).
     * @param Cd Base discrete-time output matrix (phasor domain).
     * @param Dd Base discrete-time feed-through matrix (phasor domain).
     * @param swVec Current switch state vector.
     * @param swVecOld Previous switch state vector.
     * @param swOnRes ON-resistances for switches.
     * @param swOffRes OFF-resistances for switches.
     * @param Ads Output: modified state matrix.
     * @param Bds Output: modified input matrix.
     * @param Cds Output: modified output matrix.
     * @param Dds Output: modified feed-through matrix.
     */
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
