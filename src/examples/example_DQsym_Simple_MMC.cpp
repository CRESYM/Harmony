#include "Examples.h"

#include "../network.h"
#include "../Include_components.h"
#include "../Solver/DQsym/DQsym.h"

using cd = std::complex<double>;

void example_DQsym_Simple_MMC()
{
    std::cout << "=== example_DQsym_MMC (sigma-delta, network-integrated) ===\n";

    const double f = 50.0;
    const double omega = 2.0 * M_PI * f;
    const double Vdc = 100.0;
    const int    nKeep = 5;

    // ================================================================
    //  Network
    // ================================================================
    //
    //  Pin matching rule: every element-to-bus connection must have
    //  element.input_pins == bus.getPinNumber() at that terminal.
    //
    //  Ground buses: internal name "gnd" (so finalizeCounts skips them),
    //  but stored under unique keys and with correct pin counts.
    //
    Network net;

    // Signal buses
    Bus* ac_bus = new Bus("AC1", "AC1", 3);    // 3-phase PCC
    Bus* dc_bus = new Bus("DC1", "DC1", 2);    // 2-pin DC bus (DC+, DC-)

    // Ground buses — pin count matches the element connected to them
    Bus* gnd_ac = new Bus("gnd", "AC1", 3);    // 3-pin ground for AC source
    Bus* gnd_dc = new Bus("gnd", "DC1", 2);    // 2-pin ground for DC source

    net.addBus("AC1", ac_bus);
    net.addBus("DC1", dc_bus);
    net.addBus("gnd_ac", gnd_ac);    // unique key, but getBusName()="gnd"
    net.addBus("gnd_dc", gnd_dc);    // unique key, but getBusName()="gnd"

    // ---- MMC ----
    // Converter(symbol, location) → Element(symbol, location, 3, 2)
    //   Terminal 1 → AC bus (3-pin ↔ input_pins=3)  ✓
    //   Terminal 2 → DC bus (2-pin ↔ output_pins=2)  ✓
    std::vector<double> params = {
        omega, 0.0, 0.0, 0.0, 0.0, 0.0, Vdc,
        52.9e-3, 166.3e-3, 1.7568e-3, 1, 0.0, 10.0, 0.0
    };
    MMC* mmc = new MMC("MMC1", "AC1_DC1", params);
    net.addElement(mmc);
    net.connectElementToBus(mmc, 1, ac_bus);     // 3-pin ↔ 3-pin ✓
    net.connectElementToBus(mmc, 2, dc_bus);     // 2-pin ↔ 2-pin ✓

    // ---- DC source (2-pin) ----
    //   Terminal 1 → DC bus (2-pin)   ✓
    //   Terminal 2 → gnd_dc (2-pin)   ✓
    DC_source* vs_dc = new DC_source("Vs_dc", "DC1", 2,
        std::vector<double>{Vdc / 2.0, -Vdc / 2.0}, 0.0);
    net.addElement(vs_dc);
    net.connectElementToBus(vs_dc, 1, dc_bus);
    net.connectElementToBus(vs_dc, 2, gnd_dc);  // 2-pin ↔ 2-pin ✓

    // ---- AC source (3-pin) ----
    //   Terminal 1 → AC bus (3-pin)   ✓
    //   Terminal 2 → gnd_ac (3-pin)   ✓
    AC_source* vs_ac = new AC_source("Vs_ac", "AC1", 3, 0.0, 1e-6);  // Z=1e-6 (not 0!)
    net.addElement(vs_ac);
    net.connectElementToBus(vs_ac, 1, ac_bus);
    net.connectElementToBus(vs_ac, 2, gnd_ac);   // 3-pin ↔ 3-pin ✓

    std::cout << "Network: 1 MMC, 1 DC source (2-pin), 1 AC source (3-pin)\n";
    std::cout << "Buses: AC1(3), DC1(2), gnd_ac(3), gnd_dc(2)\n";

    // ================================================================
    //  StateSpaceModel test
    // ================================================================
    StateSpaceModel ssm;
    ssm.formState(&net, { ac_bus });

    std::cout << "\nA (" << ssm.getA().rows() << "x" << ssm.getA().cols() << "):\n"
        << ssm.getA() << "\n\n";
    std::cout << "B (" << ssm.getB().rows() << "x" << ssm.getB().cols() << "):\n"
        << ssm.getB() << "\n\n";

    // ================================================================
    //  DQsym simulation
    // ================================================================
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

    // Input: source symbols in state-space order
    //   DC source: 2 symbols (V_Vs_dc0, V_Vs_dc1)
    //   AC source: 3 symbols (V_Vs_ac0, V_Vs_ac1, V_Vs_ac2)
    // Total nu = 5 (check B.cols())
    cfg.inputFunction = [Vdc](int, double, int nu, int nKeep) -> MatrixXcd {
        MatrixXcd u = MatrixXcd::Zero(nu, nKeep);
        // Set values at h=0 (DC component)
        // Order depends on element iteration — check B matrix printout
        u(0, 0) = cd(Vdc / 2.0, 0.0);     // V_dc+ = +50V
        u(1, 0) = cd(-Vdc / 2.0, 0.0);    // V_dc- = -50V
        // u(2..4) = 0  (AC source = 0V)
        return u;
        };

    std::cout << "Running DQsym simulation...\n";
    DQsymResult result = dq.run(cfg);

    std::cout << "Done: " << result.time.size() << " steps, "
        << result.DSSabcHist.size() << " output groups.\n";

    dq.exportCSV("DQsym_MMC_SigmaDelta.csv");
    dq.plot();

    cout << "Press enter to continue...\n";
    cin.get();

    delete mmc; delete vs_dc; delete vs_ac;
    delete ac_bus; delete dc_bus; delete gnd_ac; delete gnd_dc;
}
