#include <gtest/gtest.h>

#include "Elements/RES/PV_generic.h"
#include "Elements/RES/PV_plant.h"

#include <cmath>
#include <complex>
#include <vector>

namespace {

constexpr double kQ = kPvElectronCharge;
constexpr double kB = kPvBoltzmann;

MatrixXcd yFromAbcd(
    const Eigen::MatrixXd& A,
    const Eigen::MatrixXd& B,
    const Eigen::MatrixXd& C,
    const Eigen::MatrixXd& D,
    double frequency)
{
    const std::complex<double> s(0.0, 2.0 * M_PI * frequency);
    const int n = static_cast<int>(A.rows());
    return C.cast<std::complex<double>>()
        * (s * Eigen::MatrixXcd::Identity(n, n) - A.cast<std::complex<double>>()).inverse()
        * B.cast<std::complex<double>>()
        + D.cast<std::complex<double>>();
}

MatrixXcd toEigen(const std::vector<std::vector<std::complex<double>>>& Y) {
    MatrixXcd out(static_cast<int>(Y.size()), static_cast<int>(Y[0].size()));
    for (int r = 0; r < out.rows(); ++r)
        for (int c = 0; c < out.cols(); ++c)
            out(r, c) = Y[r][c];
    return out;
}

// Ideal-diode slope used by the existing PVplant (Rsr = 0, Rsh open, Ig = Isc).
double harmonyKpv(double Ns, double Np, double n, double Isc, double I0, double I, double T) {
    return -kQ * (Np * I0 + Np * Isc - I) / (Ns * n * kB * T);
}

PVgenericParameters demoPlant() {
    PVgenericParameters p;
    p.pv.Ns = 100.0;
    p.pv.Np = 20.0;
    p.pv.n = 1.3;
    p.pv.Isc_ref = 8.0;
    p.pv.Io_ref = 1e-9;
    p.pv.alpha_T = 0.0;
    p.pv.Eg = 1.12;
    p.pv.Tc = 298.15;
    p.pv.Tc_ref = 298.15;
    p.pv.G = 1000.0;
    p.pv.G_ref = 1000.0;
    p.pv.Rsr = 0.05;
    p.pv.Rsh = 80.0;
    p.C_pv = 1e-3;
    p.L_boost = 1e-3;
    p.C_dc = 5e-3;
    p.Vdc_ref = 400.0;
    p.Qref = 0.0;
    p.Dmax = 0.95;
    p.kp_boost = 1e-6;
    p.ki_boost = 0.05;
    p.tau_boost = 0.0;
    p.L1 = 200e-6;
    p.R1 = 0.02;
    p.Cf = 50e-6;
    p.Rc = 0.3;
    p.L2 = 0.0;
    p.Vm = 230.0 * std::sqrt(2.0 / 3.0);
    p.theta = 0.0;
    p.omega = 2.0 * M_PI * 50.0;
    p.kp_dc = 0.4;
    p.ki_dc = 30.0;
    p.kp_q = 1e-4;
    p.ki_q = 0.02;
    p.kp_i = 0.4;
    p.ki_i = 60.0;
    p.kp_pll = 8.0;
    p.ki_pll = 150.0;
    p.k_ff = 0.0;
    const PVarray array(p.pv);
    const double voc = array.openCircuitVoltage();
    const double voltage = 0.94 * voc;
    p.Pref = voltage * array.terminalCurrent(voltage);
    return p;
}

void expectYdq(PVgeneric& plant) {
    ASSERT_NO_THROW(plant.solveEquilibrium());
    const Eigen::VectorXd& x = plant.getEquilibriumState();
    const Eigen::VectorXd& u = plant.operatingInput();
    const Eigen::VectorXd residual = plant.computeStateDerivatives(x, u);
    EXPECT_LT(residual.lpNorm<Eigen::Infinity>(), 1e-4);

    const auto point = plant.operatingPoint();
    EXPECT_NEAR(point.p_conv, plant.parameters().Pref, 1e-4 * std::max(1.0, plant.parameters().Pref));
    EXPECT_NEAR(point.q_pcc, plant.parameters().Qref, 1e-4 * std::max(1.0, std::abs(plant.parameters().Qref) + 1.0));
    EXPECT_NEAR(point.v_pv * point.i_array, plant.parameters().Pref, 1e-4 * std::max(1.0, plant.parameters().Pref));
    EXPECT_GT(point.duty, 0.0);
    EXPECT_LT(point.duty, plant.parameters().Dmax);

    ASSERT_NO_THROW(plant.computeABCD());
    const int v = plant.pvVoltageIndex();
    const int i_L = plant.boostCurrentIndex();
    const double g = point.g_pv;
    EXPECT_NEAR(plant.getA()(v, v), -g / plant.parameters().C_pv, 1e-4 * std::max(1.0, std::abs(g / plant.parameters().C_pv)));
    EXPECT_NEAR(plant.getA()(v, i_L), -1.0 / plant.parameters().C_pv, 1e-4);
    for (int col = 0; col < plant.getA().cols(); ++col) {
        if (col == v || col == i_L)
            continue;
        EXPECT_NEAR(plant.getA()(v, col), 0.0, 1e-5) << "column " << col;
    }

    const double frequency = 50.0;
    const MatrixXcd Y = toEigen(plant.compute_y_parameters(frequency));
    EXPECT_EQ(Y.rows(), 2);
    EXPECT_EQ(Y.cols(), 2);
    EXPECT_TRUE(Y.allFinite());
    const MatrixXcd Y_ref = yFromAbcd(plant.getA(), plant.getB(), plant.getC(), plant.getD(), frequency);
    EXPECT_TRUE(Y.isApprox(Y_ref, 1e-8)) << Y << "\n" << Y_ref;
    const MatrixXcd Y_repeat = toEigen(plant.compute_y_parameters(frequency));
    EXPECT_TRUE(Y.isApprox(Y_repeat, 1e-12));
    EXPECT_TRUE(toEigen(plant.compute_y_parameters(5.0)).allFinite());
}

} // namespace

TEST(PVarray, IdealDiodeMatchesHarmonySlope) {
    // Operating point of example_PV_plant: the fitted ideal diode passes through (Vmpp, Impp).
    PVarrayParameters p;
    p.Ns = 1044.0;
    p.Np = 656.0;
    p.n = 1.9267367660446415;
    p.Isc_ref = 3.35;
    p.Io_ref = 1.7250567519501773e-05;
    p.Tc = 298.15;
    p.Tc_ref = 298.15;
    p.G = 1000.0;
    p.G_ref = 1000.0;
    p.Rsr = 0.0;
    p.Rsh = 0.0;
    const PVarray array(p);
    const double V = 1009603.68 / 2000.8;
    const double I = array.terminalCurrent(V);
    EXPECT_NEAR(I, 2000.8, 1e-6);
    const double k_pv = harmonyKpv(p.Ns, p.Np, p.n, p.Isc_ref, p.Io_ref, I, p.Tc);
    EXPECT_NEAR(array.dCurrentDVoltage(V), k_pv, 1e-8 * std::max(1.0, std::abs(k_pv)));
    EXPECT_NEAR(array.incrementalConductance(V), -k_pv, 1e-8 * std::max(1.0, std::abs(k_pv)));

    const double dv = 1e-4;
    const double slope = (array.terminalCurrent(V + dv) - array.terminalCurrent(V - dv)) / (2.0 * dv);
    EXPECT_NEAR(array.dCurrentDVoltage(V), slope, 1e-6 * std::max(1.0, std::abs(slope)));
}

TEST(PVarray, SeriesShuntFollowsPscadEquation) {
    PVarrayParameters p;
    p.Ns = 36.0;
    p.Np = 4.0;
    p.n = 1.2;
    p.Isc_ref = 5.2;
    p.Io_ref = 2e-9;
    p.alpha_T = 0.003;
    p.Eg = 1.12;
    p.Tc = 308.15;
    p.Tc_ref = 298.15;
    p.G = 800.0;
    p.G_ref = 1000.0;
    p.Rsr = 0.35;
    p.Rsh = 120.0;
    const PVarray array(p);

    const double Ig = (p.G / p.G_ref) * (p.Isc_ref + p.alpha_T * (p.Tc - p.Tc_ref));
    const double Io = p.Io_ref * std::pow(p.Tc / p.Tc_ref, 3.0)
        * std::exp(kQ * p.Eg / (p.n * kB) * (1.0 / p.Tc_ref - 1.0 / p.Tc));
    EXPECT_NEAR(array.photoCurrent(), Ig, 1e-12);
    EXPECT_NEAR(array.saturationCurrent(), Io, 1e-18 * std::max(1.0, Io));

    const double V = 16.0;
    const double I = array.terminalCurrent(V);
    const double VD = V / p.Ns + I * p.Rsr / p.Np;
    const double thermal = kQ / (p.n * kB * p.Tc);
    const double predicted = p.Np * (Ig - Io * (std::exp(thermal * VD) - 1.0) - VD / p.Rsh);
    EXPECT_NEAR(I, predicted, 1e-8 * std::max(1.0, std::abs(predicted)));

    const double dv = 1e-5;
    const double slope = (array.terminalCurrent(V + dv) - array.terminalCurrent(V - dv)) / (2.0 * dv);
    EXPECT_NEAR(array.dCurrentDVoltage(V), slope, 1e-5 * std::max(1.0, std::abs(slope)));

    const double Gd = thermal * Io * std::exp(thermal * VD);
    const double g = (p.Np / p.Ns) * (Gd + 1.0 / p.Rsh) / (1.0 + p.Rsr * (Gd + 1.0 / p.Rsh));
    EXPECT_NEAR(array.incrementalConductance(V), g, 1e-8 * std::max(1.0, g));

    // Same terminal current fed to the ideal Harmony slope does not reproduce g.
    const double ideal = -harmonyKpv(p.Ns, p.Np, p.n, Ig, Io, I, p.Tc);
    EXPECT_GT(std::abs(g - ideal) / g, 1e-3);
}

std::vector<double> legacyPack(double power, double current) {
    return {
        power, current, 1044.0, 656.0, 1.9267367660446415, 3.35,
        1.7250567519501773e-05, 7.2e-3, 800.0, 250e-6, 32e-3, 0.002, 0.04,
        63e-6, 0.0, 1500e-6, 0.051, 0.0, 315.0, 50.0,
        16.20032898666123, 324.0065797332246, 0.19845, 9.9225,
        0.732885617235351, 4.88590411490234
    };
}

// Independent of PVplant: ideal diode, dP/dV = 0.
double idealMaximumPowerVoltage(double Ns, double Np, double n, double Isc, double I0, double T) {
    const double Vt = Ns * n * kB * T / kQ;
    const double Voc = Vt * std::log(Isc / I0 + 1.0);
    auto powerSlope = [&](double V) {
        const double e = std::exp(V / Vt);
        const double I = Np * (Isc - I0 * (e - 1.0));
        const double dIdV = -Np * I0 / Vt * e;
        return I + V * dIdV;
    };
    double low = 1e-4 * Voc;
    double high = Voc * (1.0 - 1e-8);
    for (int i = 0; i < 80; ++i) {
        const double mid = 0.5 * (low + high);
        if (powerSlope(mid) > 0.0)
            low = mid;
        else
            high = mid;
    }
    return 0.5 * (low + high);
}

TEST(PVplant, AdmittanceUsesDiodeMaximumPowerPoint) {
    constexpr double Ns = 1044.0;
    constexpr double Np = 656.0;
    constexpr double n = 1.9267367660446415;
    constexpr double Isc = 3.35;
    constexpr double I0 = 1.7250567519501773e-05;
    constexpr double T = 298.15;
    const double V = idealMaximumPowerVoltage(Ns, Np, n, Isc, I0, T);
    const double Vt = Ns * n * kB * T / kQ;
    const double e = std::exp(V / Vt);
    const double I = Np * (Isc - I0 * (e - 1.0));
    const double dPdV = I + V * (-Np * I0 / Vt * e);
    EXPECT_NEAR(dPdV, 0.0, 1e-4);
    EXPECT_GT(std::abs(I - 2000.8), 1.0);

    const double fitted = 1009603.68 / 2000.8;
    const double e_fit = std::exp(fitted / Vt);
    const double I_fit = Np * (Isc - I0 * (e_fit - 1.0));
    const double dPdV_fit = I_fit + fitted * (-Np * I0 / Vt * e_fit);
    EXPECT_GT(dPdV_fit, 0.0);

    PVplant at_peak("PV_peak", "AC1", legacyPack(V * I, I));
    PVplant supplied("PV_supplied", "AC1", legacyPack(1009603.68, 2000.8));
    const auto Y_peak = at_peak.compute_y_parameters(10.0);
    const auto Y_supplied = supplied.compute_y_parameters(10.0);
    ASSERT_EQ(Y_peak.size(), 2u);
    ASSERT_EQ(Y_supplied.size(), 2u);
    for (int r = 0; r < 2; ++r) {
        ASSERT_EQ(Y_peak[r].size(), 2u);
        for (int c = 0; c < 2; ++c) {
            EXPECT_NEAR(Y_supplied[r][c].real(), Y_peak[r][c].real(), 1e-6);
            EXPECT_NEAR(Y_supplied[r][c].imag(), Y_peak[r][c].imag(), 1e-6);
            EXPECT_TRUE(std::isfinite(Y_peak[r][c].real()));
            EXPECT_TRUE(std::isfinite(Y_peak[r][c].imag()));
        }
    }
    EXPECT_GT(std::abs(Y_peak[0][0]), 1.0);
}

TEST(PVgeneric, EquilibriumProducesYdq) {
    PVgenericParameters parameters = demoPlant();
    PVgeneric plant("PVG", "AC1", parameters);
    expectYdq(plant);
    EXPECT_EQ(plant.numberOfStates(), 14);
}

TEST(PVgeneric, LclAngleAndPowerFilterProduceYdq) {
    PVgenericParameters parameters = demoPlant();
    parameters.L2 = 150e-6;
    parameters.theta = 0.35;
    parameters.Qref = 400.0;
    parameters.k_ff = 1.0;
    parameters.tau_boost = 0.02;
    parameters.Rc = 0.05;
    PVgeneric plant("PVG2", "AC1", parameters);
    expectYdq(plant);
    EXPECT_EQ(plant.numberOfStates(), 17);
}
