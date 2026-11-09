/**
 * @file TwoLC.cpp
 * @brief Two-level converter. Control walk follows MMC; the plant is a series reactor.
 *
 * State order: PLL (ξ, θ), outer P or Vdc, outer Q or Vac, OCC (ξd, ξq),
 * Padé delay on (m_d, m_q), then grid-frame (i_d, i_q).
 * di_d/dt = (v_Md - v_d - R i_d + ω L i_q) / L
 * di_q/dt = (v_Mq - v_q - R i_q - ω L i_d) / L
 * i_dc = 1.5 (v_Md i_d + v_Mq i_q) / V_dc
 */
#include "TwoLC.h"

#include "Solver/Helper_Functions/Differential_equations.h"

#include <cmath>
#include <stdexcept>

namespace {
constexpr int kPlant = 2;
}

TwoLC::TwoLC(const std::string& symbol, const std::string& location,
    const std::vector<double>& converter_params)
    : Converter(symbol, location)
{
    if (converter_params.size() < 10) {
        throw std::invalid_argument("2LC converter_params needs 10 values: omega, P, Q, theta, Vm, Pdc, Vdc, L, R, delay.");
    }
    omega_0 = converter_params[0];
    P = converter_params[1];
    Q = converter_params[2];
    theta = converter_params[3];
    V_m = converter_params[4];
    P_dc = converter_params[5];
    V_dc = converter_params[6];
    L_reactor = converter_params[7];
    R_reactor = converter_params[8];
    t_delay = converter_params[9];
    if (converter_params.size() > 10) {
        pade_order = static_cast<int>(std::lround(converter_params[10]));
    }
    if (L_reactor <= 0.0) {
        throw std::invalid_argument("2LC reactor inductance must be positive.");
    }
    P_min = -std::abs(P) - 1.0;
    P_max = std::abs(P) + 1.0;
    Q_min = -std::abs(P) - std::abs(Q) - 1.0;
    Q_max = std::abs(P) + std::abs(Q) + 1.0;

    const int channels = 2;
    if (t_delay != 0.0) {
        number_of_states += channels * pade_order;
        Adelay = Eigen::MatrixXd::Zero(channels * pade_order, channels * pade_order);
        Bdelay = Eigen::MatrixXd::Zero(channels * pade_order, channels);
        Cdelay = Eigen::MatrixXd::Zero(channels, channels * pade_order);
        Ddelay = Eigen::MatrixXd::Zero(channels, channels);
        if (pade_order == 2) {
            padeDelaySystemMulti2(t_delay, Adelay, Bdelay, Cdelay, Ddelay, channels);
        }
        else if (pade_order == 3) {
            padeDelaySystemMulti3(t_delay, Adelay, Bdelay, Cdelay, Ddelay, channels);
        }
        else {
            throw std::invalid_argument("2LC Padé delay supports orders 2 and 3.");
        }
    }
    Y_matrix.resize(3, 3);
}

TwoLC::TwoLC(const std::string& symbol, const std::string& location,
    const std::vector<double>& converter_params, const std::vector<double>& controller_params)
    : TwoLC(symbol, location, converter_params)
{
    initController(controller_params);
}

void TwoLC::initController(const std::vector<double>& controller_params) {
    const int ndelay = (t_delay != 0.0) ? 2 * pade_order : 0;
    for (int i = 0; i < static_cast<int>(controller_params.size());) {
        for (const auto& controller_name : controller_list) {
            if (i >= static_cast<int>(controller_params.size())) {
                break;
            }
            if (static_cast<bool>(controller_params[i])) {
                if ((i + 3) >= static_cast<int>(controller_params.size())) {
                    throw std::invalid_argument("Insufficient parameters for 2LC controller initialization.");
                }
                const int controller_type = static_cast<int>(controller_params[++i]);
                int number_of_values = 0;
                if (controller_type == 0) {
                    if ((i + 3) >= static_cast<int>(controller_params.size())) {
                        throw std::invalid_argument("Insufficient parameters for a 2LC PI controller.");
                    }
                    const std::vector<double> values = { controller_params[++i], controller_params[++i] };
                    number_of_values = static_cast<int>(controller_params[++i]);
                    std::vector<double> refs(number_of_values, 0.0);
                    if ((i + number_of_values) < static_cast<int>(controller_params.size())) {
                        refs.assign(controller_params.begin() + i + 1, controller_params.begin() + i + 1 + number_of_values);
                        i++;
                    }
                    controls[controller_name] = std::make_unique<ProportionalIntegralController>(
                        controller_name, values, number_of_values, refs);
                }
                else {
                    throw std::invalid_argument("2LC controllers are PI (type 0).");
                }
                if (controller_name == "energy" || controller_name == "zcc" || controller_name == "ccc"
                    || controller_name == "droop" || controller_name == "gfm") {
                    throw std::invalid_argument("2LC has no arm energy, circulating current, droop, or GFM loop.");
                }
                if (controller_name == "dc_voltage") {
                    vdc_index_ = number_of_states - kPlant - ndelay;
                }
                if (controller_name == "pll") {
                    number_of_states += 2;
                }
                else {
                    number_of_states += number_of_values;
                }
                i += number_of_values;
            }
            else {
                i += 1;
            }
        }
    }
    if (controls.count("dc_voltage") && controls.count("active_power")) {
        throw std::invalid_argument("2LC cannot enable DC voltage and active power together.");
    }
    if (controls.count("ac_voltage") && controls.count("reactive_power")) {
        throw std::invalid_argument("2LC cannot enable AC voltage and reactive power together.");
    }
    if (!controls.count("occ")) {
        throw std::invalid_argument("2LC requires the output-current loop.");
    }
}

Eigen::Vector3d TwoLC::operatingInput() const {
    return Eigen::Vector3d(V_dc, V_m * std::cos(theta), -V_m * std::sin(theta));
}

TwoLC::Evaluation TwoLC::evaluate(const Eigen::VectorXd& x, const Eigen::VectorXd& u) {
    Evaluation out;
    out.F = Eigen::VectorXd::Zero(number_of_states);
    const double L = L_reactor;
    const double R = R_reactor;
    const int ndelay = (t_delay != 0.0) ? 2 * pade_order : 0;
    const int plant = number_of_states - kPlant;
    const double id = x(plant);
    const double iq = x(plant + 1);
    double Vdc = u(0);
    const double Vgd_g = u(1);
    const double Vgq_g = u(2);
    double Vgd = Vgd_g;
    double Vgq = Vgq_g;
    double id_c = id;
    double iq_c = iq;
    const double Pac = 1.5 * (Vgd_g * id + Vgq_g * iq);
    const double Qac = 1.5 * (Vgd_g * iq - Vgq_g * id);

    Eigen::Matrix2d T = Eigen::Matrix2d::Identity();
    Eigen::Matrix2d Ith = Eigen::Matrix2d::Identity();
    double omega_c = omega_0;
    int i = 0;

    if (controls.count("pll")) {
        const double theta_c = x(i + 1);
        const double cth = std::cos(theta_c);
        const double sth = std::sin(theta_c);
        T << cth, -sth, sth, cth;
        Ith << cth, sth, -sth, cth;
        const Eigen::Vector2d Vg = T * Eigen::Vector2d(Vgd_g, Vgq_g);
        Vgd = Vg(0);
        Vgq = Vg(1);
        const Eigen::Vector2d ic = T * Eigen::Vector2d(id, iq);
        id_c = ic(0);
        iq_c = ic(1);
        const Eigen::VectorXd pll = controls["pll"]->define_equations(x(i), Vgq, 0.0);
        out.F(i) = pll(0);
        out.F(i + 1) = pll(1);
        omega_c = omega_0 + pll(1);
        i += 2;
    }

    double id_ref = id_c;
    double iq_ref = iq_c;
    if (controls.count("active_power")) {
        const Eigen::VectorXd pctrl = controls["active_power"]->define_equations(x(i), Pac, 0.0);
        out.F(i) = pctrl(0);
        id_ref = pctrl(1);
        i += 1;
    }
    else if (controls.count("dc_voltage")) {
        const auto gains = controls["dc_voltage"]->getParameters();
        const auto refs = controls["dc_voltage"]->getReference();
        const double Vref = refs.empty() ? V_dc : refs[0];
        const double error = Vref - Vdc;
        out.F(i) = gains[1] * error;
        id_ref = -(gains[0] * error + x(i));
        i += 1;
    }
    if (controls.count("reactive_power")) {
        const Eigen::VectorXd qctrl = controls["reactive_power"]->define_equations(x(i), Qac, 0.0);
        out.F(i) = qctrl(0);
        iq_ref = qctrl(1);
        i += 1;
    }
    else if (controls.count("ac_voltage")) {
        const Eigen::VectorXd vctrl = controls["ac_voltage"]->define_equations(x(i), Vgd, 0.0);
        out.F(i) = vctrl(0);
        iq_ref = vctrl(1);
        i += 1;
    }

    controls["occ"]->setReference({ id_ref, iq_ref });
    const Eigen::Vector2d feed(
        omega_c * L * iq_c + Vgd,
        -omega_c * L * id_c + Vgq);
    const Eigen::VectorXd occ = controls["occ"]->define_equations(
        x.segment(i, 2), Eigen::Vector2d(id_c, iq_c), feed);
    out.F(i) = occ(0);
    out.F(i + 1) = occ(1);
    Eigen::Vector2d vM = Ith * Eigen::Vector2d(occ(2), occ(3));
    i += 2;

    if (std::abs(Vdc) < 1e-6) {
        Vdc = (std::abs(V_dc) > 1e-6) ? V_dc : 1.0;
    }
    Eigen::Vector2d m = 2.0 * vM / Vdc;
    if (ndelay) {
        const Eigen::VectorXd xdelay = x.segment(i, ndelay);
        out.F.segment(i, ndelay) = Adelay * xdelay + Bdelay * m;
        m = Cdelay * xdelay + Ddelay * m;
        i += ndelay;
        // PSCAD frame: delayed modulation is rotated by ω0*τ.
        const double angle = omega_0 * t_delay;
        const double cth = std::cos(angle);
        const double sth = std::sin(angle);
        Eigen::Matrix2d compensate;
        compensate << cth, -sth, sth, cth;
        m = compensate * m;
        vM = m * Vdc / 2.0;
    }
    last_vMd_ = vM(0);
    last_vMq_ = vM(1);

    out.F(plant) = (vM(0) - Vgd_g - R * id - omega_0 * L * iq) / L;
    out.F(plant + 1) = (vM(1) - Vgq_g - R * iq + omega_0 * L * id) / L;
    const double idc = 1.5 * (vM(0) * id + vM(1) * iq) / Vdc;
    out.y << idc, id, iq;
    return out;
}

Eigen::MatrixXd TwoLC::computeStateDerivatives(const Eigen::VectorXd& x, const Eigen::VectorXd& u) {
    return evaluate(x, u).F;
}

void TwoLC::solveEquilibrium() {
    const Eigen::Vector3d u = operatingInput();
    const double Vgd = u(1);
    const double Vgq = u(2);
    const double denom = Vgd * Vgd + Vgq * Vgq;
    if (denom < 1e-6) {
        throw std::runtime_error("2LC voltage magnitude is too small.");
    }
    const double Id = (2.0 / 3.0) * (Vgd * P - Vgq * Q) / denom;
    const double Iq = (2.0 / 3.0) * (Vgq * P + Vgd * Q) / denom;
    const double vMd = Vgd + R_reactor * Id + omega_0 * L_reactor * Iq;
    const double vMq = Vgq + R_reactor * Iq - omega_0 * L_reactor * Id;

    Eigen::VectorXd x = Eigen::VectorXd::Zero(number_of_states);
    int i = 0;
    if (controls.count("pll")) {
        x(i + 1) = theta;
        i += 2;
    }
    if (controls.count("active_power")) {
        const double Ki = controls["active_power"]->getParameters()[1];
        x(i) = (std::abs(Ki) > 1e-18) ? Id / Ki : 0.0;
        i += 1;
    }
    else if (controls.count("dc_voltage")) {
        x(i) = -Id;
        i += 1;
    }
    if (controls.count("reactive_power")) {
        const double Ki = controls["reactive_power"]->getParameters()[1];
        x(i) = (std::abs(Ki) > 1e-18) ? Iq / Ki : 0.0;
        i += 1;
    }
    else if (controls.count("ac_voltage")) {
        const double Ki = controls["ac_voltage"]->getParameters()[1];
        x(i) = (std::abs(Ki) > 1e-18) ? Iq / Ki : 0.0;
        i += 1;
    }
    const double Ki_occ = controls["occ"]->getParameters()[1];
    const double angle = (t_delay != 0.0) ? omega_0 * t_delay : 0.0;
    const double cth = std::cos(angle);
    const double sth = std::sin(angle);
    Eigen::Matrix2d compensate;
    compensate << cth, -sth, sth, cth;
    const Eigen::Vector2d v_plant(vMd, vMq);
    const Eigen::Vector2d v_cmd = compensate.transpose() * v_plant;
    if (std::abs(Ki_occ) > 1e-18) {
        const Eigen::Vector2d v_ff(
            Vgd + omega_0 * L_reactor * Iq,
            Vgq - omega_0 * L_reactor * Id);
        const Eigen::Vector2d v_pi = v_cmd - v_ff;
        x(i) = v_pi(0) / Ki_occ;
        x(i + 1) = v_pi(1) / Ki_occ;
    }
    i += 2;
    const int ndelay = (t_delay != 0.0) ? 2 * pade_order : 0;
    if (ndelay) {
        const Eigen::Vector2d m = 2.0 * v_cmd / u(0);
        x.segment(i, ndelay) = -Adelay.colPivHouseholderQr().solve(Bdelay * m);
    }
    const int plant = number_of_states - kPlant;
    x(plant) = Id;
    x(plant + 1) = Iq;

    const Evaluation eq = evaluate(x, u);
    if (eq.F.norm() > 1e-4) {
        throw std::runtime_error("[2LC::solveEquilibrium] residual " + std::to_string(eq.F.norm()));
    }
    equilibrium_state = x;
    operating_input_ = u;
}

void TwoLC::computeABCD() {
    if (equilibrium_state.size() == 0) {
        throw std::runtime_error("[2LC::computeABCD] No equilibrium state.");
    }
    const Eigen::VectorXd& x0 = equilibrium_state;
    const Eigen::Vector3d u0 = (operating_input_.size() >= 3)
        ? Eigen::Vector3d(operating_input_.head(3))
        : operatingInput();
    RHSFunc rhs = [this](double, const Eigen::VectorXd& x, const Eigen::VectorXd& u) {
        return computeStateDerivatives(x, u);
    };
    auto [A, B] = computeJacobians(rhs, x0, u0);
    A_matrix = A;
    B_matrix = B;

    const int n = static_cast<int>(x0.size());
    const double eps = 1e-5;
    C_matrix = Eigen::MatrixXd::Zero(3, n);
    for (int k = 0; k < n; ++k) {
        Eigen::VectorXd xp = x0;
        xp(k) += eps;
        const Eigen::Vector3d dy = evaluate(xp, u0).y - evaluate(x0, u0).y;
        C_matrix.col(k) = dy / eps;
    }
    D_matrix = Eigen::MatrixXd::Zero(3, 3);
    for (int k = 0; k < 3; ++k) {
        Eigen::Vector3d up = u0;
        up(k) += eps;
        const Eigen::Vector3d dy = evaluate(x0, up).y - evaluate(x0, u0).y;
        D_matrix.col(k) = dy / eps;
    }
}

std::vector<std::vector<std::complex<double>>> TwoLC::compute_y_parameters(double frequency) {
    const int n = static_cast<int>(A_matrix.rows());
    std::vector<std::vector<std::complex<double>>> Y_val(3, std::vector<std::complex<double>>(3, { 0.0, 0.0 }));
    if (n == 0) {
        return Y_val;
    }
    const std::complex<double> s(0.0, 2.0 * M_PI * frequency);
    const Eigen::MatrixXcd resolvent = (s * Eigen::MatrixXcd::Identity(n, n) - A_matrix.cast<std::complex<double>>()).inverse();
    const Eigen::MatrixXcd Y = C_matrix.cast<std::complex<double>>() * resolvent * B_matrix.cast<std::complex<double>>()
        + D_matrix.cast<std::complex<double>>();
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            Y_val[r][c] = Y(r, c);
        }
    }
    return Y_val;
}

void TwoLC::printElementValues() {
    Element::printElementInfo();
    std::cout << "2LC reactor L=" << L_reactor << " H, R=" << R_reactor
        << " ohm, delay=" << t_delay << " s, Padé " << pade_order << "\n";
}
