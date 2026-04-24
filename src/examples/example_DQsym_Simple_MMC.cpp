#include "Examples.h"

#include "../network.h"
#include "../Include_components.h"
#include "../Solver/DQsym/DQsym.h"

using cd = std::complex<double>;

void example_DQsym_Simple_MMC()
{
    std::cout << "=== example_DQsym_Simple_MMC ===\n";

    const double f = 50.0;
    const double omega = 2.0 * M_PI * f;
    const double Vdc = 100.0;
    const int    nKeep = 5;

    // ---- create converter ----
    std::vector<double> params = {
        omega, 0.0, 0.0, 0.0, 0.0, 0.0, Vdc,
        52.9e-3, 166.3e-3, 1.7568e-3, 1, 0.0, 10.0
    };
    Simple_MMC mmc1("MMC1", "AC1_DC1", params);

    // ---- solver ----
    DQsym dq;
    dq.addConverter("MMC1", &mmc1);

    // ---- config ----
    Config cfg;
    cfg.dt = 2e-5;
    cfg.t_start = 0.0;
    cfg.t_end = 1.0;
    cfg.f = f;
    cfg.omega = omega;
    cfg.nKeep = nKeep;
    cfg.nArm = 5;
    cfg.nInputBlocks = 4;

    cfg.swOnRes = Eigen::VectorXd::Constant(3, 0.01);
    cfg.swOffRes = Eigen::VectorXd::Constant(3, 1e6);
    cfg.swType = Eigen::VectorXi::Zero(3);

    cfg.breakerFunction = [](int, double t) -> Eigen::VectorXi {
        Eigen::VectorXi v = Eigen::VectorXi::Zero(3);
        if (t >= 2e-4 && t < 6e-4) v.setOnes();
        return v;
        };

    // ---- per-converter routing ----
    //
    //  Global input blocks (all go into ONE DSSS):
    //    0 = DC voltage upper     (external)
    //    1 = DC voltage lower     (external)
    //    2 = MMC1 upper arm Vout  (feedback)
    //    3 = MMC1 lower arm Vout  (feedback)
    //
    //  Global output groups (from ONE DSSS):
    //    0 = upper arm currents
    //    1 = lower arm currents
    //    2 = mutual currents
    //    3 = mutual voltages
    //
    cfg.converterRoutes = {
        {
            "MMC1",             // name
            0, 1,               // upGroupIndex, lowGroupIndex (global)
            false, true,        // invertUp, invertLow
            {                   // feedbacks: converter output → global input block
                { 0, 2, false },    //   VoutUp  → block 2
                { 1, 3, false }     //   VoutLow → block 3
            }
        }
    };

    // constant DC sources
    cfg.externalInputFunction = [nKeep](int, double)
        -> std::vector<MatrixXcd>
        {
            std::vector<MatrixXcd> blocks(4, MatrixXcd::Zero(3, nKeep));
            blocks[0](2, 0) = cd(100.0, 0.0);   // DC upper
            blocks[1](2, 0) = cd(100.0, 0.0);   // DC lower
            return blocks;
        };

    // ---- run ----
    std::cout << "Running...\n";
    DQsymResult result = dq.run(cfg);
    std::cout << "Done — " << result.time.size() << " steps, "
        << result.DSSabcHist.size() << " DSS groups, "
        << result.MMCabcHist.size() << " MMC signals.\n";

    dq.exportCSV("DQsym_SimpleMMC_output.csv");
    dq.plot();

    std::cout << "Press Enter to continue...\n";
    std::cin.get();
}
