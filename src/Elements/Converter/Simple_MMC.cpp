#include "Simple_MMC.h"

/**
 * @brief MMC constructor with explicit parameters.
 * @param symbol Element symbol/name.
 * @param omega Nominal angular frequency (rad/s).
 * @param activePower Active power (W).
 * @param reactivePower Reactive power (VAR).
 * @param angle Initial phase angle (rad).
 * @param acVoltage AC voltage amplitude (V).
 * @param dcVoltage DC voltage (V).
 * @param armInductance Arm inductance (H).
 * @param armResistance Arm resistance (Ohm).
 * @param armCapacitance Arm capacitance (F).
 * @param numSubmodules Number of submodules.
 * @param reactorInductance Reactor inductance (H).
 * @param reactorResistance Reactor resistance (Ohm).
 */

 // First constructor: explicit parameter version
Simple_MMC::Simple_MMC(const std::string& symbol, const std::string& location,
    double omega, double activePower, double reactivePower,
    double angle, double acVoltage, double Pdc, double dcVoltage,
    double armInductance, double armResistance, double armCapacitance,
    int numSubmodules, double reactorInductance, double reactorResistance)
    : Converter(symbol, location)
{
    omega_0 = omega;
    P = activePower;
    Q = reactivePower;
    theta = angle;
    V_m = acVoltage;
    V_dc = dcVoltage;
    P_dc = Pdc;

    L_arm = armInductance;
    R_arm = armResistance;
    C_arm = armCapacitance;
    N = numSubmodules;
    L_reactor = reactorInductance;
    R_reactor = reactorResistance;

    P_min = 0.5 * P;
    P_max = 1.5 * P;
    Q_min = -P;
    Q_max = P;

    equilibrium_state = Eigen::VectorXd::Zero(number_of_states);
    Y_matrix.resize(3, 3);

    computeABCD();
}

// Constructor from vector
Simple_MMC::Simple_MMC(const std::string& symbol, const std::string& location,
    const std::vector<double>& converter_params)
    : Converter(symbol, location)
{
    if (converter_params.size() < 13) {
        throw std::invalid_argument("converter_params must contain at least 13 entries.");
    }

    omega_0 = converter_params[0];
    P = converter_params[1];
    Q = converter_params[2];
    theta = converter_params[3];
    V_m = converter_params[4];
    P_dc = converter_params[5];
    V_dc = converter_params[6];
    L_arm = converter_params[7];
    R_arm = converter_params[8];
    C_arm = converter_params[9];
    N = static_cast<int>(converter_params[10]);
    L_reactor = converter_params[11];
    R_reactor = converter_params[12];

    P_min = 0.5 * P;
    P_max = 1.5 * P;
    Q_min = -P;
    Q_max = P;

    equilibrium_state = Eigen::VectorXd::Zero(number_of_states);
    Y_matrix.resize(3, 3);

    computeABCD();
}

void Simple_MMC::computeABCD()
{
    A_matrix = Eigen::MatrixXd::Zero(6, 6);
    B_matrix = Eigen::MatrixXd::Zero(6, 12);
    C_matrix = Eigen::MatrixXd::Zero(12, 6);
    D_matrix = Eigen::MatrixXd::Zero(12, 12);

    // State order: x = [ip_a ip_b ip_c in_a in_b in_c]^T
    // Inputs: u = [vdp_a vdp_b vdp_c vdn_a vdn_b vdn_c vsp_a vsp_b vsp_c vsn_a vsn_b vsn_c]

    const double A11 = -(R_arm + R_reactor) / L_arm;
    const double A12 = -(R_arm + R_reactor) / L_arm;
    const double A14 = -R_reactor / L_arm;

    A_matrix << A11, 0.0, 0.0, A14, 0.0, 0.0,
        0.0, A11, 0.0, 0.0, A14, 0.0,
        0.0, 0.0, A11, 0.0, 0.0, A14,
        A14, 0.0, 0.0, A12, 0.0, 0.0,
        0.0, A14, 0.0, 0.0, A12, 0.0,
        0.0, 0.0, A14, 0.0, 0.0, A12;

    Eigen::MatrixXd I3 = Eigen::MatrixXd::Identity(3, 3);
    Eigen::MatrixXd Z3 = Eigen::MatrixXd::Zero(3, 3);

    Eigen::MatrixXd bVdp = (1.0 / L_arm) * I3;
    Eigen::MatrixXd bVdn = -(1.0 / L_arm) * I3;
    Eigen::MatrixXd bVsp = -(1.0 / L_arm) * I3;
    Eigen::MatrixXd bVsn = (1.0 / L_arm) * I3;

    B_matrix << bVdp, Z3, bVsp, Z3,
        Z3, bVdn, Z3, bVsn;

    Eigen::MatrixXd C_ipin = Eigen::MatrixXd::Identity(6, 6);
    Eigen::MatrixXd C_im(3, 6);
    C_im << I3, I3;
    Eigen::MatrixXd C_vm = R_reactor * C_im;

    C_matrix << C_ipin,
        C_im,
        C_vm;

    D_matrix.setZero();
}



// This function simulates one time step of the MMC dynamics given the time step size, arm capacitance, arm currents, and the number of harmonics to keep.
// It returns the updated capacitor voltages and output voltages for both upper and lower arms.
// inputMatrices[0] = upper-arm current harmonics 
// inputMatrices[1] = lower-arm current harmonics 
// It returns reduced internal state matrices and reduced output matrices as lists: 
// result.stateMatrices[0] = upper-arm capacitor voltage harmonics 
// result.stateMatrices[1] = lower-arm capacitor voltage harmonics 
// result.outputMatrices[0] = upper-arm output voltage harmonics 
// result.outputMatrices[1] = lower-arm output voltage harmonics
vector<MatrixXcd> Simple_MMC::simulateTimeStep(const vector<MatrixXcd>& input, double Ts, int nKeep1, int nKeep2)
{
    MatrixXcd Ilow = input[0];
    MatrixXcd Iup = input[1];

    // -----------------------------
    // Rebuild control coefficients (no memory needed)
    // -----------------------------
    auto makeCoeffs = [&](bool isUpper) {
        MatrixXcd U = MatrixXcd::Zero(3, nKeep1);

        U(0, 1) = std::complex<double>(
            isUpper ? -0.5 : 0.5, 0.0
        );

        U(2, 0) = std::complex<double>(0.5, 0.0);

        return U;
        };

    MatrixXcd Uup = makeCoeffs(true);
    MatrixXcd Ulow = makeCoeffs(false);

    // -----------------------------
    // Harmonic projection of currents
    // -----------------------------
    MatrixXcd ProdUp = truncateHarmonics(dq_multiply(Uup, Iup), nKeep2);
    MatrixXcd ProdLow = truncateHarmonics(dq_multiply(Ulow, Ilow), nKeep2);

    // Capacitor current contribution
    MatrixXcd XinUp = truncateHarmonics(ProdUp / C_arm, nKeep2);
    MatrixXcd XinLow = truncateHarmonics(ProdLow / C_arm, nKeep2);

    // -----------------------------
    // ONLY dynamic state update (must persist)
    // -----------------------------
    MatrixXcd VcUpArm = dq_integrate(ZupOld_, XupOld_, XinUp, Ts, omega_0);
    MatrixXcd VcLowArm = dq_integrate(ZlowOld_, XlowOld_, XinLow, Ts, omega_0);

    VcUpArm = truncateHarmonics(VcUpArm, N);
    VcLowArm = truncateHarmonics(VcLowArm, N);

    // Update ONLY true state
    ZupOld_ = VcUpArm;
    ZlowOld_ = VcLowArm;

    XupOld_ = XinUp;
    XlowOld_ = XinLow;

    // -----------------------------
    // Output voltages (pure algebra)
    // -----------------------------
    MatrixXcd VoutUp = truncateHarmonics(dq_multiply(VcUpArm, Uup), nKeep1);
    MatrixXcd VoutLow = truncateHarmonics(dq_multiply(VcLowArm, Ulow), nKeep1);

    // -----------------------------
    // Return only outputs (no caching)
    // -----------------------------
    return { VoutUp, VoutLow };
}