#include <gtest/gtest.h>
#include "network/network.h"
#include "network/Bus.h"
#include "core/Include_components.h"
#include "Solver/Stability_Estimate/Stability_estimate.h"
#include "Solver/OPF/Powerflow.h"
#include "json/simulation_builder.h"

#include <algorithm>
#include <cmath>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <tuple>

class TestStabilityEstimate : public testing::Test {};

TEST_F(TestStabilityEstimate, TestOperatingPoint) {
    ///* ---------- 0 Set Network Object ---------- */
    Network net;

    ///* ---------- 1.1 Create AC Buses ---------- */
    Bus* bus1_ac = new Bus("ACBUS01", "AC1", 3);
    Bus* bus2_ac = new Bus("ACBUS02", "AC1", 3);
    Bus* bus3_ac = new Bus("ACBUS03", "AC2", 3);
    Bus* bus4_ac = new Bus("ACBUS04", "AC2", 3); // 

    ///*  ---------- 1.2 Add AC Loads  ---------- */

    std::vector<double> load_params2 = { 3859.46, 2.047, 0 };
    Load* load2 = new Load("LOAD02", "AC2", 3, load_params2);
    net.connectElementToBus(load2, 1, bus4_ac);

    ///*  ---------- 1.3 Add AC Generators  ---------- */

    /// Generator 1
    double Zsrc = 0.1;
    AC_source* src1 = new AC_source("SRC01", "AC1", 3, 345e3, Zsrc);
    net.connectElementToBus(src1, 1, bus1_ac);
    map<string, double> src_info1 = {
        {"Pmax", 250.0},
        {"Pmin", 0.0},
        {"Qmax", 10.0},
        {"Qmin", -10.0},
        {"c2", 0.11},
        {"c1", 5.0},
        {"c0", 150},
        {"Vmax", 1.06},
        {"Vmin", 1.06},
        {"Vg", 345.0 * 1.06},
        {"Ref", 1}
    };

    src1->setOPFInfo(src_info1);

    ///*  ---------- 1.4 Add Branches  ---------- */
    double ACR1 = 1.0;
    double ACX1 = 40.0;
    std::complex<double> ACZ1(ACR1, ACX1);
    Impedance* br1_ac = new Impedance("br1_ac", "AC1", 3, ACZ1);
    net.connectElementToBus(br1_ac, /*terminal=*/1, bus1_ac);
    net.connectElementToBus(br1_ac, /*terminal=*/2, bus2_ac);

    double ACR2 = 1.0;
    double ACX2 = 40.0;
    std::complex<double> ACZ2(ACR2, ACX2);
    Impedance* br2_ac = new Impedance("br2_ac", "AC2", 3, ACZ2);
    net.connectElementToBus(br2_ac, /*terminal=*/1, bus3_ac);
    net.connectElementToBus(br2_ac, /*terminal=*/2, bus4_ac);

    ///*  ---------- 2.1 Create DC Buses  ---------- */
    Bus* bus1_dc = new Bus("DCBUS01", "DC1", 2);
    Bus* bus2_dc = new Bus("DCBUS02", "DC1", 2);

    ///*  ---------- 2.2 Create DC Buses  ---------- */
    double DCR1 = 10.0;
    Impedance* br1_dc = new Impedance("br1_dc", "DC1", 2, DCR1);
    net.connectElementToBus(br1_dc, /*terminal=*/1, bus1_dc);
    net.connectElementToBus(br1_dc, /*terminal=*/2, bus2_dc);

    ///*  ---------- 2.3 Create Converters ---------- */
    const double Vll_rms = 345.0e3;
    const double Vm_peak = Vll_rms * std::sqrt(2.0 / 3.0);
    vector<double> converter_params1 = {
        2 * M_PI * 50,  // Omega (Nominal Frequency in rad/s)
        50.0 * 1e6,     // Active Power (P) in W
        0 * 1e6,        // Reactive Power (Q) in VA
        0.0,            // Theta (Voltage Angle in rad)
        Vm_peak,        // AC Voltage (V_m) peak phase in V
        50 * 1e6,       // DC power (P_dc) in W
        400.0 * 1e3,    // DC Voltage (V_dc) in V
        0.05,           // Arm Inductance (L_arm) in H
        1.07,           // Arm Resistance (R_arm) in Ω
        0.01,           // Capacitance per Submodule (C_arm) in F
        400,            // Number of Submodules (N)
        0.0005,         // Reactor Inductance (L_reactor) in H
        0.0001,         // Reactor Resistance (R_reactor) in Ω
        0.0             // Time Delay (t_delay) in seconds
    };
    std::vector<double> controller_params1 = {
        1, 0, 0.001103374, 0.00073, 1, 0, // PLL controller parameters
        0, // DC voltage controller parameters
        1, 0, 6.6667e-07, 3.3333e-04, 1, 50e6, // active power
        0, // AC voltage
        1, 0, 6.6667e-07, 3.3333e-04, 1, 0, // reactive power
        1, 0, 120, 400, 1, 0, // energy controller parameters 
        1, 0, 19.93, 4500, 1, 166.67, // zcc controller parameters 
        1, 0, 117.93, 8.5e4, 2, 666.67, 0, // occ controller parameters
        1, 0, 19.93, 4500, 2, 0, 0, // ccc controller parameters
        0 // droop control
    };
    MMC* mmc1 = new MMC("MMC1", "AC1_DC1", converter_params1, controller_params1);
    net.connectElementToBus(mmc1, 1, bus2_ac);
    net.connectElementToBus(mmc1, 2, bus1_dc);
    map<string, double> mmc1_info = { {"type_dc", 1}, {"type_ac", 1} };
    mmc1->setOPFInfo(mmc1_info);

    vector<double> converter_params2 = {
        2 * M_PI * 50,  // Omega (Nominal Frequency in rad/s)
        -50.0 * 1e6,   // Active Power (P) in W
        -10e6,              // Reactive Power (Q) in VA
        0.0,            // Theta (Voltage Angle in rad)
        Vm_peak,        // AC Voltage (V_m) peak phase in V
        -50 * 1e6,     // DC power (P_dc) in W
        400.0 * 1e3,    // DC Voltage (V_dc) in V
        0.05,           // Arm Inductance (L_arm) in H
        1.07,           // Arm Resistance (R_arm) in Ω
        0.01,           // Capacitance per Submodule (C_arm) in F
        400,            // Number of Submodules (N)
        0.0005,         // Reactor Inductance (L_reactor) in H
        0.0001,         // Reactor Resistance (R_reactor) in Ω
        0.0             // Time Delay (t_delay) in seconds
    };
    std::vector<double> controller_params2 = {
        1, 0, 0.001103374, 0.00073, 1, 0, // PLL controller parameters
        1, 0, 2, 82, 1, 400e3, // DC voltage controller parameters
        0, // active power
        0, // AC voltage
        1, 0, 6.6667e-07, 3.3333e-04, 1, -10e6, // reactive power
        1, 0, 120, 400, 1, 0, // energy controller parameters 
        1, 0, 19.93, 4500, 1, -41.66, // zcc controller parameters 
        1, 0, 117.93, 8.5e4, 2, -89.71, 0, // occ controller parameters
        1, 0, 19.93, 4500, 2, 0, 0, // ccc controller parameters
        0  // droop control
    };
    MMC* mmc2 = new MMC("MMC2", "AC2_DC1", converter_params2, controller_params2);
    net.connectElementToBus(mmc2, 1, bus3_ac);
    net.connectElementToBus(mmc2, 2, bus2_dc);
    map<string, double> mmc2_info = { {"type_dc", 2}, {"type_ac", 1} };
    mmc2->setOPFInfo(mmc2_info);

    ///*----- 3 OPF Implementatiopn ----- */
    PowerFlow pf;

    //const auto& data = net.getNetData();
    std::map<std::string, double> global_params;
    double omega = 2 * M_PI * 50;
    global_params["omega"] = omega;
    global_params["baseMVA"] = 100;
    global_params["ACbaseKV"] = 345.0; // Base voltage in kV, can be adjusted as needed
    global_params["DCbaseKV"] = 400.0; // Base voltage for DC, can be adjusted as needed
    global_params["ACZbase"] =
        global_params["ACbaseKV"] * global_params["ACbaseKV"]
        / global_params["baseMVA"];

    global_params["DCZbase"] =
        global_params["DCbaseKV"] * global_params["DCbaseKV"]
        / global_params["baseMVA"];
    pf.make_OPF(&net, global_params, false, false, false, false);
    if (!pf.opfSolved()) {
        GTEST_SKIP() << "OPF did not succeed (Gurobi license may be required)";
    }

    auto eqPowers = [](const MMC& mmc) {
        const Eigen::VectorXd x = mmc.getEquilibriumState();
        const int p = static_cast<int>(x.size()) - 12;
        EXPECT_GE(p, 0) << "MMC '" << mmc.getElementSymbol()
            << "' has no plant equilibrium after OPF";
        if (p < 0)
            return std::tuple<double, double, double>(0.0, 0.0, 0.0);
        const double vgd = mmc.getVm() * std::cos(mmc.getTheta());
        const double vgq = -mmc.getVm() * std::sin(mmc.getTheta());
        const double pac = 1.5 * (vgd * x(p) + vgq * x(p + 1));
        const double qac = 1.5 * (vgq * x(p) - vgd * x(p + 1));
        const double pdc = 3.0 * mmc.getVdc() * x(p + 2);
        return std::tuple<double, double, double>(pac, qac, pdc);
    };

    // After make_OPF, plant currents must reproduce the OPF write-back (Pac, Qac, Pdc).
    {
        const auto [pac, qac, pdc] = eqPowers(*mmc1);
        EXPECT_NEAR(pac, mmc1->getP(), 2e6) << "MMC1 Pac at equilibrium vs OPF";
        EXPECT_NEAR(qac, mmc1->getQ(), 2e6) << "MMC1 Qac at equilibrium vs OPF";
        EXPECT_NEAR(pdc, mmc1->getPdc(), 3e6) << "MMC1 Pdc at equilibrium vs OPF";
    }
    {
        const auto [pac, qac, pdc] = eqPowers(*mmc2);
        EXPECT_NEAR(qac, mmc2->getQ(), 2e6) << "MMC2 Qac at equilibrium vs OPF";
        EXPECT_NEAR(pdc, mmc2->getPdc(), 2e6) << "MMC2 Pdc at equilibrium vs OPF";
    }

    // Making Stability Estimate Object
    StabilityEstimate* stability = new StabilityEstimate();
    stability->add_areas(&net);

    // Compute equivalent impedance between two AC buses, skipping the MMCs
    auto& dc_grids = stability->get_dc_grids();
    auto& ac_grids = stability->get_ac_grids();

    complex<double> y1 = 1.0 / ACZ1;
    complex<double> yeq1 = y1 * (1.0 / Zsrc) / (y1 + 1.0 / Zsrc);
    

	MatrixXcd Y_params = stability->compute_equivalent_admittance_parameters_num(dc_grids["DC1"], 1000);
	MatrixXcd Y_expected(2, 2);
    // 2-pin DC branch, scalar ports: go and return, loop R = 2 R_pole.
    const double y_dc = 1.0 / (2.0 * DCR1);
    Y_expected << y_dc, -y_dc,
		-y_dc, y_dc;
	EXPECT_TRUE(Y_params.isApprox(Y_expected, 1e-3))
        << "DC1 Y=\n" << Y_params << "\nexpected=\n" << Y_expected;

    MatrixXcd Y_params_ac1 = stability->compute_equivalent_admittance_parameters_num(ac_grids["AC1"], 1000);
	MatrixXcd Y_expected_ac1(2, 2);
    Y_expected_ac1 << yeq1, 0,
        0, yeq1;
    EXPECT_TRUE(Y_params_ac1.isApprox(Y_expected_ac1, 1e-3));
    {
        MatrixXcd Y_elem = stability->compute_equivalent_admittance_parameters_num(
            ac_grids["AC1"], 1000, /*park_per_component=*/true);
        MatrixXcd Y_block = stability->compute_equivalent_admittance_parameters_num(
            ac_grids["AC1"], 1000, /*park_per_component=*/false);
        EXPECT_TRUE(Y_block.isApprox(Y_elem, 1e-8))
            << "AC1 block A0=\n" << Y_block
            << "\nper-component=\n" << Y_elem
            << "\ndiff=\n" << (Y_block - Y_elem);
    }

	MatrixXcd Y_params_ac2 = stability->compute_equivalent_admittance_parameters_num(ac_grids["AC2"], 1000);
	// Structural checks for the AC2 passive admittance matrix.
	// The network is symmetric (balanced 3-phase), so the dq result should be:
	//   - 2x2 (dq frame, one converter terminal)
	//   - diagonal: decoupled d and q axes for a balanced network
	//   - equal d and q entries
	//   - positive real part (passive)
	EXPECT_EQ(Y_params_ac2.rows(), 2);
	EXPECT_EQ(Y_params_ac2.cols(), 2);
	EXPECT_TRUE(Y_params_ac2.allFinite());
	// For a balanced 3-phase passive network in dq frame, Y_dq has the form:
	//   [[a, b], [-b, a]]  (equal diagonal, antisymmetric off-diagonal)
	// Diagonal entries should be equal
	EXPECT_NEAR(std::abs(Y_params_ac2(0, 0) - Y_params_ac2(1, 1)), 0.0, 1e-8);
	// Off-diagonal entries should be antisymmetric: Y(0,1) = -Y(1,0)
	EXPECT_NEAR(std::abs(Y_params_ac2(0, 1) + Y_params_ac2(1, 0)), 0.0, 1e-8);
	// Positive real part (passive element)
	EXPECT_GT(Y_params_ac2(0, 0).real(), 0.0);
    {
        MatrixXcd Y_elem = stability->compute_equivalent_admittance_parameters_num(
            ac_grids["AC2"], 1000, /*park_per_component=*/true);
        MatrixXcd Y_block = stability->compute_equivalent_admittance_parameters_num(
            ac_grids["AC2"], 1000, /*park_per_component=*/false);
        EXPECT_TRUE(Y_block.isApprox(Y_elem, 1e-8))
            << "AC2 block A0=\n" << Y_block
            << "\nper-component=\n" << Y_elem
            << "\ndiff=\n" << (Y_block - Y_elem);
    }

	MatrixXcd TF_mmc2_ac = stability->compute_transfer_function("MMC2", "AC", 1000);
	EXPECT_EQ(TF_mmc2_ac.rows(), 2);
	EXPECT_EQ(TF_mmc2_ac.cols(), 2);
	EXPECT_TRUE(TF_mmc2_ac.allFinite());


    delete stability;
}

// Park A0 on the assembled AC block vs per-element apply_transformation.
// No OPF: converters are skipped when forming the passive AC Y.
TEST_F(TestStabilityEstimate, AcBlockParkMatchesPerComponent) {
    Network net;

    Bus* bus1_ac = new Bus("ACBUS01", "AC1", 3);
    Bus* bus2_ac = new Bus("ACBUS02", "AC1", 3);
    Bus* bus3_ac = new Bus("ACBUS03", "AC2", 3);
    Bus* bus4_ac = new Bus("ACBUS04", "AC2", 3);

    Load* load2 = new Load("LOAD02", "AC2", 3, { 3859.46, 2.047, 0 });
    net.connectElementToBus(load2, 1, bus4_ac);

    AC_source* src1 = new AC_source("SRC01", "AC1", 3, 345e3, 0.1);
    net.connectElementToBus(src1, 1, bus1_ac);

    Impedance* br1_ac = new Impedance("br1_ac", "AC1", 3, std::complex<double>(1.0, 40.0));
    net.connectElementToBus(br1_ac, 1, bus1_ac);
    net.connectElementToBus(br1_ac, 2, bus2_ac);

    Impedance* br2_ac = new Impedance("br2_ac", "AC2", 3, std::complex<double>(1.0, 40.0));
    net.connectElementToBus(br2_ac, 1, bus3_ac);
    net.connectElementToBus(br2_ac, 2, bus4_ac);

    Bus* bus1_dc = new Bus("DCBUS01", "DC1", 2);
    Bus* bus2_dc = new Bus("DCBUS02", "DC1", 2);
    Impedance* br1_dc = new Impedance("br1_dc", "DC1", 2, 10.0);
    net.connectElementToBus(br1_dc, 1, bus1_dc);
    net.connectElementToBus(br1_dc, 2, bus2_dc);

    const double Vm_peak = 345.0e3 * std::sqrt(2.0 / 3.0);
    std::vector<double> converter_params = {
        2 * M_PI * 50, 50e6, 0, 0.0, Vm_peak, 50e6, 400e3,
        0.05, 1.07, 0.01, 400, 0.0005, 0.0001, 0.0
    };
    std::vector<double> controller_params = {
        1, 0, 0.001103374, 0.00073, 1, 0,
        0,
        1, 0, 6.6667e-07, 3.3333e-04, 1, 50e6,
        0,
        1, 0, 6.6667e-07, 3.3333e-04, 1, 0,
        1, 0, 120, 400, 1, 0,
        1, 0, 19.93, 4500, 1, 166.67,
        1, 0, 117.93, 8.5e4, 2, 666.67, 0,
        1, 0, 19.93, 4500, 2, 0, 0,
        0
    };
    MMC* mmc1 = new MMC("MMC1", "AC1_DC1", converter_params, controller_params);
    net.connectElementToBus(mmc1, 1, bus2_ac);
    net.connectElementToBus(mmc1, 2, bus1_dc);

    std::vector<double> converter_params2 = converter_params;
    converter_params2[1] = -50e6;
    converter_params2[5] = -50e6;
    MMC* mmc2 = new MMC("MMC2", "AC2_DC1", converter_params2, controller_params);
    net.connectElementToBus(mmc2, 1, bus3_ac);
    net.connectElementToBus(mmc2, 2, bus2_dc);

    StabilityEstimate stability;
    stability.add_areas(&net);
    auto& ac_grids = stability.get_ac_grids();
    ASSERT_TRUE(ac_grids.count("AC1"));
    ASSERT_TRUE(ac_grids.count("AC2"));

    const double freqs[] = { 100.0, 1000.0, 5000.0 };
    for (double f : freqs) {
        MatrixXcd Yb1 = stability.compute_equivalent_admittance_parameters_num(
            ac_grids["AC1"], f, /*park_per_component=*/false);
        MatrixXcd Ye1 = stability.compute_equivalent_admittance_parameters_num(
            ac_grids["AC1"], f, /*park_per_component=*/true);
        EXPECT_TRUE(Yb1.isApprox(Ye1, 1e-8))
            << "AC1 at " << f << " Hz, block=\n" << Yb1
            << "\nper-component=\n" << Ye1
            << "\ndiff=\n" << (Yb1 - Ye1);

        MatrixXcd Yb2 = stability.compute_equivalent_admittance_parameters_num(
            ac_grids["AC2"], f, /*park_per_component=*/false);
        MatrixXcd Ye2 = stability.compute_equivalent_admittance_parameters_num(
            ac_grids["AC2"], f, /*park_per_component=*/true);
        EXPECT_TRUE(Yb2.isApprox(Ye2, 1e-8))
            << "AC2 at " << f << " Hz, block=\n" << Yb2
            << "\nper-component=\n" << Ye2
            << "\ndiff=\n" << (Yb2 - Ye2);
    }
}

TEST_F(TestStabilityEstimate, P2PPowerImpedanceBlockVsPerComponent) {
    const std::filesystem::path jsonPath =
        std::filesystem::path(__FILE__).parent_path().parent_path()
        / "benchmarking" / "powerimpedance" / "harmony" / "p2p.json";
    ASSERT_TRUE(std::filesystem::exists(jsonPath)) << jsonPath;

    std::ifstream in(jsonPath);
    ASSERT_TRUE(in) << jsonPath;
    const JSON config = JSON::parse(in);

    Network net;
    SimulationBuilder builder;
    ASSERT_NO_THROW(builder.buildFromJSON(config, net));

    StabilityEstimate stability;
    stability.add_areas(&net);
    auto& ac_grids = stability.get_ac_grids();
    ASSERT_TRUE(ac_grids.count("AC1"));
    ASSERT_TRUE(ac_grids.count("AC2"));

    auto relErr = [](const MatrixXcd& a, const MatrixXcd& b) {
        const double nb = b.norm();
        if (nb < 1e-18)
            return a.norm();
        return (a - b).norm() / nb;
    };

    double maxRelY = 0.0;
    const double freqs[] = { 10.0, 50.0, 100.0, 500.0, 1000.0 };
    for (double f : freqs) {
        for (const char* area : { "AC1", "AC2" }) {
            MatrixXcd Yb = stability.compute_equivalent_admittance_parameters_num(
                ac_grids[area], f, /*park_per_component=*/false);
            MatrixXcd Ye = stability.compute_equivalent_admittance_parameters_num(
                ac_grids[area], f, /*park_per_component=*/true);
            ASSERT_EQ(Yb.rows(), Ye.rows()) << area << " at " << f << " Hz";
            ASSERT_EQ(Yb.cols(), Ye.cols()) << area << " at " << f << " Hz";
            const double rel = relErr(Yb, Ye);
            maxRelY = std::max(maxRelY, rel);
            EXPECT_LT(rel, 1e-6)
                << area << " at " << f << " Hz, rel=" << rel
                << "\nblock=\n" << Yb << "\nper-component=\n" << Ye;
        }
    }
    std::cout << "[P2P] max relative Yeq error (block vs per-component): "
              << maxRelY << "\n";

    const int nTime = 11;
    const double f0 = 10.0, f1 = 1000.0;
    auto sweepMs = [&](bool perComponent) {
        const auto t0 = std::chrono::steady_clock::now();
        for (int k = 0; k < nTime; ++k) {
            const double f = f0 * std::pow(f1 / f0, static_cast<double>(k) / (nTime - 1));
            (void)stability.compute_equivalent_admittance_parameters_num(
                ac_grids["AC1"], f, perComponent);
            (void)stability.compute_equivalent_admittance_parameters_num(
                ac_grids["AC2"], f, perComponent);
        }
        const auto t1 = std::chrono::steady_clock::now();
        return std::chrono::duration<double, std::milli>(t1 - t0).count();
    };
    const double msElem = sweepMs(true);
    const double msBlock = sweepMs(false);
    std::cout << "[P2P] Yeq sweep " << nTime << " freqs x AC1+AC2: per-component "
              << msElem << " ms, block " << msBlock << " ms"
              << " (block/per-component = " << (msElem > 0.0 ? msBlock / msElem : 0.0)
              << ")\n";
}

TEST_F(TestStabilityEstimate, AcBlockYeffDiffersFromA0WhenUnbalanced) {
    Network net;

    Bus* gnd = new Bus("gnd", "GND", 1);
    Bus* bsrc = new Bus("Bsrc", "AC1", 3);
    Bus* bmid = new Bus("Bmid", "AC1", 3);
    Bus* bpcc = new Bus("Bpcc", "AC1", 3);
    Bus* bdc = new Bus("Bdc", "DC1", 2);

    AC_source* src = new AC_source("Vs_ac", "AC1", 3, 200.0, 0.5);
    net.connectElementToBus(src, 1, bsrc);

    Resistor* r1 = new Resistor("R1", "AC1", 3, { 1.0, 1.0, 5.0 });
    net.connectElementToBus(r1, 1, bsrc);
    net.connectElementToBus(r1, 2, bmid);

    Inductor* l1 = new Inductor("L1", "AC1", 3, { 0.01 });
    net.connectElementToBus(l1, 1, bmid);
    net.connectElementToBus(l1, 2, bpcc);

    Capacitor* c1 = new Capacitor("C1", "AC1", 3, { 20e-6 });
    net.connectElementToBus(c1, 1, bpcc);
    net.connectElementToBus(c1, 2, gnd);

    DC_source* vdc = new DC_source("Vs_dc", "DC1", 2, std::vector<double>{ 200.0, -200.0 }, 0.1);
    net.connectElementToBus(vdc, 1, bdc);

    std::vector<double> converter_params = {
        2 * M_PI * 50, 0.0, 0.0, 0.0, 200.0, 0.0, 400.0,
        0.0529, 0.1663, 0.0017568, 36.0, 0.01, 10.0, 0.0
    };
    std::vector<double> controller_params = {
        0, 0, 1, 0, 0.000257, 0.0032, 1, 1000.0,
        0, 0, 0, 0,
        1, 0, 48.0, 480.0, 2, 0.0, 0.0,
        0, 0
    };
    MMC* mmc = new MMC("MMC1", "AC1_DC1", converter_params, controller_params);
    net.connectElementToBus(mmc, 1, bpcc);
    net.connectElementToBus(mmc, 2, bdc);

    StabilityEstimate stability;
    stability.add_areas(&net);
    auto& ac_grids = stability.get_ac_grids();
    ASSERT_TRUE(ac_grids.count("AC1"));

    const double freqs[] = { 80.0, 200.0, 400.0 };
    double maxRel = 0.0;
    for (double f : freqs) {
        MatrixXcd Ya0 = stability.compute_equivalent_admittance_parameters_num(
            ac_grids["AC1"], f, /*park_per_component=*/false, /*yeff=*/false);
        MatrixXcd Yeff = stability.compute_equivalent_admittance_parameters_num(
            ac_grids["AC1"], f, /*park_per_component=*/false, /*yeff=*/true);
        ASSERT_EQ(Ya0.rows(), 2);
        ASSERT_EQ(Yeff.rows(), 2);
        EXPECT_TRUE(Ya0.allFinite()) << "A0 at " << f << " Hz";
        EXPECT_TRUE(Yeff.allFinite()) << "Yeff at " << f << " Hz";
        const double nb = Ya0.norm();
        const double rel = nb < 1e-18 ? Yeff.norm() : (Yeff - Ya0).norm() / nb;
        maxRel = std::max(maxRel, rel);
    }
    EXPECT_GT(maxRel, 1e-6)
        << "expected unbalance C± correction, max rel=" << maxRel;
    std::cout << "[Yeff] max rel ||Yeff-A0||/||A0|| = " << maxRel << "\n";
}