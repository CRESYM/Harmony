#include "Examples.h"

#include "../Elements/Converter/Simple_MMC.h"
#include "../Solver/DQsym/DQsym.h"


void example_DQsym_Simple_MMC()
{
    std::cout << "Entered example_DQsym_Simple_MMC()\n";


    const double f = 50.0;
    const double omega = 2.0 * M_PI * f;
    const double Vdc = 100.0;

    MatrixXd Ad, Bd, Cd, Dd;

    std::vector<double> converter_params = {
        omega, 0.0, 0.0, 0.0, 0.0, 0.0, Vdc,
        52.9e-3, 166.3e-3, 1.7568e-3, 1, 0.0, 10.0
    };
    Simple_MMC mmc1("MMC1", "AC1_DC1", converter_params);


    // This is an input to the run function
    const double Ts = 2e-5;
    const int nKeep = 5;


    
    const double t0 = 0.0;
    const double tEnd = 1.0;
    
    //swOnRes = VectorXd(3);
    //swOnRes << 0.01, 0.01, 0.01;

    //swOffRes = Eigen::VectorXd(3);
    //swOffRes << 1e6, 1e6, 1e6;

    //swType = Eigen::VectorXi(3);
    //swType << 0, 0, 0; // 0 for no switching, 1 for switching

    //breakerFunction = [](int step, double t) -> Eigen::VectorXi
    //    {
    //        (void)step;

    //        Eigen::VectorXi brkVec(3);
    //        brkVec.setZero();

    //        if (t >= 2e-4 && t < 6e-4) {
    //            brkVec.setOnes();
    //        } // breakers open during this interval

    //        return brkVec;
    //    };

    //feedbackInjections = {
    //    {2, DQsymrun::InternalSignal::VoutUp, false},   // {block index, signal to inject, invert sign?}
    //    {3, DQsymrun::InternalSignal::VoutLow, false}   // {block index, signal to inject, invert sign?}
    //};

    //externalInputFunction = [nKeepMMC](int step, double t) -> std::vector<MatrixXcd>
    //    {
    //        (void)step;
    //        (void)t;

    //        std::vector<MatrixXcd> blocks(4, MatrixXcd::Zero(3, nKeepMMC));

    //        MatrixXcd u1(3, nKeepMMC);
    //        u1 <<
    //            cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0),
    //            cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0),
    //            cd(100, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0);

    //        MatrixXcd u2(3, nKeepMMC);
    //        u2 <<
    //            cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0),
    //            cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0),
    //            cd(100, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0);

    //        blocks[0] = u1;
    //        blocks[1] = u2;

    //        return blocks;
    //    };

    //std::cout << "Constructing DQsymrun object\n";
    //DQsymrun sim(Ad, Bd, Cd, Dd, cfg);

    /*std::cout << "Calling sim.run()\n";
    auto result = sim.run();*/

    //std::cout << "Returned from sim.run()\n";

    //sim.exportCSV("DQsymSimpleMMC_abc_output.csv");
    //std::cout << "Wrote DQsymSimpleMMC_abc_output.csv\n";

    //std::cout << "Calling plot\n";
    //sim.plot();

    // optional: use result if you want direct access
    //std::cout << "Simulation stored " << result.time.size() << " time points.\n";

    std::cout << "Press Enter to continue...\n";
    std::cin.get();
}