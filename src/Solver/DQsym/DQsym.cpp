#include "DQsym.h"
#include "../../Constants.h"

#include "../../network.h"      // For access to the Network class and its members
#include "../../Include_components.h"


void DQsym::initialize(Network* net) {
    if (net->is_area_empty())
        net->add_areas();

    ac_grid_names = net->get_ac_grid_names();
    dc_grid_names = net->get_dc_grid_names();
    ac_grids = net->get_ac_grids();
    dc_grids = net->get_dc_grids();
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
//  run
// ===================================================================

DQsymResult DQsym::run(Config& cfg)
{
    // ------------------------------------------------------------------
    //  Step 0: compute and discretize every converter
    // ------------------------------------------------------------------
    for (auto& [name, elem] : converters)
    {
        Converter* conv = dynamic_cast<Converter*>(elem);
        if (!conv) continue;
        conv->computeABCD();
        conv->discretize(cfg.dt);
    }

    // ------------------------------------------------------------------
    //  Step 1: get global system matrices
    //  TODO: replace with assembled network state-space once available.
    //  For now, use the first converter's discrete matrices.
    // ------------------------------------------------------------------
    if (converters.empty())
        throw std::runtime_error("run(): no converters registered.");

    Converter* first = dynamic_cast<Converter*>(converters.begin()->second);
    if (!first)
        throw std::runtime_error("run(): first element is not a Converter.");

    MatrixXcd AdC = first->getAd().cast<std::complex<double>>();
    MatrixXcd BdC = first->getBd().cast<std::complex<double>>();
    MatrixXcd CdC = first->getCd().cast<std::complex<double>>();
    MatrixXcd DdC = first->getDd().cast<std::complex<double>>();

    VectorXcd xo = VectorXcd::Zero(AdC.rows());

    const int nGroups = static_cast<int>(CdC.rows()) / 3;

    // ------------------------------------------------------------------
    //  Step 2: allocate result, reset DSSS state
    // ------------------------------------------------------------------
    const int N = static_cast<int>((cfg.t_end - cfg.t_start) / cfg.dt) + 1;

    DQsymResult result;
    result.time.resize(N);
    result.DSSabcHist.assign(nGroups, Eigen::MatrixXd::Zero(N, 3));
    result.brkHistory = Eigen::MatrixXi::Zero(N, cfg.swType.size());

    dssState_ = DSSState{};     // fresh state

    bool mmcHistAllocated = false;
    int  totalMMCSignals = 0;
    std::unordered_map<std::string, std::vector<MatrixXcd>> lastOutputs;

    // ------------------------------------------------------------------
    //  Step 3: main loop
    // ------------------------------------------------------------------
    for (int k = 0; k < N; ++k)
    {
        const double t = cfg.t_start + k * cfg.dt;
        const double theta = 2.0 * M_PI * cfg.f * t;
        result.time[k] = t;

        // ---- 3a. breaker ----
        Eigen::VectorXi brkVec = cfg.breakerFunction
            ? cfg.breakerFunction(k, t)
            : Eigen::VectorXi::Zero(cfg.swType.size());
        result.brkHistory.row(k) = brkVec.transpose();

        // ---- 3b. global external inputs ----
        std::vector<MatrixXcd> uBlocks = cfg.externalInputFunction
            ? cfg.externalInputFunction(k, t)
            : std::vector<MatrixXcd>(cfg.nInputBlocks,
                MatrixXcd::Zero(3, cfg.nKeep));

        // ---- 3c. per-converter feedback injection ----
        for (const auto& cr : cfg.converterRoutes)
        {
            auto it = lastOutputs.find(cr.name);
            if (it == lastOutputs.end()) continue;
            const auto& outs = it->second;

            for (const auto& fb : cr.feedbacks) {
                if (fb.signalIndex < 0 || fb.signalIndex >= static_cast<int>(outs.size())) continue;
                if (fb.targetBlock < 0 || fb.targetBlock >= static_cast<int>(uBlocks.size())) continue;

                MatrixXcd signal = outs[fb.signalIndex];
                if (fb.invert) signal = -signal;
                uBlocks[fb.targetBlock] = signal;
            }
        }

        // ---- 3d. pack global input ----
        MatrixXcd u(3 * cfg.nInputBlocks, cfg.nKeep);
        for (int b = 0; b < cfg.nInputBlocks; ++b)
            u.block(3 * b, 0, 3, cfg.nKeep) = uBlocks[b];

        // ---- 3e. ONE global DSSS call ----
        MatrixXcd y = DSSS(dssState_, AdC, BdC, CdC, DdC,
            cfg.swOnRes, cfg.swOffRes, cfg.swType, brkVec,
            u, xo, cfg.dt, cfg.f);

        // ---- 3f. per-converter: extract currents → simulateTimeStep ----
        for (const auto& cr : cfg.converterRoutes)
        {
            auto it = converters.find(cr.name);
            if (it == converters.end()) continue;

            Converter* conv = dynamic_cast<Converter*>(it->second);
            if (!conv) continue;

            MatrixXcd Iup = y.block(3 * cr.upGroupIndex, 0, 3, cfg.nKeep);
            MatrixXcd Ilow = y.block(3 * cr.lowGroupIndex, 0, 3, cfg.nKeep);
            if (cr.invertUp)  Iup = -Iup;
            if (cr.invertLow) Ilow = -Ilow;

            auto mmcOut = conv->simulateTimeStep({ Ilow, Iup },
                cfg.dt, cfg.nKeep, cfg.nArm);
            lastOutputs[cr.name] = mmcOut;
        }

        // ---- 3g. allocate MMC history (once) ----
        if (!mmcHistAllocated)
        {
            totalMMCSignals = 0;
            for (const auto& cr : cfg.converterRoutes) {
                auto it = lastOutputs.find(cr.name);
                if (it != lastOutputs.end())
                    totalMMCSignals += static_cast<int>(it->second.size());
            }
            result.MMCabcHist.assign(totalMMCSignals,
                Eigen::MatrixXd::Zero(N, 3));
            mmcHistAllocated = true;
        }

        // ---- 3h. store DSS abc history ----
        auto abcGroups = dqn2abc_groups_at_time(y, theta);
        for (int g = 0; g < nGroups; ++g)
            result.DSSabcHist[g].row(k) = abcGroups[g].transpose();

        // ---- 3i. store MMC abc history ----
        int mmcIdx = 0;
        for (const auto& cr : cfg.converterRoutes) {
            auto it = lastOutputs.find(cr.name);
            if (it == lastOutputs.end()) continue;
            for (const auto& sig : it->second) {
                result.MMCabcHist[mmcIdx].row(k) =
                    dqn2abc_at_time(sig, theta).transpose();
                ++mmcIdx;
            }
        }
    }

    result_ = result;
    hasRun_ = true;
    return result;
}


// ===================================================================
//  Results
// ===================================================================

void DQsym::exportCSV(const std::string& filename) const
{
    if (!hasRun_)
        throw std::runtime_error("exportCSV() called before run().");

    std::vector<Eigen::MatrixXd> values;
    values.push_back(result_.brkHistory.cast<double>());
    for (const auto& m : result_.DSSabcHist) values.push_back(m);
    for (const auto& m : result_.MMCabcHist) values.push_back(m);

    std::vector<std::string> headers;
    headers.push_back("brk");
    for (int g = 0; g < static_cast<int>(result_.DSSabcHist.size()); ++g)
        headers.push_back("DSS_abc" + std::to_string(g + 1));
    for (int g = 0; g < static_cast<int>(result_.MMCabcHist.size()); ++g)
        headers.push_back("MMC_abc" + std::to_string(g + 1));

    write_file(result_.time, values, headers, filename);
}

void DQsym::plot() const
{
    if (!hasRun_)
        throw std::runtime_error("plot() called before run().");

    plot_abc_groups_implot(result_.time, result_.DSSabcHist,
        "DSS outputs converted to abc");

    if (!result_.MMCabcHist.empty())
        plot_abc_groups_implot(result_.time, result_.MMCabcHist,
            "MMC internal abc waveforms");
}

const DQsymResult& DQsym::getResult() const
{
    if (!hasRun_)
        throw std::runtime_error("getResult() called before run().");
    return result_;
}
