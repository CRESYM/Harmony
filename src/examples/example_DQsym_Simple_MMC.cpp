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

    Simple_MMC* mmc1 = new Simple_MMC("MMC1", "AC1_DC1", params);

    // ---- solver ----
    DQsym dq;
	dq.addConverter("MMC1", mmc1);

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
    //  Global input blocks:
    //    0 = DC voltage upper     (external, constant)
    //    1 = DC voltage lower     (external, constant)
    //    2 = MMC1 upper arm Vout  (feedback)
    //    3 = MMC1 lower arm Vout  (feedback)
    //
    //  MMC1's B matrix expects 4 input blocks in that same order,
    //  so inputBlocks = {0, 1, 2, 3} (identity mapping).
    //
    cfg.converterRoutes = {
        {
            "MMC1",             // name
            0, 1,               // upGroupIndex, lowGroupIndex (in THIS converter's DSS output)
            false, true,        // invertUp, invertLow
            { 0, 1, 2, 3 },    // inputBlocks: global block i → local input i
            {                   // feedbacks: converter output → global block
                { 0, 2, false },    //   VoutUp  → global block 2
                { 1, 3, false }     //   VoutLow → global block 3
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