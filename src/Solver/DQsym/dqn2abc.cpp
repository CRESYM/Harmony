#include "dqn2abc.h"

#include <iostream>
#include <cmath>
#include <stdexcept>

using Eigen::Matrix3cd;
using Eigen::MatrixXcd;
using Eigen::Vector3d;
using std::complex;

constexpr double PI = 3.141592653589793238462643383279502884;

//------------------------------------------------------------------------------
// Reconstruct abc waveform at a single angle theta
//------------------------------------------------------------------------------
Eigen::Vector3d dqn2abc_at_time(const Eigen::MatrixXcd& Xdcpnz_c, double theta)
{
    if (Xdcpnz_c.rows() != 3) {
        throw std::runtime_error("dqn2abc_at_time: Xdcpnz_c must have 3 rows.");
    }

    static bool initialized = false;
    static Matrix3cd Sas; // symmetrical-components transform
    static Matrix3cd Ssa; // inverse transform

    if (!initialized) {
        const complex<double> a(-0.5, std::sqrt(3.0) / 2.0);
        const complex<double> a2(-0.5, -std::sqrt(3.0) / 2.0);

        Sas <<
            complex<double>(1.0, 0.0), a, a2,
            complex<double>(1.0, 0.0), a2, a,
            complex<double>(1.0, 0.0), complex<double>(1.0, 0.0), complex<double>(1.0, 0.0);

        Sas /= 3.0;
        Ssa = Sas.inverse();
        initialized = true;
    }

    Vector3d Xabc = Vector3d::Zero();
    const int ncols = static_cast<int>(Xdcpnz_c.cols());

    for (int i = 0; i < ncols; ++i) {
        if (i == 0) {
            // DC / zero-order contribution
            Xabc += (Ssa * Xdcpnz_c.col(0)).real();
        }
        else {
            // Harmonic index: column i corresponds to harmonic h = i
            const int h = i;
            const double th = h * theta;

            const complex<double> Xp = Xdcpnz_c(0, i); // positive sequence
            const complex<double> Xn = Xdcpnz_c(1, i); // negative sequence
            const complex<double> Xz = Xdcpnz_c(2, i); // zero sequence

            const double mag_p = std::abs(Xp);
            const double ang_p = std::arg(Xp);

            const double mag_n = std::abs(Xn);
            const double ang_n = std::arg(Xn);

            const double mag_z = std::abs(Xz);
            const double ang_z = std::arg(Xz);

            Vector3d abc3p;
            abc3p <<
                mag_p * std::sin(th + ang_p),
                mag_p* std::sin(th + ang_p - 2.0 * PI / 3.0),
                mag_p* std::sin(th + ang_p + 2.0 * PI / 3.0);

            Vector3d abc3n;
            abc3n <<
                mag_n * std::sin(th + ang_n),
                mag_n* std::sin(th + ang_n + 2.0 * PI / 3.0),
                mag_n* std::sin(th + ang_n - 2.0 * PI / 3.0);

            Vector3d abc3z;
            abc3z <<
                mag_z * std::sin(th + ang_z),
                mag_z* std::sin(th + ang_z),
                mag_z* std::sin(th + ang_z);

            Xabc += abc3p + abc3n + abc3z;
        }
    }

    return Xabc;
}

//------------------------------------------------------------------------------
// Simulate abc waveform over time
//------------------------------------------------------------------------------
ABCResult simulate_dqn2abc(const Eigen::MatrixXcd& Xdcpnz_c,
    double freq_hz,
    double t0,
    double t1,
    double Ts)
{
    if (Xdcpnz_c.rows() != 3) {
        throw std::runtime_error("simulate_dqn2abc: Xdcpnz_c must have 3 rows.");
    }

    if (Ts <= 0.0) {
        throw std::runtime_error("simulate_dqn2abc: Ts must be > 0.");
    }

    if (t1 < t0) {
        throw std::runtime_error("simulate_dqn2abc: t1 must be >= t0.");
    }

    const int N = static_cast<int>((t1 - t0) / Ts) + 1;

    ABCResult res;
    res.t.resize(N);
    res.Xabc = Eigen::MatrixXd::Zero(N, 3);

    for (int k = 0; k < N; ++k) {
        const double t = t0 + static_cast<double>(k) * Ts;
        const double theta = 2.0 * PI * freq_hz * t;

        res.t[k] = t;
        res.Xabc.row(k) = dqn2abc_at_time(Xdcpnz_c, theta).transpose();
    }

    return res;
}

//------------------------------------------------------------------------------
// Simple test runner
//------------------------------------------------------------------------------
void run_dqn2abc()
{
    const double freq_hz = 50.0;
    const double t0 = 0.0;
    const double t1 = 1.0;
    const double Ts = 2e-5;

    Eigen::MatrixXcd Xdcpnz_c(3, 6);
    Xdcpnz_c <<
        std::complex<double>(0.0, 0.0), std::complex<double>(-0.2455, -0.8802), std::complex<double>(-0.1021, 0.3194), std::complex<double>(0.3739, 0.7338), std::complex<double>(0.2551, -0.8851), std::complex<double>(0.1611, 0.9839),
        std::complex<double>(0.0, 0.0), std::complex<double>(-0.2165, 0.5141), std::complex<double>(-0.4002, 0.2114), std::complex<double>(-0.0824, -0.1413), std::complex<double>(-0.5369, 0.2124), std::complex<double>(-0.5156, -0.3999),
        std::complex<double>(0.0, 0.0), std::complex<double>(-0.1194, -0.1157), std::complex<double>(-0.3357, -0.1288), std::complex<double>(-0.8896, -0.0929), std::complex<double>(0.0032, -0.2576), std::complex<double>(0.2427, 0.3017);

    ABCResult res = simulate_dqn2abc(Xdcpnz_c, freq_hz, t0, t1, Ts);

    std::cout << "dqn2abc simulation complete.\n";
    std::cout << "Generated " << res.t.size() << " samples.\n";
    std::cout << "First sample: "
        << "xa = " << res.Xabc(0, 0) << ", "
        << "xb = " << res.Xabc(0, 1) << ", "
        << "xc = " << res.Xabc(0, 2) << "\n";
}