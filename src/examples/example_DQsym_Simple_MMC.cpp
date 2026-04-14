#include "Examples.h"

#include "../Elements/Converter/Simple_MMC.h"
#include "../Solver/DQsym/DQsym.h"
#include "../Constants.h"


void example_DQsym_Simple_MMC()
{
    double f = 50;
    double omega = 2 * M_PI * f; // Nominal frequency in rad/s
    double Vdc = 100; // DC voltage in Volts
    std::vector<double> converter_params = { omega, 0, 0, 0.0, 0, 0, Vdc, 52.9e-3, 166.3e-3, 1.7568e-3, 1, 0, 10.0};

    /*Simple_MMC(const std::string & symbol, const std::string & location,
        double omega, double activePower, double reactivePower,
        double angle, double acVoltage, double Pdc, double dcVoltage,
        double armInductance, double armResistance, double armCapacitance,
        int numSubmodules, double reactorInductance, double reactorResistance);*/

    Simple_MMC* mmc1 = new Simple_MMC("MMC1", "AC1_DC1", converter_params); 

	cout << "MMC system matrices:" << endl;
    cout << "A matrix:" << endl;
    cout << mmc1->getA() << endl;
    cout << "B matrix:" << endl;
	cout << mmc1->getB() << endl;
	cout << "C matrix:" << endl;
	cout << mmc1->getC() << endl;
	cout << "D matrix:" << endl;
	cout << mmc1->getD() << endl;
   
//    Simple_MMC mmc;
//    DQsym dqVis;
//    dqVis.reset();
// 
//    VectorXd swOnRes(3);
//    swOnRes << 0.01, 0.01, 0.01;
//
//    VectorXd swOffRes(3);
//    swOffRes << 1e6, 1e6, 1e6;
//
//    VectorXi swType(3);
//    swType << 0, 0, 0;
//
//    VectorXcd xo = VectorXcd::Zero(6);
//
//    const double dt = 2e-5;
//    const double f0 = 50.0;
//    const double t0 = 0.0;
//    const double tEnd = 0.2;
//    const double C = 1.758e-3;
//
//    const int N = static_cast<int>((tEnd - t0) / dt) + 1;
//    const int nGroups = static_cast<int>(Cd.rows() / 3);
//
//    mmc.reset();
//    mmc.setSystemMatrices(Ad, Bd, Cd, Dd);
//    mmc.setSwitchData(swOnRes, swOffRes, swType);
//    mmc.setParameters(dt, f0, C, 5, 9);
//    mmc.setInitialState(xo);
//
//    std::vector<double> time(N);
//    Eigen::MatrixXi brkHistory(N, 3);
//    std::vector<Eigen::MatrixXd> XabcHist(nGroups, Eigen::MatrixXd::Zero(N, 3));
//
//    Eigen::MatrixXd IupAbcHist(N, 3);
//    Eigen::MatrixXd IlowAbcHist(N, 3);
//    Eigen::MatrixXd VcUpAbcHist(N, 3);
//    Eigen::MatrixXd VcLowAbcHist(N, 3);
//    Eigen::MatrixXd VoutUpAbcHist(N, 3);
//    Eigen::MatrixXd VoutLowAbcHist(N, 3);
//
//    for (int k = 0; k < N; ++k)
//    {
//        const double t = t0 + k * dt;
//        const double theta = 2.0 * M_PI * f0 * t;
//        time[k] = t;
//
//        VectorXi brkVec(3);
//        brkVec.setZero();
//        if (t >= 2e-4 && t < 6e-4) {
//            brkVec.setOnes();
//        }
//
//        brkHistory.row(k) = brkVec.transpose();
//
//        MatrixXcd u1(3, 5);
//        u1 <<
//            cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0),
//            cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0),
//            cd(100, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0);
//
//        MatrixXcd u2(3, 5);
//        u2 <<
//            cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0),
//            cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0),
//            cd(100, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0);
//
//        Simple_MMC::StepResult result = mmc.step(u1, u2, brkVec);
//
//        for (int g = 0; g < nGroups; ++g)
//        {
//            MatrixXcd block = result.y_mmc.block(3 * g, 0, 3, result.y_mmc.cols());
//            Eigen::Vector3d xabc = dqVis.dqn2abc_at_time(block, theta);
//            XabcHist[g].row(k) = xabc.transpose();
//        }
//
//        IupAbcHist.row(k) = dqVis.dqn2abc_at_time(result.i_up, theta).transpose();
//        IlowAbcHist.row(k) = dqVis.dqn2abc_at_time(result.i_low, theta).transpose();
//        VcUpAbcHist.row(k) = dqVis.dqn2abc_at_time(result.vc_up, theta).transpose();
//        VcLowAbcHist.row(k) = dqVis.dqn2abc_at_time(result.vc_low, theta).transpose();
//        VoutUpAbcHist.row(k) = dqVis.dqn2abc_at_time(result.vout_up, theta).transpose();
//        VoutLowAbcHist.row(k) = dqVis.dqn2abc_at_time(result.vout_low, theta).transpose();
//    }

}