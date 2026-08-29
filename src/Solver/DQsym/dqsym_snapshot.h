#pragma once

/**
 * @file dqsym_snapshot.h
 * @brief Operating-point snapshots from DQsym transients for stability assessment.
 */

#include "../../HarmonyTypes.h"

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class MMC;
class Network;
class StateSpaceModel;

/** @brief Six scalars passed to MMC::update_MMC at a snapshot. */
struct MmcOperatingPoint {
    double Vm = 0.0;
    double theta = 0.0;
    double Pac = 0.0;
    double Qac = 0.0;
    double Vdc = 0.0;
    double Pdc = 0.0;
};

/** @brief Converter operating points captured at one simulation time. */
struct DqsymSnapshotRecord {
    double t = 0.0;
    int step_index = 0;
    std::map<std::string, MmcOperatingPoint> converters;
};

/** @brief Parameters for impedance stability assessment at a snapshot. */
struct DqsymStabilityPickConfig {
    std::string converter_id;
    std::string location = "AC";
    double freq_start = 0.1;
    double freq_end = 10000.0;
    int freq_points = 200;
    std::string plot_type = "bode";
    bool plot = true;
};

/**
 * @brief Retains snapshot history and (optionally) the built network for plot-pick stability.
 */
class DqsymSnapshotSession {
public:
    static DqsymSnapshotSession& instance();

    void clear();
    void setRetainNetwork(bool retain) { retain_network_ = retain; }
    bool shouldRetainNetwork() const { return retain_network_; }

    void setStabilityConfig(const DqsymStabilityPickConfig& cfg);
    const std::optional<DqsymStabilityPickConfig>& stabilityConfig() const { return stability_cfg_; }

    void setNetworkPointer(Network* network);
    void adoptNetwork(std::unique_ptr<Network> network);
    Network* network();

    void addRecord(DqsymSnapshotRecord record);
    const std::vector<DqsymSnapshotRecord>& records() const { return records_; }
    bool hasRecords() const { return !records_.empty(); }

    int nearestRecordIndex(double t) const;
    bool applyRecordToNetwork(int index);
    void runStabilityAssessment(bool plottingEnabled, const std::string& titleSuffix = "");

private:
    bool retain_network_ = false;
    Network* network_ptr_ = nullptr;
    std::unique_ptr<Network> network_;
    std::vector<DqsymSnapshotRecord> records_;
    std::optional<DqsymStabilityPickConfig> stability_cfg_;
};

/** @brief Grid voltage (d, q) at the MMC AC terminal from the DQsym input vector. */
Eigen::Vector2d readMmcGridVoltageDq(
    const MMC& mmc,
    const StateSpaceModel& ssm,
    const MatrixXcd& u);

/** @brief Collect MMC operating points from the current DQsym step. */
DqsymSnapshotRecord collectConverterSnapshots(
    Network* net,
    const std::map<std::string, std::vector<MatrixXcd>>& elementStates,
    const StateSpaceModel& ssm,
    const MatrixXcd& u,
    double t,
    int stepIndex);
