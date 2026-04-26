#include "Examples.h"

#include "../network.h"
#include "../Include_components.h"
#include "../Solver/DQsym/DQsym.h"

void example_DQsym_Simple_MMC()
{
    std::cout << "=== example_DQsym_MMC (linear+nonlinear, DQsym mode) ===\n";

    const double f = 50.0;
    const double omega = 2.0 * M_PI * f;
    const double Vdc = 100.0;
    const int    nKeep = 5;

    Network net;

    Bus* gnd = new Bus("gnd", "GND", 1);
    Bus* ac_bus = new Bus("AC1", "AC1", 3);
    Bus* dc_bus = new Bus("DC1", "DC1", 2);    // 2-pin DC bus

    net.addBus(gnd);
    net.addBus(ac_bus);
    net.addBus(dc_bus);

    // MMC
    std::vector<double> params = {
        omega, 0.0, 0.0, 0.0, 0.0, 0.0, Vdc,
        52.9e-3, 166.3e-3, 1.7568e-3, 1, 0.0, 10.0, 0.0
    };
    MMC* mmc = new MMC("MMC1", "AC1_DC1", params);
    net.addElement(mmc);
    net.connectElementToBus(mmc, 1, ac_bus);
    net.connectElementToBus(mmc, 2, dc_bus);

    // DC source (2-pin, values set internally via simulateInputStep)
    DC_source* vs_dc = new DC_source("Vs_dc", "DC1", 2,
        std::vector<double>{Vdc / 2.0, -Vdc / 2.0}, 0.0);
    net.addElement(vs_dc);
    net.connectElementToBus(vs_dc, 1, dc_bus);
    net.connectElementToBus(vs_dc, 2, gnd);

    // AC source (3-pin, zero voltage)
    AC_source* vs_ac = new AC_source("Vs_ac", "AC1", 3, 0.0, 0.0);
    net.addElement(vs_ac);
    net.connectElementToBus(vs_ac, 1, ac_bus);
    net.connectElementToBus(vs_ac, 2, gnd);

    // DQsym — no inputFunction, sources/MMC handle u automatically
    DQsym dq;
    dq.initialize(&net);

    Config cfg;
    cfg.dt = 2e-5;
    cfg.t_start = 0.0;
    cfg.t_end = 0.5;
    cfg.f = f;
    cfg.omega = omega;
    cfg.nKeep = nKeep;

    cfg.swOnRes = Eigen::VectorXd::Constant(1, 0.01);
    cfg.swOffRes = Eigen::VectorXd::Constant(1, 1e6);
    cfg.swType = Eigen::VectorXi::Zero(1);
    cfg.breakerFunction = nullptr;
    cfg.outputBuses = { ac_bus };

    // Run — StateSpaceModel uses SSMMode::DQsym internally
    // DC source 2-pin → expanded to 6 B columns (2 groups of 3)
    // AC source 3-pin → 3 B columns (1 group of 3)
    // MMC virtual 12  → 12 B columns (4 groups of 3)
    // Total B_dqsym: 12×21 columns, all in groups of 3

    DQsymResult result = dq.run(cfg);

    std::cout << "Done: " << result.time.size() << " steps, "
        << result.DSSabcHist.size() << " groups.\n";

    //dq.exportCSV("DQsym_MMC_SigmaDelta.csv");
    dq.plot();

	cout << "Example complete. Press any key to exit.\n";
	std::cin.get();

}
