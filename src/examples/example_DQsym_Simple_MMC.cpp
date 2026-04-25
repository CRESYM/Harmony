#include "Examples.h"

#include "../network.h"
#include "../Include_components.h"
#include "../Solver/DQsym/DQsym.h"

using cd = std::complex<double>;

void example_DQsym_Simple_MMC()
{
    std::cout << "=== example_DQsym_Simple_MMC (sigma-delta, network) ===\n";

    const double f = 50.0;
    const double omega = 2.0 * M_PI * f;
    const double Vdc = 100.0;
    const int    nKeep = 5;

    // ================================================================
    //  Network: AC bus (3-pin) — MMC — DC bus (2-pin) — DC source — gnd
    // ================================================================
    Network net;

    Bus* ac_bus = new Bus("ACBUS1", "AC1", 3);     // 3-phase PCC
    Bus* dc_bus = new Bus("DCBUS1", "DC1", 2);     // 2-pin: pin0 = DC+, pin1 = DC-
    Bus* ac_gnd = new Bus("GNDBUSAC", "AC1", 3);
	Bus* dc_gnd = new Bus("GNDBUSDC", "DC1", 2);

    net.addBus("AC1", ac_bus);
    net.addBus("DC1", dc_bus);
    net.addBus("gnd", ac_gnd);
	net.addBus("gnd", dc_gnd);
    // MMC: terminal 0 → AC bus, terminal 1 → DC bus
    std::vector<double> params = {
        omega, 0.0, 0.0, 0.0, 0.0, 0.0, Vdc,
        52.9e-3, 166.3e-3, 1.7568e-3, 1, 0.0, 10.0
    };
    Simple_MMC* mmc = new Simple_MMC("MMC1", "AC1_DC1", params);
    net.addElement("MMC1", mmc);
	net.connectElementToBus(mmc, 1, ac_bus);
    net.connectElementToBus(mmc, 2, dc_bus);

    // DC voltage source (2-pin): between DC bus and ground
    //   pin 0 → DC bus pin 0 (DC+)
    //   pin 1 → DC bus pin 1 (DC-)
    // Source value provided via input function
    DC_source* vs_dc = new DC_source("Vs_dc", "DC1", 2, std::vector<double>{Vdc, -Vdc}, 0);
    net.addElement("Vs_dc", vs_dc);
	net.connectElementToBus(vs_dc, 1, dc_bus);
	net.connectElementToBus(vs_dc, 2, dc_gnd);

	// AC source (3-pin): between AC bus and ground
	//   pin 0 → AC bus pin 0 (phase A)
	//   pin 1 → AC bus pin 1 (phase B)
	//  pin 2 → AC bus pin 2 (phase C)
	// Source value provided via input function
	AC_source* vs_ac = new AC_source("Vs_ac", "AC1", 3, 0.0, 0.0);
	net.addElement("Vs_ac", vs_ac);
	net.connectElementToBus(vs_ac, 1, ac_bus);
	net.connectElementToBus(vs_ac, 2, ac_gnd);

    std::cout << "Network: 1 MMC (3+2 pins), 1 DC source (2 pins), 1 AC source (3 pins), 4 buses\n";

	//net.printConnections();

    //// ================================================================
    ////  DQsym config
    //// ================================================================
    //DQsym dq;
    //dq.initialize(&net);

    //Config cfg;
    //cfg.dt = 2e-5;
    //cfg.t_start = 0.0;
    //cfg.t_end = 0.5;
    //cfg.f = f;
    //cfg.omega = omega;
    //cfg.nKeep = nKeep;
    //cfg.nArm = nKeep;

    //cfg.swOnRes = Eigen::VectorXd::Constant(3, 0.01);
    //cfg.swOffRes = Eigen::VectorXd::Constant(3, 1e6);
    //cfg.swType = Eigen::VectorXi::Zero(3);

    //cfg.breakerFunction = nullptr;   // no breaker events

    //cfg.outputBuses = { ac_bus };

    //// 4 state groups: i^Δ, i^Σ, v_C^Δ, v_C^Σ
    //cfg.converterRoutes = { { "MMC1", {0, 1, 2, 3}, {} } };

    //// Input: DC source has 2 pins (2 symbols: V_Vs_dc0, V_Vs_dc1)
    ////   pin 0 (DC+): +Vdc/2 = +50V at h=0
    ////   pin 1 (DC-): -Vdc/2 = -50V at h=0
    //cfg.inputFunction = [Vdc](int, double, int nu, int nKeep) -> MatrixXcd {
    //    MatrixXcd u = MatrixXcd::Zero(nu, nKeep);
    //    u(0, 0) = cd(+Vdc / 2.0, 0.0);     // V_dc+ = +50V
    //    u(1, 0) = cd(-Vdc / 2.0, 0.0);     // V_dc- = -50V
    //    return u;
    //    };

    //// ================================================================
    ////  Run
    //// ================================================================
    //std::cout << "Running simulation...\n";
    //DQsymResult result = dq.run(cfg);

    //std::cout << "Done — " << result.time.size() << " steps, "
    //    << result.DSSabcHist.size() << " output groups.\n\n"
    //    << "  [0] i^Delta_abc    (differential current)\n"
    //    << "  [1] i^Sigma_abc    (circulating current)\n"
    //    << "  [2] v_C^Delta_abc  (differential cap voltage)\n"
    //    << "  [3] v_C^Sigma_abc  (common-mode cap voltage)\n";

    //dq.exportCSV("DQsym_SigDelta_MMC_output.csv");
    //dq.plot();

    delete mmc; delete vs_dc; delete vs_ac; delete ac_bus; delete dc_bus; delete ac_gnd; delete dc_gnd;
}