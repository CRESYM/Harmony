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

// This function creates the control coefficient matrix U for the MMC arm control
// based on the number of columns and the arm type (upper or lower).
MatrixXcd Simple_MMC::makeArmControlCoeffs(int nCols, ArmType armType)
{
    if (nCols < 2) {
        throw std::invalid_argument("Control coefficient matrix needs at least 2 columns.");
    }

    MatrixXcd U = MatrixXcd::Zero(3, nCols);

    U(0, 1) = std::complex<double>(
        armType == ArmType::Upper ? -0.5 : 0.5, 0.0
    );

    U(2, 0) = std::complex<double>(0.5, 0.0);

    return U;
}



// This function simulates one time step of the MMC dynamics given the time step size, arm capacitance, arm currents, and the number of harmonics to keep.
// It returns the updated capacitor voltages and output voltages for both upper and lower arms.
Simple_MMC::StepResult Simple_MMC::simulateTimeStep(  double Ts, double C, const MatrixXcd& Iup, const MatrixXcd& Ilow, int nArm, int nKeep)
{
    if (Ts <= 0.0) {
        throw std::invalid_argument("Ts must be positive.");
    }

    if (C <= 0.0) {
        throw std::invalid_argument("C must be positive.");
    }

    if (nArm <= 0) {
        throw std::invalid_argument("nArm must be positive.");
    }

    if (nKeep < 2) {
        throw std::invalid_argument("nKeep must be at least 2.");
    }

    if (Iup.rows() != 3 || Iup.cols() != nKeep) {
        throw std::runtime_error("Iup must be of size 3 x nKeep.");
    }

    if (Ilow.rows() != 3 || Ilow.cols() != nKeep) {
        throw std::runtime_error("Ilow must be of size 3 x nKeep.");
    }

    // Reinitialize harmonic memory if dimensions changed or if first call
    if (currentNArm_ != nArm || currentNKeep_ != nKeep) {
        Uup_ = makeArmControlCoeffs(nKeep, ArmType::Upper);
        Ulow_ = makeArmControlCoeffs(nKeep, ArmType::Lower);

        ZupOld_ = MatrixXcd::Zero(3, nArm);
        XupOld_ = MatrixXcd::Zero(3, nArm);
        ZlowOld_ = MatrixXcd::Zero(3, nArm);
        XlowOld_ = MatrixXcd::Zero(3, nArm);

        lastVcUp_ = MatrixXcd::Zero(3, nKeep);
        lastVcLow_ = MatrixXcd::Zero(3, nKeep);
        lastVoutUp_ = MatrixXcd::Zero(3, nKeep);
        lastVoutLow_ = MatrixXcd::Zero(3, nKeep);

        currentNArm_ = nArm;
        currentNKeep_ = nKeep;
    }

    // Multiply arm currents by arm control coefficients
    MatrixXcd ProdUp = dq_.multiply(Uup_, Iup);
    MatrixXcd ProdLow = dq_.multiply(Ulow_, Ilow);

    // Keep arm-order harmonics
    ProdUp = truncateHarmonics(ProdUp, nArm);
    ProdLow = truncateHarmonics(ProdLow, nArm);

    // Capacitor-voltage derivative inputs
    MatrixXcd XinUp = truncateHarmonics(ProdUp * (1.0 / C), nArm);
    MatrixXcd XinLow = truncateHarmonics(ProdLow * (1.0 / C), nArm);

    // Integrate capacitor voltages
    MatrixXcd VcUpArm = dq_.integrate(ZupOld_, XupOld_, XinUp, Ts, omega_0);
    MatrixXcd VcLowArm = dq_.integrate(ZlowOld_, XlowOld_, XinLow, Ts, omega_0);

    VcUpArm = truncateHarmonics(VcUpArm, nArm);
    VcLowArm = truncateHarmonics(VcLowArm, nArm);

    // Update stored integration memory
    ZupOld_ = VcUpArm;
    XupOld_ = XinUp;
    ZlowOld_ = VcLowArm;
    XlowOld_ = XinLow;

    // Compute output arm voltages
    MatrixXcd VoutUpFull = dq_.multiply(VcUpArm, Uup_);
    MatrixXcd VoutLowFull = dq_.multiply(VcLowArm, Ulow_);

    MatrixXcd VoutUp = truncateHarmonics(VoutUpFull, nKeep);
    MatrixXcd VoutLow = truncateHarmonics(VoutLowFull, nKeep);

    // Store latest reduced states/results
    lastVcUp_ = truncateHarmonics(VcUpArm, nKeep);
    lastVcLow_ = truncateHarmonics(VcLowArm, nKeep);
    lastVoutUp_ = VoutUp;
    lastVoutLow_ = VoutLow;

    StepResult result;
   /* result.VcUp = lastVcUp_;
    result.VcLow = lastVcLow_;*/
    result.VoutUp = lastVoutUp_;
    result.VoutLow = lastVoutLow_;

    return result;
}