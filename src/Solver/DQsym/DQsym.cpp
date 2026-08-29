/**
 * @file DQsym.cpp
 * @brief Implementation of Dynamic phasor (DQsym) time-domain solver with switch handling.
 */
#include "DQsym.h"
#include "core/Constants.h"

#include "network/network.h"      // For access to the Network class and its members
#include "core/Include_components.h"
#include "ui/Visualization.h"
#include "Solver/Stability_Estimate/Stability_estimate.h"

#include <algorithm>

namespace {

int nearestTimeIndex(const std::vector<double>& time, double t_sel)
{
    if (time.empty())
        return 0;
    auto it = std::lower_bound(time.begin(), time.end(), t_sel);
    if (it == time.end())
        return static_cast<int>(time.size()) - 1;
    if (it == time.begin())
        return 0;
    const int i1 = static_cast<int>(std::distance(time.begin(), it));
    const int i0 = i1 - 1;
    return (std::abs(time[i1] - t_sel) < std::abs(time[i0] - t_sel)) ? i1 : i0;
}

void restoreMmcControllers(Network* net, const DQsymResult& result, int k)
{
    if (!net)
        return;
    for (auto& [name, elem] : net->get_converters()) {
        MMC* mmc = dynamic_cast<MMC*>(elem);
        if (!mmc || !result.stateHist.count(name))
            continue;
        const Eigen::MatrixXd& X = result.stateHist.at(name);
        if (k < 0 || k >= X.cols())
            continue;
        const int n_ctrl = mmc->getNumberOfInternalStates() - 12;
        if (n_ctrl > 0 && X.rows() >= n_ctrl) {
            mmc->x_ctrl_dqsym_ = X.col(k).head(n_ctrl);
            mmc->dqsym_initialized_ = true;
        }
    }
}

void analyzeSnapshot(Network* net, const DQsymResult& result, double t, bool plotResults,
    double fStart = 0.1, double fEnd = 10000.0, int fPoints = 500)
{
    if (!net)
        throw std::runtime_error("analyzeSnapshot: null network.");
    if (result.time.empty())
        throw std::runtime_error("analyzeSnapshot: empty time history.");
    if (result.stateHist.empty())
        throw std::runtime_error("analyzeSnapshot: no converter snapshots (need an MMC in the DQsym run).");

    const int k = nearestTimeIndex(result.time, t);
    std::cout << "[DQsym] Snapshot linearization at t = "
        << result.time[k] << " s (step " << k << ")\n";

    int nConv = 0;
    for (auto& [name, elem] : net->get_converters()) {
        MMC* mmc = dynamic_cast<MMC*>(elem);
        if (!mmc || !result.stateHist.count(name))
            continue;
        const Eigen::MatrixXd& X = result.stateHist.at(name);
        const Eigen::MatrixXd& U = result.inputHist.at(name);
        if (k >= X.cols() || k >= U.cols())
            continue;
        try {
            mmc->setEquilibriumState(X.col(k), U.col(k));
            mmc->computeABCD();
            mmc->checkStability();
            mmc->printEigenvalues();
            try {
                mmc->writeFile(fStart, fEnd, fPoints);
            }
            catch (const std::exception& ex) {
                std::cout << "[DQsym] Y-parameter CSV for '" << name
                    << "' failed: " << ex.what() << "\n";
            }
            ++nConv;
            if (plotResults) {
                mmc->plotEigenvalues();
                mmc->plotParticipationFactors();
            }
        }
        catch (const std::exception& ex) {
            std::cout << "[DQsym] Linearization of '" << name << "' failed: " << ex.what() << "\n";
        }
    }
    if (nConv == 0)
        throw std::runtime_error("analyzeSnapshot: no MMC snapshot columns at this time.");

    try {
        net->is_area_empty() ? net->add_areas() : void();
        StabilityEstimate stability;
        stability.add_areas(net);
        for (auto& [name, elem] : net->get_converters()) {
            if (!result.stateHist.count(name))
                continue;
            auto trySide = [&](const char* loc) {
                bool wrote = false;
                try {
                    stability.writeFileTF(name, loc, fStart, fEnd, fPoints);
                    wrote = true;
                }
                catch (const std::exception& ex) {
                    std::cout << "[DQsym] Transfer-function CSV at " << loc << " for '"
                        << name << "' failed: " << ex.what() << "\n";
                }
                if (plotResults) {
                    try {
                        stability.bodeplotTF(name, loc, fStart, fEnd, fPoints);
                        stability.nyquistplotTF(name, loc, fStart, std::min(fEnd, 2000.0),
                            std::min(fPoints, 400));
                        wrote = true;
                    }
                    catch (const std::exception& ex) {
                        std::cout << "[DQsym] Impedance plots at " << loc << " for '"
                            << name << "' failed: " << ex.what() << "\n";
                    }
                }
                return wrote;
            };
            if (!trySide("AC"))
                trySide("DC");
        }
    }
    catch (const std::exception& ex) {
        std::cout << "[DQsym] StabilityEstimate failed: " << ex.what() << "\n";
    }
}

DQsymResult mergeDQsymResults(const DQsymResult& prefix, int kKeep, const DQsymResult& extra)
{
    DQsymResult out = prefix;
    const int n0 = std::max(0, kKeep + 1);
    const int n1 = static_cast<int>(extra.time.size());
    const int n = n0 + n1;

    out.time.resize(n);
    for (int i = 0; i < n0 && i < static_cast<int>(prefix.time.size()); ++i)
        out.time[i] = prefix.time[i];
    for (int i = 0; i < n1; ++i)
        out.time[n0 + i] = extra.time[i];

    const int nbrk = std::max(prefix.brkHistory.cols(), extra.brkHistory.cols());
    out.brkHistory = Eigen::MatrixXi::Zero(n, nbrk);
    if (prefix.brkHistory.size() > 0)
        out.brkHistory.block(0, 0, std::min(n0, static_cast<int>(prefix.brkHistory.rows())),
            std::min(nbrk, static_cast<int>(prefix.brkHistory.cols()))) =
            prefix.brkHistory.topLeftCorner(
                std::min(n0, static_cast<int>(prefix.brkHistory.rows())),
                std::min(nbrk, static_cast<int>(prefix.brkHistory.cols())));
    if (extra.brkHistory.size() > 0)
        out.brkHistory.block(n0, 0, std::min(n1, static_cast<int>(extra.brkHistory.rows())),
            std::min(nbrk, static_cast<int>(extra.brkHistory.cols()))) =
            extra.brkHistory.topLeftCorner(
                std::min(n1, static_cast<int>(extra.brkHistory.rows())),
                std::min(nbrk, static_cast<int>(extra.brkHistory.cols())));

    out.DSSabcHist.resize(prefix.DSSabcHist.size());
    for (size_t g = 0; g < prefix.DSSabcHist.size(); ++g) {
        out.DSSabcHist[g] = Eigen::MatrixXd::Zero(n, 3);
        const int r0 = std::min(n0, static_cast<int>(prefix.DSSabcHist[g].rows()));
        if (r0 > 0)
            out.DSSabcHist[g].topRows(r0) = prefix.DSSabcHist[g].topRows(r0);
        if (g < extra.DSSabcHist.size()) {
            const int r1 = std::min(n1, static_cast<int>(extra.DSSabcHist[g].rows()));
            if (r1 > 0)
                out.DSSabcHist[g].block(n0, 0, r1, 3) = extra.DSSabcHist[g].topRows(r1);
        }
    }

    for (auto& [name, X] : prefix.stateHist) {
        Eigen::MatrixXd M = Eigen::MatrixXd::Zero(X.rows(), n);
        const int c0 = std::min(n0, static_cast<int>(X.cols()));
        if (c0 > 0)
            M.leftCols(c0) = X.leftCols(c0);
        if (extra.stateHist.count(name)) {
            const auto& Xe = extra.stateHist.at(name);
            const int c1 = std::min(n1, static_cast<int>(Xe.cols()));
            if (c1 > 0 && Xe.rows() == M.rows())
                M.block(0, n0, Xe.rows(), c1) = Xe.leftCols(c1);
        }
        out.stateHist[name] = std::move(M);
    }
    for (auto& [name, U] : prefix.inputHist) {
        Eigen::MatrixXd M = Eigen::MatrixXd::Zero(U.rows(), n);
        const int c0 = std::min(n0, static_cast<int>(U.cols()));
        if (c0 > 0)
            M.leftCols(c0) = U.leftCols(c0);
        if (extra.inputHist.count(name)) {
            const auto& Ue = extra.inputHist.at(name);
            const int c1 = std::min(n1, static_cast<int>(Ue.cols()));
            if (c1 > 0 && Ue.rows() == M.rows())
                M.block(0, n0, Ue.rows(), c1) = Ue.leftCols(c1);
        }
        out.inputHist[name] = std::move(M);
    }

    out.xHist.resize(n);
    for (int i = 0; i < n0 && i < static_cast<int>(prefix.xHist.size()); ++i)
        out.xHist[i] = prefix.xHist[i];
    for (int i = 0; i < n1 && i < static_cast<int>(extra.xHist.size()); ++i)
        out.xHist[n0 + i] = extra.xHist[i];

    out.cfg = extra.cfg;
    out.cfg.resumeX = MatrixXcd();
    return out;
}

} // namespace


void DQsym::initialize(Network* net)
{
    net_ = net;
    ac_grid_names.clear();
    dc_grid_names.clear();
    ac_grids.clear();
    dc_grids.clear();
    converters.clear();

	net->is_area_empty() ? net->add_areas() : void();

	ac_grids = net->get_ac_grids();
	dc_grids = net->get_dc_grids();
	ac_grid_names = net->get_ac_grid_names();
	dc_grid_names = net->get_dc_grid_names();
	converters = net->get_converters();
}

/**
 * @brief Solves the discrete-time state-space system with switch-dependent matrices.
 *
 * This function computes the output of a discrete-time state-space system for a given
 * sequence of inputs. It supports dynamic changes in the system matrices based on the
 * state of switches (breakers). The state-space model is updated whenever the switch
 * configuration changes.
 *
 * The core calculation is performed in the phasor domain. The state vector is rotated
 * at each time step to account for the system's fundamental frequency, and the final
 * output is computed based on the updated state.
 *
 * @note This implementation does not include a half-step predictor/corrector.
 *
 * @param Ad The discrete-time state matrix.
 * @param Bd The discrete-time input matrix.
 * @param Cd The discrete-time output matrix.
 * @param Dd The discrete-time feed-through matrix.
 * @param swOnRes A vector of ON-resistances for the switches.
 * @param swOffRes A vector of OFF-resistances for the switches.
 * @param swType A vector indicating the type of each switch.
 * @param brkVec A vector representing the current state of the breakers (switches).
 * @param u The input matrix over the simulation time.
 * @param xo The initial state vector.
 * @param dt The time step for the simulation.
 * @param f0 The fundamental frequency of the system.
 * @return A matrix representing the system's output over the simulation time.
 */
 // ===================================================================
 //  DSSS — operates directly on the supplied DSSState
 // ===================================================================

MatrixXcd DQsym::DSSS(
    DSSState& st,
    const MatrixXcd& Ad, const MatrixXcd& Bd,
    const MatrixXcd& Cd, const MatrixXcd& Dd,
    const VectorXd& swOnRes, const VectorXd& swOffRes,
    const VectorXi& swType, const VectorXi& brkVec,
    const MatrixXcd& u, const VectorXcd& xo,
    double dt, double f0)
{
    int T = u.cols();
    int nx = Ad.rows();
    int ny = Cd.rows();

    MatrixXcd x = MatrixXcd::Zero(nx, T);
    MatrixXcd y = MatrixXcd::Zero(ny, T);

    MatrixXcd A0, B0, C0, D0;
    convertToPhasor(Ad, Bd, Cd, Dd, A0, B0, C0, D0);

    if (!st.initialized)
    {
        st.nStates = nx;
        st.nInputs = Bd.cols();
        st.nOutputs = ny;
        st.nSwitches = (swType.array() == 1).count();

        st.x_old = MatrixXcd::Zero(nx, T);
        st.x_old.col(0) = xo;

        st.swVec = brkVec;
        st.swVecOld = st.swVec;
        st.yswitch = VectorXcd::Zero(st.nSwitches);

        buildMatricesForState(A0, B0, C0, D0,
            st.swVec, swType, swOnRes, swOffRes,
            st.Ads, st.Bds, st.Cds, st.Dds);
        st.initialized = true;
    }

    if ((swType.array() != 0).any())
        st.swVec = brkVec;

    if ((swType.array() != 0).any() &&
        (st.swVecOld.array() != st.swVec.array()).any())
    {
        buildMatricesForState(A0, B0, C0, D0,
            st.swVec, swType, swOnRes, swOffRes,
            st.Ads, st.Bds, st.Cds, st.Dds);
        st.swVecOld = st.swVec;
    }

    x = st.Ads * st.x_old + st.Bds * u;

    VectorXcd expVec(T);
    for (int k = 0; k < T; ++k)
        expVec(k) = std::exp(std::complex<double>(0.0, -2.0 * M_PI * f0 * dt * k));

    MatrixXcd x2 = x * expVec.asDiagonal();
    st.x_old = x2;

    y = st.Cds * x2 + st.Dds * u;
    return y;
}


// ===================================================================
//  buildMatricesForState
// ===================================================================

void DQsym::buildMatricesForState(
    const MatrixXcd& A0, const MatrixXcd& B0,
    const MatrixXcd& C0, const MatrixXcd& D0,
    const VectorXi& swVec_in, const VectorXi& swType,
    const VectorXd& swOnRes, const VectorXd& swOffRes,
    MatrixXcd& Ao, MatrixXcd& Bo, MatrixXcd& Co, MatrixXcd& Do)
{
    Ao = A0;  Bo = B0;  Co = C0;  Do = D0;

    if (!(swType.array() != 0).any() || (swVec_in.array() == 0).all())
        return;

    int ns = A0.rows();
    int no = C0.rows();
    int nsw = (swType.array() == 1).count();

    VectorXcd ysw(nsw);
    for (int s = 0; s < nsw; ++s)
        ysw(s) = (swVec_in(s) == 1)
        ? std::complex<double>(1.0 / swOnRes(s), 0.0)
        : std::complex<double>(1.0 / swOffRes(s), 0.0);

    VectorXcd DxCol(no);
    VectorXcd BDcol(ns);

    for (int s = 0; s < nsw; ++s)
    {
        int kSw = swVec_in(s);
        if (kSw == 0) continue;

        std::complex<double> tmp = Do(s, s) * ysw(s);
        std::complex<double> temp = 1.0 / (1.0 - tmp * static_cast<double>(kSw));
        std::complex<double> t2 = ysw(s) * temp * static_cast<double>(kSw);

        for (int i = 0; i < no; ++i) DxCol(i) = Do(i, s) * t2;
        DxCol(s) = temp;

        for (int i = 0; i < ns; ++i)
            BDcol(i) = Bo(i, s) * ysw(s) * static_cast<double>(kSw);

        RowVectorXcd rowC = Co.row(s);
        RowVectorXcd rowD = Do.row(s);
        Co.row(s).setZero();
        Do.row(s).setZero();

        for (int i = 0; i < no; ++i) {
            Co.row(i) += DxCol(i) * rowC;
            Do.row(i) += DxCol(i) * rowD;
        }
        for (int i = 0; i < ns; ++i) {
            Ao.row(i) += BDcol(i) * Co.row(s);
            Bo.row(i) += BDcol(i) * Do.row(s);
        }
    }
}


// ===================================================================
//  run — assembles global state-space, discretizes, DSSS loop
// ===================================================================

DQsymResult DQsym::run(Config& cfg)
{
    if (!net_)
        throw std::runtime_error("DQsym::run(): call initialize(Network*) first.");

    // ---- Step 0: state-space with DQsym mode (B columns in groups of 3) ----
    StateSpaceModel ssm;
    ssm.formState(net_, cfg.outputBuses, SSMMode::DQsym);

	cout << "[DQsym] State-space model formed with DQsym mode.\n";

    int nx = ssm.getA().rows();
    int nu = ssm.getB().cols();   // B_dqsym columns (all groups of 3)

    //cout << "State-space model formed with Standard mode:\n"
    //    << "A: " << ssm.getA() << "\n"
    //    << "B: " << ssm.getB() << "\n"
    //    << "C: " << ssm.getC() << "\n"
    //    << "D: " << ssm.getD() << "\n";

    // ---- Step 1: discretize ----
    MatrixXd Cd_id = Eigen::MatrixXd::Identity(nx, nx);
    MatrixXd Dd_z = Eigen::MatrixXd::Zero(nx, nu);
    MatrixXd Ad_r, Bd_r;

    std::ofstream file("state_space_outputcont.txt");
    file << "State-space model formed:\n\n"
        << "Ad_r (" << ssm.getA().rows() << "x" << ssm.getA().cols() << "):\n\n"
        << ssm.getA() << "\n\n"
        << "Bd_r (" << ssm.getB().rows() << "x" << ssm.getB().cols() << "):\n\n"
        << ssm.getB() << "\n\n"
        << "Cd_id (" << Cd_id.rows() << "x" << Cd_id.cols() << "):\n\n"
        << Cd_id << "\n\n"
        << "Dd_z (" << Dd_z.rows() << "x" << Dd_z.cols() << "):\n\n"
        << Dd_z << "\n\n";

    file.close();

    discretizeABCD(ssm.getA(), ssm.getB(), Cd_id, Dd_z,
        cfg.dt, Ad_r, Bd_r, Cd_id, Dd_z);

   std::ofstream file1("state_space_output1.txt");
    file1 << "State-space model formed:\n\n"
        << "Ad_r (" << Ad_r.rows() << "x" << Ad_r.cols() << "):\n\n"
        << Ad_r << "\n\n"
        << "Bd_r (" << Bd_r.rows() << "x" << Bd_r.cols() << "):\n\n"
        << Bd_r << "\n\n"
        << "Cd_id (" << Cd_id.rows() << "x" << Cd_id.cols() << "):\n\n"
        << Cd_id << "\n\n"
        << "Dd_z (" << Dd_z.rows() << "x" << Dd_z.cols() << "):\n\n"
        << Dd_z << "\n\n";

    file1.close();

    MatrixXcd AdC = Ad_r.cast<std::complex<double>>();
    MatrixXcd BdC = Bd_r.cast<std::complex<double>>();
    MatrixXcd CdC = Cd_id.cast<std::complex<double>>();
    MatrixXcd DdC = Dd_z.cast<std::complex<double>>();

    


    int ny = CdC.rows();
    const int nGroups = (ny + 2) / 3;   // ceil(ny/3); pad below if ny % 3 != 0
    const int padRows = nGroups * 3 - ny;
    VectorXcd xo = VectorXcd::Zero(nx);

    // ---- Step 2: allocate ----
    const int N = static_cast<int>((cfg.t_end - cfg.t_start) / cfg.dt) + 1;

    DQsymResult result;
    result.time.resize(N);
    result.DSSabcHist.assign(nGroups, Eigen::MatrixXd::Zero(N, 3));
    result.brkHistory = Eigen::MatrixXi::Zero(N, cfg.swType.size());
    result.xHist.assign(N, MatrixXcd());
    result.cfg = cfg;
    result.cfg.resumeX = MatrixXcd();

    dssState_ = DSSState{};

    // Per-element states for feedback (initialized to zero)
    std::map<std::string, std::vector<MatrixXcd>> elementStates;
    for (const auto& [name, elem] : converters) {
        int nStates = elem->getNumberOfPlantStates();
        if (nStates <= 0) continue;
        int nStateGroups = nStates / 3;
        elementStates[name] = std::vector<MatrixXcd>(
            nStateGroups, MatrixXcd::Zero(3, cfg.nKeep));
        if (dynamic_cast<MMC*>(elem)) {
            const int nFull = elem->getNumberOfInternalStates();
            result.stateHist[name] = Eigen::MatrixXd::Zero(nFull, N);
            result.inputHist[name] = Eigen::MatrixXd::Zero(3, N);
        }
    }

    const bool resume = cfg.resumeX.rows() == nx && cfg.resumeX.cols() == cfg.nKeep;
    if (resume) {
        MatrixXcd A0, B0, C0, D0;
        convertToPhasor(AdC, BdC, CdC, DdC, A0, B0, C0, D0);
        dssState_.nStates = nx;
        dssState_.nInputs = BdC.cols();
        dssState_.nOutputs = ny;
        dssState_.nSwitches = static_cast<int>((cfg.swType.array() == 1).count());
        dssState_.x_old = cfg.resumeX;
        Eigen::VectorXi brk0 = cfg.breakerFunction
            ? cfg.breakerFunction(0, cfg.t_start)
            : Eigen::VectorXi::Zero(cfg.swType.size());
        dssState_.swVec = brk0;
        dssState_.swVecOld = brk0;
        dssState_.yswitch = VectorXcd::Zero(dssState_.nSwitches);
        buildMatricesForState(A0, B0, C0, D0, dssState_.swVec, cfg.swType,
            cfg.swOnRes, cfg.swOffRes,
            dssState_.Ads, dssState_.Bds, dssState_.Cds, dssState_.Dds);
        dssState_.initialized = true;

        for (const auto& [name, elem] : converters) {
            int nStates = elem->getNumberOfPlantStates();
            if (nStates <= 0) continue;
            int startRow = ssm.getStateIndex(name, 0);
            if (startRow < 0) continue;
            int nStateGroups = nStates / 3;
            std::vector<MatrixXcd> groups(nStateGroups);
            for (int g = 0; g < nStateGroups; ++g)
                groups[g] = cfg.resumeX.block(startRow + 3 * g, 0, 3, cfg.nKeep);
            elementStates[name] = groups;
        }
        cout << "[DQsym] Resuming from stored phasor state ("
            << nx << " x " << cfg.nKeep << ").\n";
    }

    // ---- Step 3: main loop ----
    for (int k = 0; k < N; ++k)
    {
        double t = cfg.t_start + k * cfg.dt;
        double theta = 2.0 * M_PI * cfg.f * t;
        result.time[k] = t;

        // 3a. Breaker
        Eigen::VectorXi brkVec = cfg.breakerFunction
            ? cfg.breakerFunction(k, t)
            : Eigen::VectorXi::Zero(cfg.swType.size());
        result.brkHistory.row(k) = brkVec.transpose();

        //add18/5[

        

        // 3b. Build u (nu × nKeep) — sources + MMC feedback from previous step
        MatrixXcd u = ssm.buildInputVector(cfg.nKeep, elementStates);
		//cout << "Input vector u at step " << k << ":\n" << u << "\n";


        std::map<std::string, Eigen::Vector2d> vg_dq_step;

        for (const auto& [name, elem] : converters) {
            MMC* mmc = dynamic_cast<MMC*>(elem);
            if (!mmc) continue;

            // Find the AC bus this MMC connects to (terminal 1)
            Bus* ac_bus = nullptr;
            for (auto& [bus, terminal] : mmc->getConnections()) {
                if (terminal == 1) { ac_bus = bus; break; }
            }

            // Find the AC source connected to the same AC bus and read its voltage from u
            Eigen::Vector2d Vg_dq(0.0, 0.0);
            if (ac_bus) {
                for (const auto& g : ssm.getInputGroups()) {
                    if (g.isVirtual) continue;
                    // Check if this source connects to our AC bus
                    bool found = false;
                    for (auto& [bus, terminal] : g.element->getConnections()) {
                        if (bus == ac_bus) { found = true; break; }
                    }
                    if (!found) continue;

                    // Read positive-sequence fundamental: row dqsymStartCol+0, col 1
                    if (g.dqsymStartCol < u.rows() && u.cols() >= 2) {
                        std::complex<double> v_fund = u(g.dqsymStartCol, 1);
                        Vg_dq(0) = v_fund.real();
                        Vg_dq(1) = v_fund.imag();
                    }
                    break;
                }
            }
            vg_dq_step[name] = Vg_dq;

            if (elementStates.count(name)) {
                mmc->stepControllers(cfg.dt, elementStates.at(name), Vg_dq);
            }
        }
        // === END DQsym closed-loop control: step controllers ===

        //add18/5]
        

        // 3c. DSSS
        MatrixXcd y = DSSS(dssState_, AdC, BdC, CdC, DdC,
            cfg.swOnRes, cfg.swOffRes, cfg.swType, brkVec,
            u, xo, cfg.dt, cfg.f);
        result.xHist[k] = y;


        if (k == 6000) {

            std::ofstream file("state_space_output2.txt");

            file << "State-space model formed with Standard mode:\n\n"

                << "A (" << AdC.rows() << "x" << AdC.cols() << "):\n"
                << AdC << "\n\n"

                << "B (" << BdC.rows() << "x" << BdC.cols() << "):\n"
                << BdC << "\n\n"

                << "C (" << CdC.rows() << "x" << CdC.cols() << "):\n"
                << CdC << "\n\n"

                << "D (" << DdC.rows() << "x" << DdC.cols() << "):\n"
                << DdC << "\n\n"

                << "u (" << u.rows() << "x" << u.cols() << "):\n"
                << u << "\n";

            file.close();
        }
		//cout << y << "\n";

        // 3d. Extract state groups, update elementStates for next step
        for (const auto& [name, elem] : converters) {
            int nStates = elem->getNumberOfPlantStates();
            if (nStates <= 0) continue;

            int startRow = ssm.getStateIndex(name, 0);
            if (startRow < 0) continue;

            int nStateGroups = nStates / 3;
            std::vector<MatrixXcd> groups(nStateGroups);
            for (int g = 0; g < nStateGroups; ++g)
                groups[g] = y.block(startRow + 3 * g, 0, 3, cfg.nKeep);

            elementStates[name] = groups;
        }

        for (const auto& [name, elem] : converters) {
            MMC* mmc = dynamic_cast<MMC*>(elem);
            if (!mmc || !elementStates.count(name) || !result.stateHist.count(name))
                continue;

            const int nFull = mmc->getNumberOfInternalStates();
            Eigen::VectorXd x = Eigen::VectorXd::Zero(nFull);
            const int n_ctrl = nFull - 12;
            if (n_ctrl > 0 && mmc->x_ctrl_dqsym_.size() == n_ctrl)
                x.head(n_ctrl) = mmc->x_ctrl_dqsym_;
            mmc->fillPlantFromHarmonics(x, elementStates[name]);
            result.stateHist[name].col(k) = x;

            const auto& groups = elementStates[name];
            double Vdc_meas = 0.0;
            if (groups.size() >= 4 && groups[3].rows() > 2 && groups[3].cols() > 0)
                Vdc_meas = 2.0 * groups[3](2, 0).real();
            const Eigen::Vector2d Vg = vg_dq_step.count(name)
                ? vg_dq_step[name] : Eigen::Vector2d::Zero();
            result.inputHist[name].col(k) << Vdc_meas, Vg(0), Vg(1);
        }

        // 3e. ABC reconstruction (pad state rows to a multiple of 3 when needed)
        MatrixXcd yPlot = y;
        if (padRows > 0) {
            MatrixXcd pad = MatrixXcd::Zero(padRows, y.cols());
            yPlot.resize(ny + padRows, y.cols());
            yPlot.topRows(ny) = y;
            yPlot.bottomRows(padRows).setZero();
        }
        auto abcGroups = dqn2abc_groups_at_time(yPlot, theta);
        for (int g = 0; g < nGroups && g < (int)abcGroups.size(); ++g)
            result.DSSabcHist[g].row(k) = abcGroups[g].transpose();
    }

    cout << "Simulation completed with " << N << " steps and "
		<< nGroups << " groups.\n";

    result_ = result;
    hasRun_ = true;
    return result;
}



// ===================================================================
//  Results
// ===================================================================

//void DQsym::exportCSV(const std::string& filename) const
//{
//    if (!hasRun_)
//        throw std::runtime_error("exportCSV() before run().");
//
//    std::vector<Eigen::MatrixXd> values = {};
//    values.push_back(result_.brkHistory.cast<double>());
//    for (const auto& m : result_.DSSabcHist) {
//        values.push_back(m);
//		cout << "DSSabcHist group with shape (" << m.rows() << "x" << m.cols() << ")\n";
//    }
//
//    std::vector<std::string> headers;
//    headers.push_back("brk");
//    for (int g = 0; g < static_cast<int>(result_.DSSabcHist.size()); ++g)
//        headers.push_back("state_abc" + std::to_string(g + 1));
//
//	cout << "Exporting CSV with " << values.size() << " matrices and headers: ";
//
//    write_file(result_.time, values, headers, filename);
//}
void DQsym::exportCSV(const std::string& filename) const
{
    if (!hasRun_)
        throw std::runtime_error("exportCSV() before run().");

    std::vector<Eigen::MatrixXd> values = {};
    for (const auto& m : result_.DSSabcHist)
        values.push_back(m);

    std::vector<std::string> headers;
    for (int g = 0; g < static_cast<int>(result_.DSSabcHist.size()); ++g)
        headers.push_back("state_abc" + std::to_string(g + 1));

    write_file(result_.time, values, headers, filename);
    cout << "CSV file saved" << "\n";
}

void DQsym::plot() const
{
    if (!hasRun_)
        throw std::runtime_error("plot() before run().");

    auto result = std::make_shared<DQsymResult>(result_);
    Network* net = net_;
    auto live = (net && net->liveFlag()) ? net->liveFlag()
        : std::make_shared<std::atomic<bool>>(false);
    const std::string abcTitle = "State-space outputs (abc)";

    auto analyze = [result, net, live](double t_sel)
    {
        if (!live || !*live || !net)
            throw std::runtime_error("network is no longer live; re-run the study to analyze a snapshot.");
        if (!result || result->time.empty())
            return;
        analyzeSnapshot(net, *result, t_sel, true);
    };

    auto cont = std::make_shared<std::function<void(double, double)>>();
    *cont = [result, net, live, abcTitle, analyze, cont](double t_sel, double extra_t)
    {
        if (!live || !*live || !net)
            throw std::runtime_error("network is no longer live; re-run the study to continue.");
        if (!result || result->time.empty() || extra_t <= 0.0)
            return;
        if (static_cast<int>(result->xHist.size()) != static_cast<int>(result->time.size()))
            throw std::runtime_error("no stored phasor history to resume from (re-run DQsym).");

        const int k = nearestTimeIndex(result->time, t_sel);
        if (k >= static_cast<int>(result->xHist.size()) || result->xHist[k].size() == 0)
            throw std::runtime_error("snapshot at t* has no phasor state.");

        Config cfg = result->cfg;
        cfg.dt = (cfg.dt > 0.0) ? cfg.dt : 2e-5;
        cfg.t_start = result->time[k] + cfg.dt;
        cfg.t_end = result->time[k] + extra_t;
        if (cfg.t_end <= cfg.t_start)
            cfg.t_end = cfg.t_start + cfg.dt;
        cfg.resumeX = result->xHist[k];

        restoreMmcControllers(net, *result, k);

        DQsym dq;
        dq.initialize(net);
        DQsymResult more = dq.run(cfg);
        *result = mergeDQsymResults(*result, k, more);

        plot_abc_groups_implot(result->time, result->DSSabcHist, abcTitle, analyze, *cont);
        std::cout << "[DQsym] Continued from t = " << result->time[k]
            << " s; now ends at t = " << result->time.back() << " s\n";
    };

    plot_abc_groups_implot(result_.time, result_.DSSabcHist, abcTitle, analyze, *cont);
}

void DQsym::analyzeAtTime(double t, bool plotResults, double fStart, double fEnd, int fPoints) const
{
    if (!hasRun_)
        throw std::runtime_error("analyzeAtTime() before run().");
    if (!net_)
        throw std::runtime_error("analyzeAtTime: network not initialized.");
    analyzeSnapshot(net_, result_, t, plotResults, fStart, fEnd, fPoints);
}

void DQsym::setResult(DQsymResult result)
{
    result_ = std::move(result);
    hasRun_ = true;
}

const DQsymResult& DQsym::getResult() const
{
    if (!hasRun_)
        throw std::runtime_error("getResult() before run().");
    return result_;
}
