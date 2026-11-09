#include <gtest/gtest.h>

#include "Elements/Converter/TwoLC.h"

#include <cmath>
#include <vector>

namespace {

std::vector<double> casePack(bool vdc_control) {
    const double s_va = 100e6;
    const double vll = 220e3;
    const double vdc = 640e3;
    const double vm = vll * std::sqrt(2.0 / 3.0);
    const double zac = vll * vll / s_va;
    const double iac = 2.0 * s_va / (3.0 * vm);
    const double omega = 2.0 * M_PI * 50.0;
    const double zbase = 220.0 * 220.0 / 100.0;
    const double L = 0.08 * zbase / omega;
    const double R = 0.01 * 0.08 * zbase;
    const double P = -70e6;
    const double Q = -20e6;
    std::vector<double> conv = {
        omega, P, Q, 0.0, vm, -P, vdc, L, R, 200e-6, 3.0
    };
    const double kp_pll = 0.397887357729738 * omega / vm;
    const double ki_pll = 7.957747154594767 * omega / vm;
    const double kp_i = 0.254647908947033 * zac;
    const double ki_i = 0.8 * zac;
    const double kp_outer = 0.04 * iac / s_va;
    const double ki_outer = 40.0 * iac / s_va;
    const double kp_v = 5.0 * iac / vdc;
    const double ki_v = 5.0 * iac / vdc;
    std::vector<double> ctrl = { 1, 0, kp_pll, ki_pll, 1, 0 };
    if (vdc_control) {
        ctrl.insert(ctrl.end(), { 1, 0, kp_v, ki_v, 1, vdc, 0 });
    }
    else {
        ctrl.insert(ctrl.end(), { 0, 1, 0, kp_outer, ki_outer, 1, P });
    }
    ctrl.insert(ctrl.end(), { 0, 1, 0, kp_outer, ki_outer, 1, Q, 0, 0 });
    ctrl.insert(ctrl.end(), { 1, 0, kp_i, ki_i, 2, 0.0, 0.0, 0, 0, 0 });
    conv.insert(conv.end(), ctrl.begin(), ctrl.end());
    return conv;
}

TwoLC make(bool vdc_control) {
    const auto packed = casePack(vdc_control);
    const std::vector<double> conv(packed.begin(), packed.begin() + 11);
    const std::vector<double> ctrl(packed.begin() + 11, packed.end());
    return TwoLC("c2lc", "AC1_DC1", conv, ctrl);
}

}

TEST(TwoLC, PqEquilibriumIsConsistent) {
    TwoLC converter = make(false);
    EXPECT_NO_THROW(converter.solveEquilibrium());
    converter.computeABCD();
    const auto Y = converter.compute_y_parameters(50.0);
    EXPECT_EQ(Y.size(), 3u);
    EXPECT_TRUE(std::isfinite(Y[1][1].real()));
    EXPECT_GT(std::abs(Y[1][1]), 0.0);
}

TEST(TwoLC, VdcEquilibriumIsConsistent) {
    TwoLC converter = make(true);
    EXPECT_NO_THROW(converter.solveEquilibrium());
    converter.computeABCD();
    const auto Y = converter.compute_y_parameters(50.0);
    EXPECT_TRUE(std::isfinite(Y[0][0].real()));
}
