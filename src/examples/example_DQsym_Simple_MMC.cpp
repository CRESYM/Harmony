#include "Examples.h"

#include "../Elements/Converter/Simple_MMC.h"
#include "../Solver/DQsym/DQsym.h"
#include "../Constants.h"

#include <algorithm>
#include <complex>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
    using cd = std::complex<double>;
    constexpr double PI = 3.141592653589793238462643383279502884;

    void write_triplet(std::ofstream& f, const Eigen::Vector3d& x)
    {
        f << "," << x(0) << "," << x(1) << "," << x(2);
    }

    void write_combined_header(std::ofstream& f, int nGroups)
    {
        f << "t,brk1,brk2,brk3";

        for (int g = 0; g < nGroups; ++g)
        {
            f << ",xa" << (g + 1)
                << ",xb" << (g + 1)
                << ",xc" << (g + 1);
        }

        f << ",iup_a,iup_b,iup_c"
            << ",ilow_a,ilow_b,ilow_c"
            << ",vcup_a,vcup_b,vcup_c"
            << ",vclow_a,vclow_b,vclow_c"
            << ",voutup_a,voutup_b,voutup_c"
            << ",voutlow_a,voutlow_b,voutlow_c\n";
    }
}

void example_DQsym_Simple_MMC()
{
    using MatrixXcd = Eigen::MatrixXcd;
    using VectorXd = Eigen::VectorXd;
    using VectorXi = Eigen::VectorXi;
    using VectorXcd = Eigen::VectorXcd;

    Simple_MMC mmc;
    DQsym dqVis;
    dqVis.reset();

    // =========================================================================
    // MMC system matrices
    // =========================================================================
    MatrixXcd Ad(6, 6);
    Ad <<
        cd(0.99993712863053097, 0), cd(0, 0), cd(0, 0), cd(0.0001953429935529639, 0), cd(0, 0), cd(0, 0),
        cd(0, 0), cd(0.99993712863053097, 0), cd(0, 0), cd(0, 0), cd(0.0001953429935529639, 0), cd(0, 0),
        cd(0, 0), cd(0, 0), cd(0.99993712863053097, 0), cd(0, 0), cd(0, 0), cd(0.0001953429935529639, 0),
        cd(0, 0), cd(0, 0), cd(0, 0), cd(0.99954644264342507, 0), cd(0, 0), cd(0, 0),
        cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0.99954644264342507, 0), cd(0, 0),
        cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0.99954644264342507, 0);

    MatrixXcd Bd(6, 12);
    Bd <<
        cd(0, 0), cd(0, 0), cd(15.260081225795597, 0), cd(0, 0), cd(0, 0), cd(3.6429162092755831, 0), cd(-15.260081225795597, 0), cd(0, 0), cd(0, 0), cd(-3.6429162092755831, 0), cd(0, 0), cd(0, 0),
        cd(0, 0), cd(15.260081225795597, 0), cd(0, 0), cd(0, 0), cd(3.6429162092755831, 0), cd(0, 0), cd(0, 0), cd(-15.260081225795597, 0), cd(0, 0), cd(0, 0), cd(-3.6429162092755831, 0), cd(0, 0),
        cd(15.260081225795597, 0), cd(0, 0), cd(0, 0), cd(3.6429162092755831, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(-15.260081225795597, 0), cd(0, 0), cd(0, 0), cd(-3.6429162092755831, 0),
        cd(0, 0), cd(0, 0), cd(-11.617165016520014, 0), cd(0, 0), cd(0, 0), cd(11.617165016520014, 0), cd(11.617165016520014, 0), cd(0, 0), cd(0, 0), cd(-11.617165016520014, 0), cd(0, 0), cd(0, 0),
        cd(0, 0), cd(-11.617165016520014, 0), cd(0, 0), cd(0, 0), cd(11.617165016520014, 0), cd(0, 0), cd(0, 0), cd(11.617165016520014, 0), cd(0, 0), cd(0, 0), cd(-11.617165016520014, 0), cd(0, 0),
        cd(-11.617165016520014, 0), cd(0, 0), cd(0, 0), cd(11.617165016520014, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(11.617165016520014, 0), cd(0, 0), cd(0, 0), cd(-11.617165016520014, 0);

    MatrixXcd Cd(12, 6);
    Cd <<
        cd(0, 0), cd(0, 0), cd(0, 0), cd(-1.0333969214350068e-05, 0), cd(0, 0), cd(0, 0),
        cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(-1.0333969214350068e-05, 0), cd(0, 0),
        cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(-1.0333969214350068e-05, 0),
        cd(1.9999371286305313e-05, 0), cd(0, 0), cd(0, 0), cd(1.9534299355296394e-09, 0), cd(0, 0), cd(0, 0),
        cd(0, 0), cd(1.9999371286305313e-05, 0), cd(0, 0), cd(0, 0), cd(1.9534299355296394e-09, 0), cd(0, 0),
        cd(0, 0), cd(0, 0), cd(1.9999371286305313e-05, 0), cd(0, 0), cd(0, 0), cd(1.9534299355296394e-09, 0),
        cd(1.9999371286305313e-05, 0), cd(0, 0), cd(0, 0), cd(1.9997417856369784e-05, 0), cd(0, 0), cd(0, 0),
        cd(0, 0), cd(1.9999371286305313e-05, 0), cd(0, 0), cd(0, 0), cd(1.9997417856369784e-05, 0), cd(0, 0),
        cd(0, 0), cd(0, 0), cd(1.9999371286305313e-05, 0), cd(0, 0), cd(0, 0), cd(1.9997417856369784e-05, 0),
        cd(0, 0), cd(0, 0), cd(0, 0), cd(1.9995464426434252e-05, 0), cd(0, 0), cd(0, 0),
        cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(1.9995464426434252e-05, 0), cd(0, 0),
        cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(1.9995464426434252e-05, 0);

    MatrixXcd Dd(12, 12);
    Dd <<
        cd(0, 0), cd(0, 0), cd(0.19271632564033428, 0), cd(0, 0), cd(0, 0), cd(-0.19271632564033439, 0), cd(-0.19271632564033428, 0), cd(0, 0), cd(0, 0), cd(0.19271632564033439, 0), cd(0, 0), cd(0, 0),
        cd(0, 0), cd(0.19271632564033428, 0), cd(0, 0), cd(0, 0), cd(-0.19271632564033439, 0), cd(0, 0), cd(0, 0), cd(-0.19271632564033428, 0), cd(0, 0), cd(0, 0), cd(0.19271632564033439, 0), cd(0, 0),
        cd(0.19271632564033428, 0), cd(0, 0), cd(0, 0), cd(-0.19271632564033439, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(-0.19271632564033428, 0), cd(0, 0), cd(0, 0), cd(0.19271632564033439, 0),
        cd(0, 0), cd(0, 0), cd(0.00015260081225795599, 0), cd(0, 0), cd(0, 0), cd(3.6429162092755837e-05, 0), cd(-0.00015260081225795599, 0), cd(0, 0), cd(0, 0), cd(-3.6429162092755837e-05, 0), cd(0, 0), cd(0, 0),
        cd(0, 0), cd(0.00015260081225795599, 0), cd(0, 0), cd(0, 0), cd(3.6429162092755837e-05, 0), cd(0, 0), cd(0, 0), cd(-0.00015260081225795599, 0), cd(0, 0), cd(0, 0), cd(-3.6429162092755837e-05, 0), cd(0, 0),
        cd(0.00015260081225795599, 0), cd(0, 0), cd(0, 0), cd(3.6429162092755837e-05, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(-0.00015260081225795599, 0), cd(0, 0), cd(0, 0), cd(-3.6429162092755837e-05, 0),
        cd(0, 0), cd(0, 0), cd(3.6429162092755823e-05, 0), cd(0, 0), cd(0, 0), cd(0.00015260081225795599, 0), cd(-3.6429162092755823e-05, 0), cd(0, 0), cd(0, 0), cd(-0.00015260081225795599, 0), cd(0, 0), cd(0, 0),
        cd(0, 0), cd(3.6429162092755823e-05, 0), cd(0, 0), cd(0, 0), cd(0.00015260081225795599, 0), cd(0, 0), cd(0, 0), cd(-3.6429162092755823e-05, 0), cd(0, 0), cd(0, 0), cd(-0.00015260081225795599, 0), cd(0, 0),
        cd(3.6429162092755837e-05, 0), cd(0, 0), cd(0, 0), cd(0.00015260081225795599, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(-3.6429162092755837e-05, 0), cd(0, 0), cd(0, 0), cd(-0.00015260081225795599, 0),
        cd(0, 0), cd(0, 0), cd(-0.00011617165016520014, 0), cd(0, 0), cd(0, 0), cd(0.00011617165016520014, 0), cd(0.00011617165016520014, 0), cd(0, 0), cd(0, 0), cd(-0.00011617165016520014, 0), cd(0, 0), cd(0, 0),
        cd(0, 0), cd(-0.00011617165016520014, 0), cd(0, 0), cd(0, 0), cd(0.00011617165016520014, 0), cd(0, 0), cd(0, 0), cd(0.00011617165016520014, 0), cd(0, 0), cd(0, 0), cd(-0.00011617165016520014, 0), cd(0, 0),
        cd(-0.00011617165016520014, 0), cd(0, 0), cd(0, 0), cd(0.00011617165016520014, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0.00011617165016520014, 0), cd(0, 0), cd(0, 0), cd(-0.00011617165016520014, 0);

    VectorXd swOnRes(3);
    swOnRes << 0.01, 0.01, 0.01;

    VectorXd swOffRes(3);
    swOffRes << 1e6, 1e6, 1e6;

    VectorXi swType(3);
    swType << 0, 0, 0;

    VectorXcd xo = VectorXcd::Zero(6);

    const double dt = 2e-5;
    const double f0 = 50.0;
    const double t0 = 0.0;
    const double tEnd = 0.2;
    const double C = 1.758e-3;

    const int N = static_cast<int>((tEnd - t0) / dt) + 1;
    const int nGroups = static_cast<int>(Cd.rows() / 3);

    mmc.reset();
    mmc.setSystemMatrices(Ad, Bd, Cd, Dd);
    mmc.setSwitchData(swOnRes, swOffRes, swType);
    mmc.setParameters(dt, f0, C, 5, 9);
    mmc.setInitialState(xo);

    std::vector<double> time(N);
    Eigen::MatrixXi brkHistory(N, 3);
    std::vector<Eigen::MatrixXd> XabcHist(nGroups, Eigen::MatrixXd::Zero(N, 3));

    Eigen::MatrixXd IupAbcHist(N, 3);
    Eigen::MatrixXd IlowAbcHist(N, 3);
    Eigen::MatrixXd VcUpAbcHist(N, 3);
    Eigen::MatrixXd VcLowAbcHist(N, 3);
    Eigen::MatrixXd VoutUpAbcHist(N, 3);
    Eigen::MatrixXd VoutLowAbcHist(N, 3);

    for (int k = 0; k < N; ++k)
    {
        const double t = t0 + k * dt;
        const double theta = 2.0 * PI * f0 * t;
        time[k] = t;

        VectorXi brkVec(3);
        brkVec.setZero();
        if (t >= 2e-4 && t < 6e-4) {
            brkVec.setOnes();
        }

        brkHistory.row(k) = brkVec.transpose();

        MatrixXcd u1(3, 5);
        u1 <<
            cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0),
            cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0),
            cd(100, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0);

        MatrixXcd u2(3, 5);
        u2 <<
            cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0),
            cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0),
            cd(100, 0), cd(0, 0), cd(0, 0), cd(0, 0), cd(0, 0);

        Simple_MMC::StepResult result = mmc.step(u1, u2, brkVec);

        for (int g = 0; g < nGroups; ++g)
        {
            MatrixXcd block = result.y_mmc.block(3 * g, 0, 3, result.y_mmc.cols());
            Eigen::Vector3d xabc = dqVis.dqn2abc_at_time(block, theta);
            XabcHist[g].row(k) = xabc.transpose();
        }

        IupAbcHist.row(k) = dqVis.dqn2abc_at_time(result.i_up, theta).transpose();
        IlowAbcHist.row(k) = dqVis.dqn2abc_at_time(result.i_low, theta).transpose();
        VcUpAbcHist.row(k) = dqVis.dqn2abc_at_time(result.vc_up, theta).transpose();
        VcLowAbcHist.row(k) = dqVis.dqn2abc_at_time(result.vc_low, theta).transpose();
        VoutUpAbcHist.row(k) = dqVis.dqn2abc_at_time(result.vout_up, theta).transpose();
        VoutLowAbcHist.row(k) = dqVis.dqn2abc_at_time(result.vout_low, theta).transpose();
    }

    {
        std::ofstream f("DQsymSimpleMMC_output.csv");
        if (!f) {
            std::cerr << "ERROR: cannot open DQsymSimpleMMC_output.csv\n";
            return;
        }

        f << std::setprecision(17);
        write_combined_header(f, nGroups);

        for (int k = 0; k < N; ++k)
        {
            f << time[k] << ","
                << brkHistory(k, 0) << ","
                << brkHistory(k, 1) << ","
                << brkHistory(k, 2);

            for (int g = 0; g < nGroups; ++g)
            {
                f << ","
                    << XabcHist[g](k, 0) << ","
                    << XabcHist[g](k, 1) << ","
                    << XabcHist[g](k, 2);
            }

            write_triplet(f, IupAbcHist.row(k).transpose());
            write_triplet(f, IlowAbcHist.row(k).transpose());
            write_triplet(f, VcUpAbcHist.row(k).transpose());
            write_triplet(f, VcLowAbcHist.row(k).transpose());
            write_triplet(f, VoutUpAbcHist.row(k).transpose());
            write_triplet(f, VoutLowAbcHist.row(k).transpose());
            f << "\n";
        }
    }

    std::cout << "Wrote DQsymSimpleMMC_output.csv\n";

    {
        std::ofstream gp("plot_DQsymSimpleMMC.gp");
        if (!gp) {
            std::cerr << "ERROR: cannot open plot_DQsymSimpleMMC.gp\n";
            return;
        }

        gp << "set datafile separator ','\n";
        gp << "set grid\n";
        gp << "set key outside\n";
        gp << "set xlabel 'Time (s)'\n";
        gp << "set terminal pngcairo size 1400,1600\n";
        gp << "set output 'DQsymSimpleMMC.png'\n";
        gp << "set multiplot layout " << (nGroups + 2) << ",1 title 'DQsym Simple_MMC outputs'\n";

        for (int g = 0; g < nGroups; ++g)
        {
            const int colA = 5 + 3 * g;
            const int colB = colA + 1;
            const int colC = colA + 2;

            gp << "set ylabel 'set " << (g + 1) << "'\n";
            gp << "plot 'DQsymSimpleMMC_output.csv' using 1:" << colA << " with lines title 'xa" << (g + 1) << "',\\\n";
            gp << "     'DQsymSimpleMMC_output.csv' using 1:" << colB << " with lines title 'xb" << (g + 1) << "',\\\n";
            gp << "     'DQsymSimpleMMC_output.csv' using 1:" << colC << " with lines title 'xc" << (g + 1) << "'\n";
        }

        gp << "set ylabel 'Vc'\n";
        gp << "plot 'DQsymSimpleMMC_output.csv' using 1:23 with lines title 'VcUp_a',\\\n";
        gp << "     'DQsymSimpleMMC_output.csv' using 1:24 with lines title 'VcUp_b',\\\n";
        gp << "     'DQsymSimpleMMC_output.csv' using 1:25 with lines title 'VcUp_c',\\\n";
        gp << "     'DQsymSimpleMMC_output.csv' using 1:26 with lines title 'VcLow_a',\\\n";
        gp << "     'DQsymSimpleMMC_output.csv' using 1:27 with lines title 'VcLow_b',\\\n";
        gp << "     'DQsymSimpleMMC_output.csv' using 1:28 with lines title 'VcLow_c'\n";

        gp << "set ylabel 'Vout->MMC'\n";
        gp << "plot 'DQsymSimpleMMC_output.csv' using 1:29 with lines title 'VoutUp_a',\\\n";
        gp << "     'DQsymSimpleMMC_output.csv' using 1:30 with lines title 'VoutUp_b',\\\n";
        gp << "     'DQsymSimpleMMC_output.csv' using 1:31 with lines title 'VoutUp_c',\\\n";
        gp << "     'DQsymSimpleMMC_output.csv' using 1:32 with lines title 'VoutLow_a',\\\n";
        gp << "     'DQsymSimpleMMC_output.csv' using 1:33 with lines title 'VoutLow_b',\\\n";
        gp << "     'DQsymSimpleMMC_output.csv' using 1:34 with lines title 'VoutLow_c'\n";

        gp << "unset multiplot\n";
        gp << "unset output\n";
    }

#ifdef _WIN32
    std::system("start \"\" gnuplot -p plot_DQsymSimpleMMC.gp");
#else
    std::system("gnuplot -p plot_DQsymSimpleMMC.gp > /dev/null 2>&1 &");
#endif

}