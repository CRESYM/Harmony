#include "Examples.h"

#include "../Elements/Converter/Simple_MMC.h"
#include "../Solver/DQsym/DQsym.h"
#include "../Solver/Helper_Functions/Helper_Functions.h"
#include "../Solver/DQsym/DQsymrun.h"
#include "../Constants.h"

#include <complex>
#include <iostream>

void example_DQsym_Simple_MMC()
{
    std::cout << "Entered example_DQsym_Simple_MMC()\n";

    using MatrixXcd = DQsymrun::MatrixXcd;
    using cd = DQsymrun::cd;

    const double f = 50.0;
    const double omega = 2.0 * M_PI * f;
    const double Vdc = 100.0;

    Eigen::MatrixXd Ad, Bd, Cd, Dd;

    std::vector<double> converter_params = {
        omega, 0.0, 0.0, 0.0, 0.0, 0.0, Vdc,
        52.9e-3, 166.3e-3, 1.7568e-3, 1, 0.0, 10.0
    };

    const double Ts = 2e-5;
    const double t0 = 0.0;
    const double tEnd = 1.0;
    const double C = 1.758e-3;
    const int nKeepMMC = 5;
    const int nArm = 5;

    std::cout << "Creating Simple_MMC\n";
    Simple_MMC mmc1("MMC1", "AC1_DC1", converter_params);

    std::cout << "Calling computeABCD\n";
    mmc1.computeABCD();

    std::cout << "Calling discretizeABCD\n";
    discretizeABCD(
        mmc1.getA(),
        mmc1.getB(),
        mmc1.getC(),
        mmc1.getD(),
        Ts,
        Ad, Bd, Cd, Dd
    );


    /*std::cout << "Dd matrix:\n" << Dd << "\n\n";*/

    DQsymrun::Config cfg;
    cfg.Ts = Ts;
    cfg.t0 = t0;
    cfg.tEnd = tEnd;
    cfg.f = f;
    cfg.omega = omega;
    cfg.C = C;
    cfg.nKeep = nKeepMMC;// number of MMC arm Harmonics to keep in the simulation (for visualization and feedback injection)
    cfg.nArm = nArm; //
    cfg.nInputBlocks = 4;
    cfg.upGroupIndex = 0; // index of the block that receives the up feedback (0-based)
    cfg.lowGroupIndex = 1; // index of the block that receives the low feedback (0-based)
    cfg.invertUpFeedback = false;
    cfg.invertLowFeedback = true;

    cfg.swOnRes = Eigen::VectorXd(3);
    cfg.swOnRes << 0.01, 0.01, 0.01;

    cfg.swOffRes = Eigen::VectorXd(3);
    cfg.swOffRes << 1e6, 1e6, 1e6;

    cfg.swType = Eigen::VectorXi(3);
    cfg.swType << 0, 0, 0; // 0 for no switching, 1 for switching

    cfg.breakerFunction = [](int step, double t) -> Eigen::VectorXi
        {
            (void)step;

            Eigen::VectorXi brkVec(3);
            brkVec.setZero();

            if (t >= 2e-4 && t < 6e-4) {
                brkVec.setOnes();
            } // breakers open during this interval

            return brkVec;
        };

    cfg.feedbackInjections = {
        {2, DQsymrun::InternalSignal::VoutUp, false},   // {block index, signal to inject, invert sign?}
        {3, DQsymrun::InternalSignal::VoutLow, false}   // {block index, signal to inject, invert sign?}
    };

    cfg.externalInputFunction = [nKeepMMC](int step, double t) -> std::vector<MatrixXcd>
        {
            (void)step;
            (void)t;

            std::vector<MatrixXcd> blocks(4, MatrixXcd::Zero(3, nKeepMMC));

            MatrixXcd u1(3, nKeepMMC);
            u1 <<
                cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0),
                cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0),
                cd(100, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0);

            MatrixXcd u2(3, nKeepMMC);
            u2 <<
                cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0),
                cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0),
                cd(100, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0);

            blocks[0] = u1;
            blocks[1] = u2;

            return blocks;
        };

    std::cout << "Constructing DQsymrun object\n";
    DQsymrun sim(Ad, Bd, Cd, Dd, cfg);

    std::cout << "Calling sim.run()\n";
    auto result = sim.run();

    std::cout << "Returned from sim.run()\n";

    sim.exportCSV("DQsymSimpleMMC_abc_output.csv");
    std::cout << "Wrote DQsymSimpleMMC_abc_output.csv\n";

    std::cout << "Calling plot\n";
    sim.plot();

    // optional: use result if you want direct access
    std::cout << "Simulation stored " << result.time.size() << " time points.\n";

    std::cout << "Press Enter to continue...\n";
    std::cin.get();
}