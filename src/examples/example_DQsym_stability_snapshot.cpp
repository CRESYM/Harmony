/**
 * @file example_DQsym_stability_snapshot.cpp
 * @brief DQsym transient with operating-point snapshots and stability assessment.
 */
#include "Examples.h"

#include "../network.h"
#include "../Include_components.h"
#include "../Solver/DQsym/DQsym.h"
#include "../Solver/DQsym/dqsym_snapshot.h"
#include "../Solver/Helper_Functions/Visualization.h"

void example_DQsym_stability_snapshot(bool plotting_enabled /*=true*/)
{
    std::cout << "=== example_DQsym_stability_snapshot ===\n";

    const double f = 50.0;
    const double omega = 2.0 * M_PI * f;
    const double Vdc = 400.0;
    const double Vac = 200.0;
    const int nKeep = 5;

    Network net;

    Bus* gnd = new Bus("gnd", "GND", 1);
    Bus* ac_bus = new Bus("AC1", "AC1", 3);
    Bus* dc_bus = new Bus("DC1", "DC1", 2);

    net.addBus(gnd);
    net.addBus(ac_bus);
    net.addBus(dc_bus);

    std::vector<double> controller_params = {
        0,
        0,
        1, 0, 2.57e-4, 3.2e-3, 1, 1e3,
        0,
        0,
        0,
        0,
        1, 0, 48, 480, 2, 0, 0,
        0,
        0
    };

    std::vector<double> params = {
        omega, 0, 0.0, 0.0, Vac, 0, Vdc,
        52.9e-3, 166.3e-3, 1.7568e-3, 36, 1e-2, 10, 0.0
    };

    MMC* mmc = new MMC("MMC1", "AC1_DC1", params, controller_params);
    net.addElement(mmc);
    net.connectElementToBus(mmc, 1, ac_bus);
    net.connectElementToBus(mmc, 2, dc_bus);

    DC_source* vs_dc = new DC_source("Vs_dc", "DC1", 2,
        std::vector<double>{Vdc / 2.0, -Vdc / 2.0}, 0.0);
    net.addElement(vs_dc);
    net.connectElementToBus(vs_dc, 1, dc_bus);
    net.connectElementToBus(vs_dc, 2, gnd);

    AC_source* vs_ac = new AC_source("Vs_ac", "AC1", 3, Vac, 0.0);
    net.addElement(vs_ac);
    net.connectElementToBus(vs_ac, 1, ac_bus);
    net.connectElementToBus(vs_ac, 2, gnd);

    DqsymStabilityPickConfig stabCfg;
    stabCfg.converter_id = "MMC1";
    stabCfg.location = "AC";
    stabCfg.freq_start = 1.0;
    stabCfg.freq_end = 1000.0;
    stabCfg.freq_points = 100;
    stabCfg.plot_type = "bode";
    stabCfg.plot = plotting_enabled;

    auto& session = DqsymSnapshotSession::instance();
    session.clear();
    session.setNetworkPointer(&net);
    session.setStabilityConfig(stabCfg);
    session.setRetainNetwork(true);

    DQsym dq;
    dq.initialize(&net);

    Config cfg;
    cfg.dt = 2e-5;
    cfg.t_start = 0.0;
    cfg.t_end = 2.0;
    cfg.f = f;
    cfg.omega = omega;
    cfg.nKeep = nKeep;
    cfg.swOnRes = Eigen::VectorXd::Constant(1, 0.01);
    cfg.swOffRes = Eigen::VectorXd::Constant(1, 1e6);
    cfg.swType = Eigen::VectorXi::Zero(1);
    cfg.breakerFunction = nullptr;
    cfg.outputBuses = { ac_bus };
    cfg.plotting_enabled = plotting_enabled;
    cfg.snapshot_times = { 1.0 };
    cfg.stability_at_snapshots = stabCfg;
    cfg.stability_on_pick = stabCfg;
    cfg.record_snapshot_history = true;
    cfg.snapshot_history_stride = 100;

    const DQsymResult result = dq.run(cfg);
    std::cout << "Done: " << result.time.size() << " steps, "
        << result.DSSabcHist.size() << " waveform groups.\n";

    if (plotting_enabled) {
        dq.plot();
        if (visualization_is_running()) {
            std::cout << "Close the plot window when finished (click a time to rerun stability).\n";
            visualization_wait();
        }
    }
}
