#ifndef PV_GENERIC_H_
#define PV_GENERIC_H_

/**
 * @file PV_generic.h
 * @brief PSCAD generic PV array and its ODE plant model.
 *
 * PVplant (the existing class) is left unchanged. It builds a symbolic dq
 * admittance from an ideal-diode slope
 *   k_pv = dI/dV = -q (Np I0 + Np Isc - I) / (Ns n k T)
 * at T = 298.15 K with Rsr = 0 and Rsh infinite, a boost PI on PV voltage
 * with an MPP linearisation, and no reactive-power loop.
 *
 * PVarray is the PSCAD single-diode array (Photovoltaic Example / Master
 * Library form). PVgeneric integrates that array with the generic-example
 * power stage: input capacitor, averaged boost in power mode, DC link, and
 * an LCL inverter. The state equations are integrated and linearised the
 * same way as MMC / TwoLC (equilibrium, ABCD, Y(s)).
 *
 * MATLAB frame, separate from the MMC: vd = Vm cos θ, vq = +Vm sin θ,
 *   P = 3/2 (vd id + vq iq),  Q = 3/2 (vq id - vd iq).
 * PSCAD's q axis is the opposite one, so a PSCAD (d, q) pair maps by
 * diag(1, -1). Port current is the current leaving the plant into the grid.
 * Ydq is 2×2 from (vd, vq) to (id, iq) at the PCC.
 *
 * PSCAD PI blocks are Kp (1 + 1/(s Ti)). Here Ki = Kp/Ti and each integrator
 * stores the error, matching ProportionalIntegralController:
 *   xi_dot = error,  y = Kp error + Ki xi.
 */

#include "RES_base.h"

#include <complex>
#include <vector>

/** Electron charge (C), same value as PVplant. */
inline constexpr double kPvElectronCharge = 1.602176634e-19;
/** Boltzmann constant (J/K), same value as PVplant. */
inline constexpr double kPvBoltzmann = 1.380649e-23;

/**
 * @brief PSCAD single-diode PV array parameters.
 *
 * Rsh <= 0 means an open shunt. Eg is the bandgap voltage in volts
 * (1.12 for silicon): the exponent is q Eg /(n k) (1/Tc_ref - 1/Tc).
 */
struct PVarrayParameters {
    double Ns = 1.0;
    double Np = 1.0;
    double n = 1.5;
    double Isc_ref = 8.0;
    double Io_ref = 1e-10;
    double alpha_T = 0.0;
    double Eg = 1.12;
    double Tc = 298.15;
    double Tc_ref = 298.15;
    double G = 1000.0;
    double G_ref = 1000.0;
    double Rsr = 0.0;
    double Rsh = 0.0;
};

/**
 * @brief Single-diode array in the PSCAD form.
 *
 * I = Np [ Ig - Io (exp(q (V/Ns + I Rsr/Np) / (n k Tc)) - 1)
 *         - (V/Ns + I Rsr/Np) / Rsh ]
 * Ig = (G/Gref) [ Isc_ref + alpha_T (Tc - Tc_ref) ]
 * Io = Io_ref (Tc/Tc_ref)^3 exp[ q Eg /(n k) (1/Tc_ref - 1/Tc) ]
 *
 * Incremental conductance (positive):
 *   g = -dI/dV = (Np/Ns) (Gd + 1/Rsh) / (1 + Rsr (Gd + 1/Rsh))
 *   Gd = q Io /(n k Tc) exp(q VD /(n k Tc)),  VD = V/Ns + I Rsr/Np.
 * With Rsr = 0, Rsh open, G = Gref and Tc = Tc_ref this g equals -k_pv
 * of PVplant.
 */
class PVarray {
public:
    explicit PVarray(const PVarrayParameters& parameters);

    const PVarrayParameters& parameters() const { return p_; }

    double photoCurrent() const;
    double saturationCurrent() const;
    double terminalCurrent(double voltage) const;
    /** Positive incremental conductance -dI/dV at @p voltage. */
    double incrementalConductance(double voltage) const;
    /** Diode slope dI/dV. Equals PVplant's k_pv in the ideal-diode case. */
    double dCurrentDVoltage(double voltage) const {
        return -incrementalConductance(voltage);
    }
    double openCircuitVoltage() const;
    /**
     * @brief High-voltage root of v I(v) = @p power (right of the MPP).
     * The power-mode boost is stable on this side of the curve.
     */
    double voltageForPower(double power) const;

private:
    PVarrayParameters p_;
};

/**
 * @brief Plant parameters for the PSCAD generic PV example.
 *
 * AC filter is the LCL already used by PVplant: converter R1-L1, shunt Cf
 * with series Rc, then grid L2. L2 = 0 ties the capacitor node to the PCC
 * (Rc must be positive). tau_boost = 0 skips the boost power filter.
 * k_ff is the PCC voltage feedforward switch in {0, 1}.
 */
struct PVgenericParameters {
    PVarrayParameters pv;
    double C_pv = 1e-3;
    double L_boost = 1e-3;
    double C_dc = 5e-3;
    double Vdc_ref = 800.0;
    double Pref = 1e4;
    double Qref = 0.0;
    double Dmax = 0.95;
    double kp_boost = 1e-5;
    double ki_boost = 1e-3;
    double tau_boost = 0.0;
    double L1 = 200e-6;
    double R1 = 0.01;
    double Cf = 50e-6;
    double Rc = 0.2;
    double L2 = 0.0;
    double Vm = 187.794;
    double theta = 0.0;
    double omega = 2.0 * 3.14159265358979323846 * 50.0;
    double kp_dc = 0.5;
    double ki_dc = 40.0;
    double kp_q = 1e-4;
    double ki_q = 1e-2;
    double kp_i = 0.5;
    double ki_i = 80.0;
    double kp_pll = 10.0;
    double ki_pll = 200.0;
    double k_ff = 0.0;
};

/**
 * @class PVgeneric
 * @brief ODE model of the PSCAD generic PV plant.
 * @ingroup res
 *
 * State order: v_pv, i_L, boost integrator, optional power-filter state,
 * v_dc, Vdc integrator, Q integrator, current integrators (d, q),
 * PLL integrator, PLL angle, i1_dq, vcf_dq, and i2_dq when L2 > 0.
 *
 *   Cpv dv_pv/dt = I(v_pv) - i_L
 *   Lb di_L/dt   = v_pv - (1 - D) v_dc
 *   D            = sat( PI(Pref - Ppv), 0, Dmax ),  Ppv = v_pv i_L
 *   Cdc dv_dc/dt = (1 - D) i_L - Pconv / v_dc
 *   Pconv        = 3/2 (vsd i1d + vsq i1q)
 *   id*          = -PI(Vdc* - v_dc),   iq* = -PI(Q* - Qpcc)
 *   vs           = PI(i* - i1) + ωc L1 decoupling + k_ff vpcc
 */
class PVgeneric : public RES_base {
public:
    PVgeneric(const std::string& symbol, const std::string& location,
        const PVgenericParameters& parameters);

    ~PVgeneric() override = default;

    const PVgenericParameters& parameters() const { return p_; }
    const PVarray& pvArray() const { return array_; }
    int numberOfStates() const { return layout_.n; }

    void solveEquilibrium();
    Eigen::MatrixXd computeStateDerivatives(const Eigen::VectorXd& x,
        const Eigen::VectorXd& u) const;
    void computeABCD();
    std::vector<std::vector<std::complex<double>>> compute_y_parameters(double frequency) override;

    const Eigen::MatrixXd& getA() const { return A_; }
    const Eigen::MatrixXd& getB() const { return B_; }
    const Eigen::MatrixXd& getC() const { return C_; }
    const Eigen::MatrixXd& getD() const { return D_; }
    const Eigen::VectorXd& getEquilibriumState() const { return equilibrium_; }
    const Eigen::VectorXd& operatingInput() const { return operating_input_; }

    /** Index of the PV voltage state, for Jacobian checks. */
    int pvVoltageIndex() const { return layout_.v_pv; }
    int boostCurrentIndex() const { return layout_.i_L; }

    struct OperatingPoint {
        double v_pv = 0.0;
        double i_array = 0.0;
        double duty = 0.0;
        double g_pv = 0.0;
        double p_conv = 0.0;
        double q_pcc = 0.0;
        double i2d = 0.0;
        double i2q = 0.0;
    };
    const OperatingPoint& operatingPoint() const { return point_; }

    void printElementValues() override;

private:
    struct Layout {
        int v_pv = 0;
        int i_L = 1;
        int xi_b = 2;
        int p_f = -1;
        int v_dc = 3;
        int xi_v = 4;
        int xi_q = 5;
        int xi_id = 6;
        int xi_iq = 7;
        int xi_pll = 8;
        int theta = 9;
        int i1d = 10;
        int i1q = 11;
        int vcd = 12;
        int vcq = 13;
        int i2d = -1;
        int i2q = -1;
        int n = 14;
    };

    struct Evaluation {
        Eigen::VectorXd F;
        Eigen::Vector2d y;
    };

    struct AcSteady {
        double vcd = 0.0;
        double vcq = 0.0;
        double i1d = 0.0;
        double i1q = 0.0;
        double vsd = 0.0;
        double vsq = 0.0;
        double p_conv = 0.0;
        double q_pcc = 0.0;
    };

    void buildLayout();
    void validate() const;
    Evaluation evaluate(const Eigen::VectorXd& x, const Eigen::VectorXd& u) const;
    AcSteady acSteady(double i2d, double i2q, double vd, double vq) const;

    PVgenericParameters p_;
    PVarray array_;
    Layout layout_;
    Eigen::MatrixXd A_;
    Eigen::MatrixXd B_;
    Eigen::MatrixXd C_;
    Eigen::MatrixXd D_;
    Eigen::VectorXd equilibrium_;
    Eigen::VectorXd operating_input_;
    OperatingPoint point_;
};

#endif
