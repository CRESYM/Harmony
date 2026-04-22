#include "Examples.h"

#include "../Solver/DQsym/DQsym.h"
#include "../Solver/Helper_Functions/Helper_Functions.h"

void example_DQsym_RLC()
{
    using cd = std::complex<double>;

    DQsym dq;
    //dq.reset();

    Eigen::MatrixXcd Ad(6, 6);
    Ad <<
        cd(0.999055, 0), cd(0, 0), cd(0, 0), cd(-0.00900385, 0), cd(0, 0), cd(0, 0),
        cd(0, 0), cd(0.999055, 0), cd(0, 0), cd(0, 0), cd(-0.00900385, 0), cd(0, 0),
        cd(0, 0), cd(0, 0), cd(0.999055, 0), cd(0, 0), cd(0, 0), cd(-0.00900385, 0),
        cd(0.00999427, 0), cd(0, 0), cd(0, 0), cd(0.999755, 0), cd(0, 0), cd(0, 0),
        cd(0, 0), cd(0.00999427, 0), cd(0, 0), cd(0, 0), cd(0.999755, 0), cd(0, 0),
        cd(0, 0), cd(0, 0), cd(0.00999427, 0), cd(0, 0), cd(0, 0), cd(0.999755, 0);

    Eigen::MatrixXcd Bd(6, 3);
    Bd <<
        cd(450.238, 0), cd(0, 0), cd(0, 0),
        cd(0, 0), cd(450.238, 0), cd(0, 0),
        cd(0, 0), cd(0, 0), cd(450.238, 0),
        cd(2.25096, 0), cd(0, 0), cd(0, 0),
        cd(0, 0), cd(2.25096, 0), cd(0, 0),
        cd(0, 0), cd(0, 0), cd(2.25096, 0);

    Eigen::MatrixXcd Cd(6, 6);
    Cd <<
        cd(9.99427e-08, 0), cd(0, 0), cd(0, 0), cd(1.99976e-05, 0), cd(0, 0), cd(0, 0),
        cd(0, 0), cd(9.99427e-08, 0), cd(0, 0), cd(0, 0), cd(1.99976e-05, 0), cd(0, 0),
        cd(0, 0), cd(0, 0), cd(9.99427e-08, 0), cd(0, 0), cd(0, 0), cd(1.99976e-05, 0),
        cd(-1.99905e-05, 0), cd(0, 0), cd(0, 0), cd(9.00385e-08, 0), cd(0, 0), cd(0, 0),
        cd(0, 0), cd(-1.99905e-05, 0), cd(0, 0), cd(0, 0), cd(9.00385e-08, 0), cd(0, 0),
        cd(0, 0), cd(0, 0), cd(-1.99905e-05, 0), cd(0, 0), cd(0, 0), cd(9.00385e-08, 0);

    Eigen::MatrixXcd Dd(6, 3);
    Dd <<
        cd(2.25096e-05, 0), cd(0, 0), cd(0, 0),
        cd(0, 0), cd(2.25096e-05, 0), cd(0, 0),
        cd(0, 0), cd(0, 0), cd(2.25096e-05, 0),
        cd(-0.00450238, 0), cd(0, 0), cd(0, 0),
        cd(0, 0), cd(-0.00450238, 0), cd(0, 0),
        cd(0, 0), cd(0, 0), cd(-0.00450238, 0);

    Eigen::VectorXd swOnRes(3);
    swOnRes << 0.01, 0.01, 0.01;

    Eigen::VectorXd swOffRes(3);
    swOffRes << 1e6, 1e6, 1e6;

    Eigen::VectorXi swType(3);
    swType << 0, 0, 0;

    Eigen::VectorXcd xo = Eigen::VectorXcd::Zero(6);

    const double dt = 2e-5;
    const double f0 = 50.0;
    const double t0 = 0.0;
    const double tEnd = 0.5;

    const int N = static_cast<int>((tEnd - t0) / dt) + 1;
    const int nGroups = static_cast<int>(Cd.rows() / 3);

    std::vector<double> time(N);
    Eigen::MatrixXi brkHistory(N, 3);
    std::vector<Eigen::MatrixXd> XabcHist(nGroups, Eigen::MatrixXd::Zero(N, 3));

    Eigen::MatrixXcd u(3, 6);
    u <<
        cd(0, 0), cd(-245.5, -880.19999999999993), cd(-102.09999999999999, 319.40000000000003), cd(373.90000000000003, 733.79999999999995), cd(255.09999999999999, -885.10000000000002), cd(161.09999999999999, 983.89999999999998),
        cd(0, 0), cd(-216.5, 514.10000000000002), cd(-400.19999999999999, 211.40000000000001), cd(-82.400000000000006, -141.30000000000001), cd(-536.90000000000009, 212.40000000000001), cd(-515.59999999999991, -399.89999999999998),
        cd(0, 0), cd(-119.40000000000001, -115.7), cd(-335.69999999999999, -128.80000000000001), cd(-889.59999999999991, -92.899999999999991), cd(3.2000000000000002, -257.60000000000002), cd(242.69999999999999, 301.70000000000005);

    for (int k = 0; k < N; ++k)
    {
        const double t = t0 + k * dt;
        const double theta = 2.0 * M_PI * f0 * t;
        time[k] = t;

        Eigen::VectorXi brkVec(3);
        brkVec.setZero();
        if (t >= 2e-4 && t < 6e-4) {
            brkVec.setOnes();
        }

        brkHistory.row(k) = brkVec.transpose();


        Eigen::MatrixXcd y = dq.DSSS(
            Ad, Bd, Cd, Dd,
            swOnRes, swOffRes,
            swType, brkVec,
            u, xo,
            dt, f0
        );

        std::vector<Eigen::Vector3d> abcGroups = dqn2abc_groups_at_time(y, theta);
        for (int g = 0; g < nGroups; ++g) {
            XabcHist[g].row(k) = abcGroups[g].transpose();
        }
    }

    
    std::vector<Eigen::MatrixXd> values;

    values.push_back(brkHistory.cast<double>());

    for (int g = 0; g < nGroups; ++g)
    {
        values.push_back(XabcHist[g]);
    }

    std::vector<std::string> headers;
    headers.push_back("brk");

    for (int g = 0; g < nGroups; ++g)
    {
        headers.push_back("xa" + std::to_string(g + 1));
    }

    write_file(time, values, headers, "DQsymRLC_abc_output.csv");
    

    std::cout << "Wrote DQsymRLC_abc_output.csv\n";

    plot_abc_groups_implot(time, XabcHist, "DQsym RLC outputs converted to abc");

    cout << "Press Enter to continue...\n";
    cin.get();
	
}