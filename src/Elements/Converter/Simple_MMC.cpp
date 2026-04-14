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
 * @param timeDelay Modulation time delay (s).
 */
Simple_MMC::Simple_MMC(const std::string& symbol, const std::string& location,
    double omega, double activePower, double reactivePower,
    double angle, double acVoltage, double Pdc, double dcVoltage,
    double armInductance, double armResistance, double armCapacitance,
    int numSubmodules, double reactorInductance, double reactorResistance)
    : Converter(symbol, location) // AC side - input pins; DC side - output pins
{
    omega_0 = omega; P = activePower; Q = reactivePower; theta = angle; V_m = acVoltage; V_dc = dcVoltage; P_dc = Pdc;
    L_arm = armInductance; R_arm = armResistance; C_arm = armCapacitance;
    N = numSubmodules; L_reactor = reactorInductance; R_reactor = reactorResistance; 
    // Initialize active and reactive power limits for power flow calculations
    P_min = 0.5 * P;
    P_max = 1.5 * P;
    Q_min = -P;
    Q_max = P;

    A_matrix = Eigen::MatrixXd::Zero(6, 6);
    B_matrix = Eigen::MatrixXd::Zero(6, 12);
    C_matrix = Eigen::MatrixXd::Zero(12, 6);
    D_matrix = Eigen::MatrixXd::Zero(12, 12);


	// State order: x = [ip_a ip_b ip_c in_a in_b in_c]^T
	// iu_a, iu_b, iu_c are the currents in the upper arms of phases a, b, c
	// il_a, il_b, il_c are the currents in the lower arms of phases a, b, c
    // u = [vdp_a vdp_b vdp_c vdn_a vdn_b vdn_c vsp_a vsp_b vsp_c vsn_a vsn_b vsn_c]

    // Form matrix A
	double A11 = -(R_arm + R_reactor) / L_arm; // Diagonal entries for upper arm currents
	double A12 = -(R_arm + R_reactor) / L_arm; // Coupling between upper and lower arm currents
	double A14 = -R_reactor / L_arm; // Coupling between upper and lower arm currents via reactor
    
    A_matrix << A11, 0, 0, A14, 0, 0,
                0, A11, 0, 0, A14, 0,
                0, 0, A11, 0, 0, A14,
                A14, 0, 0, A12, 0, 0,
                0, A14, 0, 0, A12, 0,
                0, 0, A14, 0, 0, A12;

	// Form matrix B 
    MatrixXd I3 = MatrixXd::Identity(3, 3);
    MatrixXd Z3 = MatrixXd::Zero(3, 3);

    // Each phase now has its own vdp and vdn input
    MatrixXd bVdp = (1.0 / L_arm) * I3;   // affects ip_a, ip_b, ip_c
    MatrixXd bVdn = -(1.0 / L_arm) * I3;  // affects in_a, in_b, in_c
    MatrixXd bVsp = -(1.0 / L_arm) * I3;  // affects ip_a, ip_b, ip_c
    MatrixXd bVsn = (1.0 / L_arm) * I3;   // affects in_a, in_b, in_c

    B_matrix << bVdp, Z3, bVsp, Z3,
        Z3, bVdn, Z3, bVsn;

	// Form matrix C
    MatrixXd C_ipin = MatrixXd::Identity(6, 6);
    MatrixXd C_im(3, 6);
    C_im << I3, I3;
    MatrixXd C_vm = R_reactor * C_im;

    C_matrix << C_ipin, C_im, C_vm;

    Y_matrix.resize(3, 3);
}

/**
 * @brief Simple MMC constructor with converter parameter vector.
 * @param symbol Element symbol/name.
 * @param converter_params Vector of converter parameters.
 */
Simple_MMC::Simple_MMC(const std::string& symbol, const std::string& location, const std::vector<double>& converter_params)
    : Converter(symbol, location) // AC side - input pins; DC side - output pins 
{
    omega_0 = converter_params[0]; P = converter_params[1]; Q = converter_params[2]; theta = converter_params[3];
    V_m = converter_params[4]; P_dc = converter_params[5]; V_dc = converter_params[6];
    L_arm = converter_params[7]; R_arm = converter_params[8]; C_arm = converter_params[9];
    N = static_cast<int>(converter_params[10]); L_reactor = converter_params[11];
    R_reactor = converter_params[12];

    // Initialize active and reactive power limits for power flow calculations
    P_min = 0.5 * P;
    P_max = 1.5 * P;
    Q_min = -P;
    Q_max = P;

    // Initialize equilibrium state vector
    equilibrium_state = Eigen::VectorXd::Zero(6); // 6 dynamic states

    A_matrix = Eigen::MatrixXd::Zero(6, 6);
    B_matrix = Eigen::MatrixXd::Zero(6, 12);
    C_matrix = Eigen::MatrixXd::Zero(12, 6);
    D_matrix = Eigen::MatrixXd::Zero(12, 12);


    // State order: x = [ip_a ip_b ip_c in_a in_b in_c]^T
    // iu_a, iu_b, iu_c are the currents in the upper arms of phases a, b, c
    // il_a, il_b, il_c are the currents in the lower arms of phases a, b, c
    // u = [vdp_a vdp_b vdp_c vdn_a vdn_b vdn_c vsp_a vsp_b vsp_c vsn_a vsn_b vsn_c]

    // Form matrix A
    double A11 = -(R_arm + R_reactor) / L_arm; // Diagonal entries for upper arm currents
    double A12 = -(R_arm + R_reactor) / L_arm; // Coupling between upper and lower arm currents
    double A14 = -R_reactor / L_arm; // Coupling between upper and lower arm currents via reactor

    A_matrix << A11, 0, 0, A14, 0, 0,
        0, A11, 0, 0, A14, 0,
        0, 0, A11, 0, 0, A14,
        A14, 0, 0, A12, 0, 0,
        0, A14, 0, 0, A12, 0,
        0, 0, A14, 0, 0, A12;

    // Form matrix B 
    MatrixXd I3 = MatrixXd::Identity(3, 3);
    MatrixXd Z3 = MatrixXd::Zero(3, 3);

    // Each phase now has its own vdp and vdn input
    MatrixXd bVdp = (1.0 / L_arm) * I3;   // affects ip_a, ip_b, ip_c
    MatrixXd bVdn = -(1.0 / L_arm) * I3;  // affects in_a, in_b, in_c
    MatrixXd bVsp = -(1.0 / L_arm) * I3;  // affects ip_a, ip_b, ip_c
    MatrixXd bVsn = (1.0 / L_arm) * I3;   // affects in_a, in_b, in_c

    B_matrix << bVdp, Z3, bVsp, Z3,
        Z3, bVdn, Z3, bVsn;

    // Form matrix C
    MatrixXd C_ipin = MatrixXd::Identity(6, 6);
    MatrixXd C_im(3, 6);
    C_im << I3, I3;
    MatrixXd C_vm = R_reactor * C_im;

    C_matrix << C_ipin, C_im, C_vm;

    Y_matrix.resize(3, 3);
    //cout << "MMC initialized with " << number_of_states << " states." << endl;
};


VectorXd Simple_MMC::simulateTimeStep(const Eigen::VectorXd& initial_state, double dt)
{
    // Placeholder for the actual simulation logic
    // This function should implement the time-stepping logic for the MMC based on the state-space model
    // and the control inputs. The current implementation just returns a zero vector of appropriate size.
    // In a complete implementation, this function would:
    // 1. Compute control inputs based on the current state and reference signals.
    // 2. Use the state-space matrices (A, B, C, D) to compute the next state and output.
    // 3. Update any internal states or memory as needed for the next time step.
    return VectorXd::Zero(6); // Return a zero vector of size equal to the number of states
}

//MatrixXcd Simple_MMC::makeUpperControlCoeffs(Eigen::Index nCols) const
//{
//    if (nCols < 2) {
//        throw std::invalid_argument("Upper control coefficient matrix needs at least 2 columns.");
//    }
//
//    MatrixXcd A = MatrixXcd::Zero(3, nCols);
//    MatrixXcd B = MatrixXcd::Zero(3, nCols);
//
//    A(0, 1) = complex<double>(-1.0, 0.0);
//    B(2, 0) = complex<double>(1.0, 0.0);
//
//    return 0.5 * (A + B);
//}
//
//MatrixXcd Simple_MMC::makeLowerControlCoeffs(Eigen::Index nCols) const
//{
//    if (nCols < 2) {
//        throw std::invalid_argument("Lower control coefficient matrix needs at least 2 columns.");
//    }
//
//    MatrixXcd A = MatrixXcd::Zero(3, nCols);
//    MatrixXcd B = MatrixXcd::Zero(3, nCols);
//
//    A(0, 1) = complex<double>(1.0, 0.0);
//    B(2, 0) = complex<double>(1.0, 0.0);
//
//    return 0.5 * (A + B);
//}
//
//MatrixXcd Simple_MMC::truncateHarmonics(const MatrixXcd& X,
//    Eigen::Index nColsToKeep) const
//{
//    if (nColsToKeep <= 0) {
//        throw std::invalid_argument("nColsToKeep must be positive.");
//    }
//
//    MatrixXcd Y = MatrixXcd::Zero(X.rows(), nColsToKeep);
//    const Eigen::Index colsToCopy = std::min<Eigen::Index>(X.cols(), nColsToKeep);
//    Y.leftCols(colsToCopy) = X.leftCols(colsToCopy);
//
//    return Y;
//}

//StepResult Simple_MMC::step(const MatrixXcd& u1,
//    const MatrixXcd& u2,
//    const VectorXi& brkVec)
//{
//
//    if (brkVec.size() != swType_.size()) {
//        throw std::invalid_argument("brkVec size must match switch vector size.");
//    }
//
//    if (u1.rows() != 3 || u2.rows() != 3) {
//        throw std::invalid_argument("u1 and u2 must have 3 rows.");
//    }
//
//    if (u1.cols() != nKeepMMC_ || u2.cols() != nKeepMMC_) {
//        throw std::invalid_argument("u1 and u2 column count must match nKeepMMC.");
//    }
//
//    MatrixXcd u3 = VoutUpForMMC_;
//    MatrixXcd u4 = VoutLowForMMC_;
//
//    MatrixXcd u = stack_u_4x_3xN(u1, u2, u3, u4);
//
//    MatrixXcd y = dq_.DSSS(
//        A_matrix, B_matrix, C_matrix, D_matrix,
//        swOnRes_, swOffRes_,
//        swType_, brkVec,
//        u, xo_,
//        dt_, f0_);
//
//    if (y.rows() < 9 || y.cols() < nKeepMMC_) {
//        throw std::runtime_error("MMC DSSS output has unexpected dimensions.");
//    }
//
//    MatrixXcd Iup = y.block(3, 0, 3, nKeepMMC_);
//    MatrixXcd Ilow = y.block(6, 0, 3, nKeepMMC_);
//
//    MatrixXcd ProdUp = dq_.multiply(Uup_, Iup);
//    MatrixXcd ProdLow = dq_.multiply(Ulow_, Ilow);
//
//    ProdUp = truncateHarmonics(ProdUp, nArm_);
//    ProdLow = truncateHarmonics(ProdLow, nArm_);
//
//    MatrixXcd XinUp = truncateHarmonics(ProdUp * (1.0 / C_), nArm_);
//    MatrixXcd XinLow = truncateHarmonics(ProdLow * (1.0 / C_), nArm_);
//
//    MatrixXcd VcUp = dq_.integrate(ZupOld_, XupOld_, XinUp, dt_, w_);
//    MatrixXcd VcLow = dq_.integrate(ZlowOld_, XlowOld_, XinLow, dt_, w_);
//
//    VcUp = truncateHarmonics(VcUp, nArm_);
//    VcLow = truncateHarmonics(VcLow, nArm_);
//
//    ZupOld_ = VcUp;
//    XupOld_ = XinUp;
//    ZlowOld_ = VcLow;
//    XlowOld_ = XinLow;
//
//    MatrixXcd VoutUpFull = dq_.multiply(VcUp, Uup_);
//    MatrixXcd VoutLowFull = dq_.multiply(VcLow, Ulow_);
//
//    VoutUpForMMC_ = truncateHarmonics(VoutUpFull, nKeepMMC_);
//    VoutLowForMMC_ = truncateHarmonics(VoutLowFull, nKeepMMC_);
//
//    StepResult result;
//    result.y_mmc = y;
//    result.i_up = Iup;
//    result.i_low = Ilow;
//    result.vc_up = VcUp;
//    result.vc_low = VcLow;
//    result.vout_up = VoutUpForMMC_;
//    result.vout_low = VoutLowForMMC_;
//
//    return result;
//}