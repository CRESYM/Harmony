#include "Examples.h"
#include "../network.h"
#include "../Bus.h"
#include "../Include_components.h"
#include "../Solver/DQsym/DQsym.h"

void example_DQsym_validation() {
    std::cout << "=== Time-domain validation ===\n\n";

    // Same parameters as DQsym
    const double f = 50.0;
    const double omega = 2.0 * M_PI * f;
    const double Vdc = 400.0;
    const double Vm = 200.0;
    const double Pref = 1500.0;
    const double iSigma_z_ref = Pref / (3.0 * Vdc);

    std::vector<double> converter_params = {
        omega, Pref, 0.0, 0.0, Vm, Pref, Vdc,
        52.9e-3, 166.3e-3, 1.7568e-3, 1, 0.0, 10.0, 0.0
    };

    std::vector<double> controller_params = {
        0, 0,
        1, 0, 6.6667e-04, 3.3333e-01, 1, Pref,
        0, 0, 0,
        1, 0, 19.93, 4500, 1, iSigma_z_ref,
        1, 0, 117.93, 8.5e4, 2, 0, 0,
        0, 0
    };

    MMC* mmc = new MMC("MMC_TD", "AC1_DC1", converter_params, controller_params);

    // SKIP solveEquilibrium — start integration from zero
    const int n_states = mmc->getNumberOfInternalStates();
    const int ip = n_states - 12;

    std::cout << "n_states = " << n_states << ", plant block at offset " << ip << "\n\n";

    // Time-domain ODE simulation
    const double dt = 2e-5;
    const double t_end = 5.0;
    const int N = static_cast<int>(t_end / dt) + 1;

    Eigen::VectorXd x = Eigen::VectorXd::Zero(n_states);
    Eigen::VectorXd u(3);
    u << Vdc, Vm, 0.0;

    std::vector<double> time(N);
    std::vector<Eigen::MatrixXd> abcHist(4, Eigen::MatrixXd::Zero(N, 3));

    auto dqz_to_abc = [&](double d, double q, double z, double theta) {
        Eigen::Vector3d abc;
        abc(0) = d * std::cos(theta) - q * std::sin(theta) + z;
        abc(1) = d * std::cos(theta - 2.0 * M_PI / 3.0) - q * std::sin(theta - 2.0 * M_PI / 3.0) + z;
        abc(2) = d * std::cos(theta + 2.0 * M_PI / 3.0) - q * std::sin(theta + 2.0 * M_PI / 3.0) + z;
        return abc;
        };

    std::cout << "Running time-domain integration (" << N << " steps)...\n";
    for (int k = 0; k < N; ++k) {
        double t = k * dt;
        time[k] = t;
        double theta = omega * t;

        double iDelta_d = x(ip + 0);
        double iDelta_q = x(ip + 1);
        double iSigma_z = x(ip + 2);
        double iSigma_d = x(ip + 3);
        double iSigma_q = x(ip + 4);
        double vCDelta_d = x(ip + 5);
        double vCDelta_q = x(ip + 6);
        double vCSigma_d = x(ip + 9);
        double vCSigma_q = x(ip + 10);
        double vCSigma_z = x(ip + 11);

        Eigen::Vector3d iD_abc = dqz_to_abc(iDelta_d, iDelta_q, 0.0, theta);
        Eigen::Vector3d iS_abc = dqz_to_abc(iSigma_d, iSigma_q, iSigma_z, 2.0 * theta);
        Eigen::Vector3d vCD_abc = dqz_to_abc(vCDelta_d, vCDelta_q, 0.0, theta);
        Eigen::Vector3d vCS_abc = dqz_to_abc(vCSigma_d, vCSigma_q, vCSigma_z, 2.0 * theta);

        abcHist[0].row(k) = iD_abc.transpose();
        abcHist[1].row(k) = iS_abc.transpose();
        abcHist[2].row(k) = vCD_abc.transpose();
        abcHist[3].row(k) = vCS_abc.transpose();

        if (k < N - 1) {
            Eigen::VectorXd dxdt = mmc->computeStateDerivatives(x, u);
            x += dxdt * dt;

            // Check for blow-up
            if (!x.allFinite()) {
                std::cout << "Diverged at step " << k << " (t=" << t << ")\n";
                break;
            }
        }
    }
    std::cout << "Integration complete.\n\n";

    std::cout << "Final plant variables:\n";
    std::cout << "  iDelta_d   = " << x(ip + 0) << "\n";
    std::cout << "  iDelta_q   = " << x(ip + 1) << "\n";
    std::cout << "  iSigma_z   = " << x(ip + 2) << "\n";
    std::cout << "  vCSigma_z  = " << x(ip + 11) << "\n\n";

    plot_abc_groups_implot(time, abcHist, "Time-domain trajectory (abc)");

    std::cout << "Press Enter to continue...\n";
    std::cin.get();
}