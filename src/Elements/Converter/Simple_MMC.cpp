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



// ---------------------------------------------------------------------------
//  simulateTimeStep
// ---------------------------------------------------------------------------
//
//  Pipeline (from MMC_DSS_Pipeline document):
//
//    Step 1:  v_mod  = K · i                        (control multiplication)
//    Step 2:  v_arm  = truncate(v_mod, nArm)         (truncate to arm harmonics)
//    Step 3:  v_c'   = (1/C) · v_arm                 (capacitor gain)
//    Step 4:  v_c    = integrate(v_c')               (capacitor dynamics)
//    Step 5:  v_keep = truncate(v_c, nKeep)          (truncate to DSS harmonics)
//    Step 6:  v_out  = K · v_keep                    (final control multiplication)
//
//  Arguments:
//    input[0] = Ilow  (3 x nKeep1)   lower-arm current from DSS
//    input[1] = Iup   (3 x nKeep1)   upper-arm current from DSS
//    Ts       = time step [s]
//    nKeep1   = nKeep  (DSS-level harmonics)
//    nKeep2   = nArm   (arm-level harmonics for internal dynamics)
//
//  Returns:
//    [0] = VoutUp   (3 x nKeep1)
//    [1] = VoutLow  (3 x nKeep1)
//
vector<MatrixXcd> Simple_MMC::simulateTimeStep(const vector<MatrixXcd>& input, double Ts, int nKeep1, int nKeep2)
{
    // ---- lazy-init persistent state ----
    if (Zold.empty()) {
        Zold = { MatrixXcd::Zero(3, nKeep2),
                 MatrixXcd::Zero(3, nKeep2) };
        Xold = { MatrixXcd::Zero(3, nKeep2),
                 MatrixXcd::Zero(3, nKeep2) };
    }

    // ---- control coefficient matrices ----
    //   Upper arm:  K(0,1) = -0.5  (fundamental),  K(2,0) = 0.5  (DC)
    //   Lower arm:  K(0,1) = +0.5  (fundamental),  K(2,0) = 0.5  (DC)
    MatrixXcd K_up = MatrixXcd::Zero(3, nKeep1);
    MatrixXcd K_low = MatrixXcd::Zero(3, nKeep1);

    K_up(0, 1) = std::complex<double>(-0.5, 0.0);
    K_up(2, 0) = std::complex<double>(0.5, 0.0);

    K_low(0, 1) = std::complex<double>(0.5, 0.0);
    K_low(2, 0) = std::complex<double>(0.5, 0.0);

    // ---- arm ordering ----
    //  Index 0 = upper arm  (K_up x Iup),   output[0] = VoutUp
    //  Index 1 = lower arm  (K_low x Ilow),  output[1] = VoutLow
    //  input = {Ilow, Iup}  ->  inputIdx maps arm to correct current
    std::vector<MatrixXcd> K = { K_up, K_low };
    const int inputIdx[2] = { 1, 0 };   // arm 0(up) -> Iup=input[1]
    // arm 1(low) -> Ilow=input[0]

    std::vector<MatrixXcd> Vout(2);

    for (int arm = 0; arm < 2; ++arm)
    {
        // Step 1: control multiplication
        MatrixXcd v_mod = dq_multiply(K[arm], input[inputIdx[arm]]);

        // Step 2: truncate to nArm
        MatrixXcd v_arm = truncateHarmonics(v_mod, nKeep2);

        // Step 3: capacitor gain
        MatrixXcd v_c_dot = v_arm / C_arm;

        // Step 4: integration (trapezoidal — Xold holds PREVIOUS step's input)
        MatrixXcd v_c = dq_integrate(Zold[arm], Xold[arm], v_c_dot, Ts, omega_0);

        // Update persistent state (keep at nArm resolution for next step)
        Zold[arm] = truncateHarmonics(v_c, nKeep2);
        Xold[arm] = v_c_dot;

        // Step 5: truncate to nKeep
        MatrixXcd v_keep = truncateHarmonics(v_c, nKeep1);

        // Step 6: final control multiplication
        Vout[arm] = truncateHarmonics(dq_multiply(v_keep, K[arm]), nKeep1);
    }

    return Vout;   // {VoutUp, VoutLow}
}


// MNA stamping and other state-space manipulations 
void Simple_MMC::writeMNAmatrix(SymEngine::DenseMatrix& matrix, std::unordered_map<Bus*, int>& bus_indices, int location,
    std::map<Element*, std::vector<RCP<const Basic>>>& symbols_map)
{
    std::vector<Bus*> buses = getBuses();
    Bus* node1 = buses.size() > 0 ? buses[0] : nullptr;
    Bus* node2 = buses.size() > 1 ? buses[1] : nullptr;

    if (node1 && (bus_indices.count(node1) != 0)) {
        int n1 = bus_indices[node1];
        for (int i = 0; i < input_pins; ++i) {
            matrix.set(n1 + i, n1 + i, addSym(matrix.get(n1 + i, n1 + i), inv(real_double(R_values[i]))));
        }

        if (node2 && (bus_indices.count(node2) != 0)) {
            int n2 = bus_indices[node2];
            for (int i = 0; i < output_pins; ++i) {
                matrix.set(n1 + i, n2 + i, subSym(matrix.get(n1 + i, n2 + i), inv(real_double(R_values[i]))));
                matrix.set(n2 + i, n1 + i, subSym(matrix.get(n2 + i, n1 + i), inv(real_double(R_values[i]))));
                matrix.set(n2 + i, n2 + i, addSym(matrix.get(n2 + i, n2 + i), inv(real_double(R_values[i]))));
            }
        }
    }
    else if (node2 && (bus_indices.count(node2) != 0)) {
        int n2 = bus_indices[node2];
        for (int i = 0; i < output_pins; ++i) {
            matrix.set(n2 + i, n2 + i, addSym(matrix.get(n2 + i, n2 + i), inv(real_double(R_values[i]))));
        }
    }
}
