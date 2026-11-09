/**
 * @file PV_generic.cpp
 * @brief PSCAD single-diode array and generic PV plant ODEs.
 */
#include "PV_generic.h"

#include "Solver/Helper_Functions/Differential_equations.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>

namespace {

constexpr double kPi = 3.14159265358979323846;

struct Park {
    double c;
    double s;
    explicit Park(double theta) : c(std::cos(theta)), s(std::sin(theta)) {}

    void toControl(double xd, double xq, double& cd, double& cq) const {
        cd = c * xd + s * xq;
        cq = -s * xd + c * xq;
    }

    void toGrid(double cd, double cq, double& xd, double& xq) const {
        xd = c * cd - s * cq;
        xq = s * cd + c * cq;
    }
};

double clampExponent(double x) {
    return std::exp(std::min(80.0, x));
}

} // namespace

PVarray::PVarray(const PVarrayParameters& parameters) : p_(parameters) {
    if (p_.Ns <= 0.0 || p_.Np <= 0.0 || p_.n <= 0.0)
        throw std::invalid_argument("PVarray Ns, Np and n must be positive.");
    if (p_.Isc_ref <= 0.0 || p_.Io_ref <= 0.0)
        throw std::invalid_argument("PVarray Isc_ref and Io_ref must be positive.");
    if (p_.Tc <= 0.0 || p_.Tc_ref <= 0.0 || p_.G_ref <= 0.0 || p_.G < 0.0)
        throw std::invalid_argument("PVarray temperature and irradiance are invalid.");
    if (p_.Rsr < 0.0)
        throw std::invalid_argument("PVarray Rsr must be non-negative.");
    if (photoCurrent() <= 0.0)
        throw std::invalid_argument("PVarray photocurrent is not positive at this operating point.");
}

double PVarray::photoCurrent() const {
    return (p_.G / p_.G_ref) * (p_.Isc_ref + p_.alpha_T * (p_.Tc - p_.Tc_ref));
}

double PVarray::saturationCurrent() const {
    const double ratio = p_.Tc / p_.Tc_ref;
    const double exponent = kPvElectronCharge * p_.Eg / (p_.n * kPvBoltzmann)
        * (1.0 / p_.Tc_ref - 1.0 / p_.Tc);
    return p_.Io_ref * std::pow(ratio, 3.0) * std::exp(exponent);
}

double PVarray::terminalCurrent(double voltage) const {
    const double Ig = photoCurrent();
    const double Io = saturationCurrent();
    const double inv_thermal = kPvElectronCharge / (p_.n * kPvBoltzmann * p_.Tc);
    auto shunt = [&](double diode_voltage) {
        return (p_.Rsh > 0.0) ? diode_voltage / p_.Rsh : 0.0;
    };

    if (p_.Rsr == 0.0) {
        const double vd = voltage / p_.Ns;
        return p_.Np * (Ig - Io * (clampExponent(inv_thermal * vd) - 1.0) - shunt(vd));
    }

    // f(I) is strictly increasing, but the exponential is steep, so Newton
    // from the Rsr = 0 guess walks too slowly. Bisect instead.
    auto residual = [&](double current) {
        const double vd = voltage / p_.Ns + current * p_.Rsr / p_.Np;
        return current / p_.Np - Ig + Io * (clampExponent(inv_thermal * vd) - 1.0) + shunt(vd);
    };
    const double span = 2.0 * p_.Np * std::max(Ig, 1.0);
    double lo = -span;
    double hi = span;
    for (int expand = 0; expand < 12 && residual(lo) > 0.0; ++expand)
        lo *= 2.0;
    for (int expand = 0; expand < 12 && residual(hi) < 0.0; ++expand)
        hi *= 2.0;
    if (residual(lo) > 0.0 || residual(hi) < 0.0)
        throw std::runtime_error("PVarray terminal current has no bracket.");
    for (int k = 0; k < 80; ++k) {
        const double mid = 0.5 * (lo + hi);
        if (residual(mid) > 0.0)
            hi = mid;
        else
            lo = mid;
        if (hi - lo <= 1e-12 * std::max(1.0, std::abs(mid)))
            return 0.5 * (lo + hi);
    }
    throw std::runtime_error("PVarray terminal current did not converge.");
}

double PVarray::incrementalConductance(double voltage) const {
    const double current = terminalCurrent(voltage);
    const double diode_voltage = voltage / p_.Ns + current * p_.Rsr / p_.Np;
    const double Io = saturationCurrent();
    const double inv_thermal = kPvElectronCharge / (p_.n * kPvBoltzmann * p_.Tc);
    const double Gd = inv_thermal * Io * clampExponent(inv_thermal * diode_voltage);
    const double gsh = (p_.Rsh > 0.0) ? 1.0 / p_.Rsh : 0.0;
    const double parallel = Gd + gsh;
    return (p_.Np / p_.Ns) * parallel / (1.0 + p_.Rsr * parallel);
}

double PVarray::openCircuitVoltage() const {
    const double thermal = p_.n * kPvBoltzmann * p_.Tc / kPvElectronCharge;
    const double Ig = photoCurrent();
    const double Io = saturationCurrent();
    double voltage = p_.Ns * thermal * std::log(Ig / Io + 1.0);
    for (int k = 0; k < 30; ++k) {
        const double current = terminalCurrent(voltage);
        if (std::abs(current) <= 1e-10 * std::max(1.0, Ig * p_.Np))
            return voltage;
        const double slope = dCurrentDVoltage(voltage);
        if (std::abs(slope) < 1e-18)
            break;
        voltage -= current / slope;
        if (voltage < 0.0)
            voltage = 0.0;
    }
    throw std::runtime_error("PVarray open-circuit voltage did not converge.");
}

double PVarray::voltageForPower(double power) const {
    if (power < 0.0)
        throw std::invalid_argument("PVarray power must be non-negative.");
    const double voc = openCircuitVoltage();
    if (power == 0.0)
        return voc;

    auto generated = [&](double voltage) { return voltage * terminalCurrent(voltage); };
    auto slope = [&](double voltage) {
        return terminalCurrent(voltage) + voltage * dCurrentDVoltage(voltage);
    };

    // dP/dV changes from positive to negative at the MPP. The power-mode
    // boost uses the high-voltage root, which can sit within a volt of the
    // peak when the request is near Pmax.
    double low = voc * 0.05;
    double high = voc * (1.0 - 1e-6);
    if (slope(low) < 0.0)
        low = voc * 1e-3;
    for (int k = 0; k < 60; ++k) {
        const double mid = 0.5 * (low + high);
        if (slope(mid) > 0.0)
            low = mid;
        else
            high = mid;
    }
    const double v_mpp = 0.5 * (low + high);
    if (generated(v_mpp) < power)
        throw std::runtime_error("PVarray cannot supply the requested power on the high-voltage side of the MPP.");

    low = v_mpp;
    high = voc;
    for (int k = 0; k < 70; ++k) {
        const double mid = 0.5 * (low + high);
        if (generated(mid) > power)
            low = mid;
        else
            high = mid;
    }
    return 0.5 * (low + high);
}

PVgeneric::PVgeneric(const std::string& symbol, const std::string& location,
    const PVgenericParameters& parameters)
    : RES_base(symbol, location), p_(parameters), array_(parameters.pv) {
    validate();
    buildLayout();
    Y_matrix.resize(2, 2);
}

void PVgeneric::validate() const {
    if (p_.C_pv <= 0.0 || p_.L_boost <= 0.0 || p_.C_dc <= 0.0)
        throw std::invalid_argument("PVgeneric capacitances and boost inductance must be positive.");
    if (p_.L1 <= 0.0 || p_.Cf <= 0.0 || p_.L2 < 0.0 || p_.R1 < 0.0 || p_.Rc < 0.0)
        throw std::invalid_argument("PVgeneric filter parameters are invalid.");
    if (p_.L2 == 0.0 && p_.Rc <= 0.0)
        throw std::invalid_argument("PVgeneric with L2 = 0 needs a positive capacitor series resistance.");
    if (p_.Vdc_ref <= 0.0 || p_.Vm <= 0.0 || p_.omega <= 0.0)
        throw std::invalid_argument("PVgeneric Vdc, Vm and omega must be positive.");
    if (p_.Dmax <= 0.0 || p_.Dmax > 1.0)
        throw std::invalid_argument("PVgeneric Dmax must lie in (0, 1].");
    if (p_.tau_boost < 0.0)
        throw std::invalid_argument("PVgeneric boost filter time constant must be non-negative.");
    if (p_.ki_boost <= 0.0 || p_.ki_dc <= 0.0 || p_.ki_q <= 0.0 || p_.ki_i <= 0.0)
        throw std::invalid_argument("PVgeneric PI integral gains must be positive.");
    if (p_.k_ff != 0.0 && p_.k_ff != 1.0)
        throw std::invalid_argument("PVgeneric k_ff must be 0 or 1.");
}

void PVgeneric::buildLayout() {
    int i = 0;
    layout_.v_pv = i++;
    layout_.i_L = i++;
    layout_.xi_b = i++;
    layout_.p_f = (p_.tau_boost > 0.0) ? i++ : -1;
    layout_.v_dc = i++;
    layout_.xi_v = i++;
    layout_.xi_q = i++;
    layout_.xi_id = i++;
    layout_.xi_iq = i++;
    layout_.xi_pll = i++;
    layout_.theta = i++;
    layout_.i1d = i++;
    layout_.i1q = i++;
    layout_.vcd = i++;
    layout_.vcq = i++;
    if (p_.L2 > 0.0) {
        layout_.i2d = i++;
        layout_.i2q = i++;
    }
    else {
        layout_.i2d = -1;
        layout_.i2q = -1;
    }
    layout_.n = i;
}

PVgeneric::AcSteady PVgeneric::acSteady(double i2d, double i2q, double vd, double vq) const {
    // Steady LCL. alpha = ω Rc Cf. Matches the capacitor solution in PVplant.
    const double w = p_.omega;
    const double alpha = w * p_.Rc * p_.Cf;
    const double det = 1.0 + alpha * alpha;
    const double rhs_d = vd - w * p_.L2 * i2q;
    const double rhs_q = vq + w * p_.L2 * i2d;
    AcSteady ac;
    ac.vcd = (rhs_d + alpha * rhs_q) / det;
    ac.vcq = (rhs_q - alpha * rhs_d) / det;
    const double icd = -w * p_.Cf * ac.vcq;
    const double icq = w * p_.Cf * ac.vcd;
    ac.i1d = i2d + icd;
    ac.i1q = i2q + icq;
    const double vnode_d = vd - w * p_.L2 * i2q;
    const double vnode_q = vq + w * p_.L2 * i2d;
    ac.vsd = vnode_d + p_.R1 * ac.i1d - w * p_.L1 * ac.i1q;
    ac.vsq = vnode_q + p_.R1 * ac.i1q + w * p_.L1 * ac.i1d;
    ac.p_conv = 1.5 * (ac.vsd * ac.i1d + ac.vsq * ac.i1q);
    ac.q_pcc = 1.5 * (vq * i2d - vd * i2q);
    return ac;
}

PVgeneric::Evaluation PVgeneric::evaluate(const Eigen::VectorXd& x, const Eigen::VectorXd& u) const {
    Evaluation out;
    out.F = Eigen::VectorXd::Zero(layout_.n);
    const double v_pv = x(layout_.v_pv);
    const double i_L = x(layout_.i_L);
    const double v_dc_state = x(layout_.v_dc);
    const double vd = u(0);
    const double vq = u(1);
    const double i_array = array_.terminalCurrent(v_pv);

    const double p_meas = v_pv * i_L;
    const double p_ctrl = (layout_.p_f >= 0) ? x(layout_.p_f) : p_meas;
    const double boost_error = p_.Pref - p_ctrl;
    double duty = p_.kp_boost * boost_error + p_.ki_boost * x(layout_.xi_b);
    duty = std::clamp(duty, 0.0, p_.Dmax);
    out.F(layout_.v_pv) = (i_array - i_L) / p_.C_pv;
    out.F(layout_.i_L) = (v_pv - (1.0 - duty) * v_dc_state) / p_.L_boost;
    out.F(layout_.xi_b) = boost_error;
    if (layout_.p_f >= 0)
        out.F(layout_.p_f) = (p_meas - p_ctrl) / p_.tau_boost;

    double v_dc = v_dc_state;
    if (std::abs(v_dc) < 1e-3)
        v_dc = (v_dc_state >= 0.0) ? 1e-3 : -1e-3;

    const Park park(x(layout_.theta));
    double vgd_c = 0.0;
    double vgq_c = 0.0;
    park.toControl(vd, vq, vgd_c, vgq_c);
    const double pll_error = vgq_c;
    const double theta_dot = p_.kp_pll * pll_error + p_.ki_pll * x(layout_.xi_pll);
    const double omega_c = p_.omega + theta_dot;
    out.F(layout_.xi_pll) = pll_error;
    out.F(layout_.theta) = theta_dot;

    double i1d = x(layout_.i1d);
    double i1q = x(layout_.i1q);
    double vcd = x(layout_.vcd);
    double vcq = x(layout_.vcq);
    double icd = 0.0;
    double icq = 0.0;
    double i2d = 0.0;
    double i2q = 0.0;
    double vnode_d = 0.0;
    double vnode_q = 0.0;
    if (layout_.i2d >= 0) {
        i2d = x(layout_.i2d);
        i2q = x(layout_.i2q);
        icd = i1d - i2d;
        icq = i1q - i2q;
        vnode_d = vcd + p_.Rc * icd;
        vnode_q = vcq + p_.Rc * icq;
    }
    else {
        vnode_d = vd;
        vnode_q = vq;
        icd = (vd - vcd) / p_.Rc;
        icq = (vq - vcq) / p_.Rc;
        i2d = i1d - icd;
        i2q = i1q - icq;
    }

    const double q_pcc = 1.5 * (vq * i2d - vd * i2q);
    const double vdc_error = p_.Vdc_ref - v_dc_state;
    const double q_error = p_.Qref - q_pcc;
    const double id_ref = -(p_.kp_dc * vdc_error + p_.ki_dc * x(layout_.xi_v));
    const double iq_ref = -(p_.kp_q * q_error + p_.ki_q * x(layout_.xi_q));
    out.F(layout_.xi_v) = vdc_error;
    out.F(layout_.xi_q) = q_error;

    double id_c = 0.0;
    double iq_c = 0.0;
    park.toControl(i1d, i1q, id_c, iq_c);
    const double id_error = id_ref - id_c;
    const double iq_error = iq_ref - iq_c;
    const double vd_c = p_.kp_i * id_error + p_.ki_i * x(layout_.xi_id)
        - omega_c * p_.L1 * iq_c + p_.k_ff * vgd_c;
    const double vq_c = p_.kp_i * iq_error + p_.ki_i * x(layout_.xi_iq)
        + omega_c * p_.L1 * id_c + p_.k_ff * vgq_c;
    out.F(layout_.xi_id) = id_error;
    out.F(layout_.xi_iq) = iq_error;

    double vsd = 0.0;
    double vsq = 0.0;
    park.toGrid(vd_c, vq_c, vsd, vsq);
    const double w = p_.omega;
    out.F(layout_.i1d) = (vsd - vnode_d - p_.R1 * i1d + w * p_.L1 * i1q) / p_.L1;
    out.F(layout_.i1q) = (vsq - vnode_q - p_.R1 * i1q - w * p_.L1 * i1d) / p_.L1;
    out.F(layout_.vcd) = (icd + w * p_.Cf * vcq) / p_.Cf;
    out.F(layout_.vcq) = (icq - w * p_.Cf * vcd) / p_.Cf;
    if (layout_.i2d >= 0) {
        out.F(layout_.i2d) = (vnode_d - vd + w * p_.L2 * i2q) / p_.L2;
        out.F(layout_.i2q) = (vnode_q - vq - w * p_.L2 * i2d) / p_.L2;
    }

    const double p_conv = 1.5 * (vsd * i1d + vsq * i1q);
    out.F(layout_.v_dc) = ((1.0 - duty) * i_L - p_conv / v_dc) / p_.C_dc;
    out.y << i2d, i2q;
    return out;
}

Eigen::MatrixXd PVgeneric::computeStateDerivatives(const Eigen::VectorXd& x,
    const Eigen::VectorXd& u) const {
    return evaluate(x, u).F;
}

void PVgeneric::solveEquilibrium() {
    const double vd = p_.Vm * std::cos(p_.theta);
    const double vq = p_.Vm * std::sin(p_.theta);
    const double v_pv = array_.voltageForPower(p_.Pref);
    const double i_array = array_.terminalCurrent(v_pv);
    const double duty = 1.0 - v_pv / p_.Vdc_ref;
    if (duty <= 0.0 || duty >= p_.Dmax) {
        std::ostringstream message;
        message << "[PVgeneric::solveEquilibrium] Duty " << duty
            << " is outside (0, Dmax). Raise Vdc_ref or reduce Pref.";
        throw std::runtime_error(message.str());
    }
    if (std::abs(v_pv * i_array - p_.Pref) > 1e-6 * std::max(1.0, std::abs(p_.Pref)))
        throw std::runtime_error("[PVgeneric::solveEquilibrium] Array power does not match Pref.");

    const double denom = vd * vd + vq * vq;
    Eigen::Vector2d grid_current;
    grid_current << (2.0 / 3.0) * (vd * p_.Pref + vq * p_.Qref) / denom,
        (2.0 / 3.0) * (vq * p_.Pref - vd * p_.Qref) / denom;

    Eigen::Vector2d residual = Eigen::Vector2d::Ones();
    for (int k = 0; k < 25 && residual.norm() > 1e-8 * std::max(1.0, std::abs(p_.Pref)); ++k) {
        const AcSteady ac = acSteady(grid_current(0), grid_current(1), vd, vq);
        residual(0) = ac.p_conv - p_.Pref;
        residual(1) = ac.q_pcc - p_.Qref;
        Eigen::Matrix2d jacobian;
        const double step = 1e-6 * std::max(1.0, grid_current.norm());
        for (int col = 0; col < 2; ++col) {
            Eigen::Vector2d perturbed = grid_current;
            perturbed(col) += step;
            const AcSteady ac_p = acSteady(perturbed(0), perturbed(1), vd, vq);
            jacobian(0, col) = (ac_p.p_conv - ac.p_conv) / step;
            jacobian(1, col) = (ac_p.q_pcc - ac.q_pcc) / step;
        }
        grid_current -= jacobian.colPivHouseholderQr().solve(residual);
    }
    const AcSteady ac = acSteady(grid_current(0), grid_current(1), vd, vq);
    residual(0) = ac.p_conv - p_.Pref;
    residual(1) = ac.q_pcc - p_.Qref;
    if (residual.norm() > 1e-6 * std::max(1.0, std::abs(p_.Pref))) {
        std::ostringstream message;
        message << "[PVgeneric::solveEquilibrium] AC power balance residual " << residual.norm();
        throw std::runtime_error(message.str());
    }

    const Park park(p_.theta);
    double id_c = 0.0;
    double iq_c = 0.0;
    double vd_c = 0.0;
    double vq_c = 0.0;
    park.toControl(ac.i1d, ac.i1q, id_c, iq_c);
    park.toControl(ac.vsd, ac.vsq, vd_c, vq_c);
    const double vgd_c = p_.Vm;
    const double vgq_c = 0.0;
    const double ff_d = -p_.omega * p_.L1 * iq_c + p_.k_ff * vgd_c;
    const double ff_q = p_.omega * p_.L1 * id_c + p_.k_ff * vgq_c;

    Eigen::VectorXd x = Eigen::VectorXd::Zero(layout_.n);
    x(layout_.v_pv) = v_pv;
    x(layout_.i_L) = i_array;
    x(layout_.xi_b) = duty / p_.ki_boost;
    if (layout_.p_f >= 0)
        x(layout_.p_f) = p_.Pref;
    x(layout_.v_dc) = p_.Vdc_ref;
    x(layout_.xi_v) = -id_c / p_.ki_dc;
    x(layout_.xi_q) = -iq_c / p_.ki_q;
    x(layout_.xi_id) = (vd_c - ff_d) / p_.ki_i;
    x(layout_.xi_iq) = (vq_c - ff_q) / p_.ki_i;
    x(layout_.xi_pll) = 0.0;
    x(layout_.theta) = p_.theta;
    x(layout_.i1d) = ac.i1d;
    x(layout_.i1q) = ac.i1q;
    x(layout_.vcd) = ac.vcd;
    x(layout_.vcq) = ac.vcq;
    if (layout_.i2d >= 0) {
        x(layout_.i2d) = grid_current(0);
        x(layout_.i2q) = grid_current(1);
    }

    operating_input_ = Eigen::Vector2d(vd, vq);
    const Evaluation eq = evaluate(x, operating_input_);
    const double ode_residual = eq.F.lpNorm<Eigen::Infinity>();
    if (ode_residual > 1e-4) {
        std::ostringstream message;
        message << "[PVgeneric::solveEquilibrium] ODE residual " << ode_residual;
        throw std::runtime_error(message.str());
    }
    equilibrium_ = x;
    point_.v_pv = v_pv;
    point_.i_array = i_array;
    point_.duty = duty;
    point_.g_pv = array_.incrementalConductance(v_pv);
    point_.p_conv = ac.p_conv;
    point_.q_pcc = ac.q_pcc;
    point_.i2d = grid_current(0);
    point_.i2q = grid_current(1);
}

void PVgeneric::computeABCD() {
    if (equilibrium_.size() == 0 || operating_input_.size() != 2)
        throw std::runtime_error("[PVgeneric::computeABCD] No equilibrium state.");
    const Eigen::VectorXd& x0 = equilibrium_;
    const Eigen::VectorXd& u0 = operating_input_;
    RHSFunc rhs = [this](double, const Eigen::VectorXd& x, const Eigen::VectorXd& u) {
        return computeStateDerivatives(x, u);
    };
    auto [A, B] = computeJacobians(rhs, x0, u0);
    A_ = A;
    B_ = B;

    const int n = static_cast<int>(x0.size());
    const double eps = 1e-8;
    C_ = Eigen::MatrixXd::Zero(2, n);
    for (int k = 0; k < n; ++k) {
        const double h = eps * std::max(1.0, std::abs(x0(k)));
        Eigen::VectorXd xp = x0;
        Eigen::VectorXd xm = x0;
        xp(k) += h;
        xm(k) -= h;
        C_.col(k) = (evaluate(xp, u0).y - evaluate(xm, u0).y) / (2.0 * h);
    }
    D_ = Eigen::MatrixXd::Zero(2, 2);
    for (int k = 0; k < 2; ++k) {
        const double h = eps * std::max(1.0, std::abs(u0(k)));
        Eigen::VectorXd up = u0;
        Eigen::VectorXd um = u0;
        up(k) += h;
        um(k) -= h;
        D_.col(k) = (evaluate(x0, up).y - evaluate(x0, um).y) / (2.0 * h);
    }
}

std::vector<std::vector<std::complex<double>>> PVgeneric::compute_y_parameters(double frequency) {
    const int n = static_cast<int>(A_.rows());
    if (n == 0)
        throw std::runtime_error("[PVgeneric::compute_y_parameters] Call computeABCD() first.");
    std::vector<std::vector<std::complex<double>>> Y(2, std::vector<std::complex<double>>(2, { 0.0, 0.0 }));
    const std::complex<double> s(0.0, 2.0 * kPi * frequency);
    const Eigen::MatrixXcd resolvent =
        (s * Eigen::MatrixXcd::Identity(n, n) - A_.cast<std::complex<double>>()).inverse();
    const Eigen::MatrixXcd Ydq = C_.cast<std::complex<double>>() * resolvent * B_.cast<std::complex<double>>()
        + D_.cast<std::complex<double>>();
    for (int r = 0; r < 2; ++r) {
        for (int c = 0; c < 2; ++c)
            Y[r][c] = Ydq(r, c);
    }
    return Y;
}

void PVgeneric::printElementValues() {
    Element::printElementInfo();
    std::cout << "PVgeneric Pref=" << p_.Pref << " W, Qref=" << p_.Qref
        << " var, Vdc=" << p_.Vdc_ref << " V, states=" << layout_.n << "\n";
    if (equilibrium_.size() != 0) {
        std::cout << "  v_pv=" << point_.v_pv << " V, I_pv=" << point_.i_array
            << " A, D=" << point_.duty << ", g_pv=" << point_.g_pv << " S\n";
    }
}
