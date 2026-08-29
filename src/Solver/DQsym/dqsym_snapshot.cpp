/**
 * @file dqsym_snapshot.cpp
 * @brief Operating-point snapshots from DQsym transients for stability assessment.
 */
#include "dqsym_snapshot.h"

#include "../../network.h"
#include "../../Include_components.h"
#include "../State_Space_Model/State_Space_Model.h"
#include "../Stability_Estimate/Stability_estimate.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace {

bool extractOperatingPoint(
    const std::vector<MatrixXcd>& states,
    const Eigen::Vector2d& Vg_dq,
    MmcOperatingPoint& out)
{
    if (states.size() < 4) {
        return false;
    }

    const MatrixXcd& iD = states[0];
    const MatrixXcd& iS = states[1];
    const MatrixXcd& vCS = states[3];

    const double iDelta_d = iD(0, 1).real();
    const double iDelta_q = iD(0, 1).imag();
    const double iSigma_z = iS(2, 0).real();

    const double Vgd = Vg_dq(0);
    const double Vgq = Vg_dq(1);

    out.Vm = std::hypot(Vgd, Vgq);
    out.theta = std::atan2(-Vgq, Vgd);
    out.Pac = 1.5 * (Vgd * iDelta_d + Vgq * iDelta_q);
    out.Qac = 1.5 * (Vgd * iDelta_q - Vgq * iDelta_d);

    const double Vdc_meas = 2.0 * vCS(2, 0).real();
    if (std::abs(Vdc_meas) < 1.0) {
        return false;
    }
    out.Vdc = Vdc_meas;
    out.Pdc = 3.0 * Vdc_meas * iSigma_z;
    return true;
}

void linearizeMmc(MMC& mmc, const MmcOperatingPoint& op) {
    mmc.update_MMC(op.Vm, op.theta, op.Pac, op.Qac, op.Vdc, op.Pdc);
    mmc.solveEquilibrium();
    mmc.computeABCD();
}

void applySnapshotRecord(Network& network, const DqsymSnapshotRecord& record) {
    for (const auto& [name, op] : record.converters) {
        auto it = network.getElements().find(name);
        if (it == network.getElements().end()) {
            std::cerr << "[snapshot] Converter '" << name << "' not found in network.\n";
            continue;
        }
        auto* mmc = dynamic_cast<MMC*>(it->second);
        if (!mmc) {
            std::cerr << "[snapshot] Element '" << name << "' is not an MMC.\n";
            continue;
        }
        try {
            linearizeMmc(*mmc, op);
        }
        catch (const std::exception& ex) {
            std::cerr << "[snapshot] Failed to linearize '" << name << "': " << ex.what() << "\n";
        }
    }
}

} // namespace


Eigen::Vector2d readMmcGridVoltageDq(
    const MMC& mmc,
    const StateSpaceModel& ssm,
    const MatrixXcd& u)
{
    Bus* ac_bus = nullptr;
    for (auto& [bus, terminal] : mmc.getConnections()) {
        if (terminal == 1) {
            ac_bus = bus;
            break;
        }
    }

    Eigen::Vector2d Vg_dq(0.0, 0.0);
    if (!ac_bus) {
        return Vg_dq;
    }

    for (const auto& g : ssm.getInputGroups()) {
        if (g.isVirtual) {
            continue;
        }
        bool found = false;
        for (auto& [bus, terminal] : g.element->getConnections()) {
            if (bus == ac_bus) {
                found = true;
                break;
            }
        }
        if (!found) {
            continue;
        }
        if (g.dqsymStartCol < u.rows() && u.cols() >= 2) {
            const std::complex<double> v_fund = u(g.dqsymStartCol, 1);
            Vg_dq(0) = v_fund.real();
            Vg_dq(1) = v_fund.imag();
        }
        break;
    }
    return Vg_dq;
}


DqsymSnapshotSession& DqsymSnapshotSession::instance() {
    static DqsymSnapshotSession session;
    return session;
}


void DqsymSnapshotSession::clear() {
    retain_network_ = false;
    network_ptr_ = nullptr;
    network_.reset();
    records_.clear();
    stability_cfg_.reset();
}


void DqsymSnapshotSession::setStabilityConfig(const DqsymStabilityPickConfig& cfg) {
    stability_cfg_ = cfg;
}


void DqsymSnapshotSession::setNetworkPointer(Network* network) {
    network_ptr_ = network;
}


void DqsymSnapshotSession::adoptNetwork(std::unique_ptr<Network> network) {
    network_ = std::move(network);
    network_ptr_ = nullptr;
}


Network* DqsymSnapshotSession::network() {
    return network_ ? network_.get() : network_ptr_;
}


void DqsymSnapshotSession::addRecord(DqsymSnapshotRecord record) {
    records_.push_back(std::move(record));
}


int DqsymSnapshotSession::nearestRecordIndex(double t) const {
    if (records_.empty()) {
        return -1;
    }
    int best = 0;
    double bestDist = std::abs(records_.front().t - t);
    for (int i = 1; i < static_cast<int>(records_.size()); ++i) {
        const double dist = std::abs(records_[static_cast<size_t>(i)].t - t);
        if (dist < bestDist) {
            bestDist = dist;
            best = i;
        }
    }
    return best;
}


bool DqsymSnapshotSession::applyRecordToNetwork(int index) {
    Network* net = network();
    if (!net || index < 0 || index >= static_cast<int>(records_.size())) {
        return false;
    }
    applySnapshotRecord(*net, records_[static_cast<size_t>(index)]);
    return true;
}


void DqsymSnapshotSession::runStabilityAssessment(
    const bool plottingEnabled,
    const std::string& titleSuffix)
{
    Network* net = network();
    if (!net || !stability_cfg_) {
        throw std::runtime_error("Snapshot stability requires a retained network and stability config.");
    }

    StabilityEstimate stability;
    stability.add_areas(net);
    stability.print_summary();

    const auto& cfg = *stability_cfg_;
    std::cout << "[snapshot] Stability assessment"
              << (titleSuffix.empty() ? "" : (" " + titleSuffix))
              << " for '" << cfg.converter_id << "' at " << cfg.location << "\n";

    stability.writeFileTF(
        cfg.converter_id,
        cfg.location,
        cfg.freq_start,
        cfg.freq_end,
        cfg.freq_points);

    if (!plottingEnabled || !cfg.plot) {
        return;
    }

    if (cfg.plot_type == "nyquist") {
        stability.nyquistplotTF(
            cfg.converter_id, cfg.location,
            cfg.freq_start, cfg.freq_end, cfg.freq_points);
    }
    else {
        stability.bodeplotTF(
            cfg.converter_id, cfg.location,
            cfg.freq_start, cfg.freq_end, cfg.freq_points);
    }
}


DqsymSnapshotRecord collectConverterSnapshots(
    Network* net,
    const std::map<std::string, std::vector<MatrixXcd>>& elementStates,
    const StateSpaceModel& ssm,
    const MatrixXcd& u,
    const double t,
    const int stepIndex)
{
    DqsymSnapshotRecord record;
    record.t = t;
    record.step_index = stepIndex;

    if (!net) {
        return record;
    }

    for (const auto& [name, elem] : net->get_converters()) {
        auto* mmc = dynamic_cast<MMC*>(elem);
        if (!mmc) {
            continue;
        }
        const auto statesIt = elementStates.find(name);
        if (statesIt == elementStates.end()) {
            continue;
        }

        MmcOperatingPoint op;
        const Eigen::Vector2d Vg_dq = readMmcGridVoltageDq(*mmc, ssm, u);
        if (extractOperatingPoint(statesIt->second, Vg_dq, op)) {
            record.converters[name] = op;
        }
    }
    return record;
}
