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

    for (const auto& [name, c] : converters)
    {
        Converter* conv_elem = dynamic_cast<Converter*>(c);
        conv_elem->computeABCD();
	}
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
MatrixXcd DQsym::DSSS(const MatrixXcd& Ad, const MatrixXcd& Bd,
    const MatrixXcd& Cd, const MatrixXcd& Dd,
    const VectorXd& swOnRes, const VectorXd& swOffRes,
    const VectorXi& swType, const VectorXi& brkVec,
    const MatrixXcd& u, const VectorXcd& xo, double dt, double f0)
{
    int T = u.cols();
    int nx = Ad.rows();
    int ny = Cd.rows();
    int nu = Bd.cols();


    MatrixXcd x = MatrixXcd::Zero(nx, T);
    MatrixXcd y = MatrixXcd::Zero(ny, T);

    MatrixXcd A0, B0, C0, D0;
    convertToPhasor(Ad, Bd, Cd, Dd, A0, B0, C0, D0);

    MatrixXcd Ads, Bds, Cds, Dds;

    if (!initialized)
    {
        nStates = nx;
        nInputs = nu;
        nOutputs = ny;
        nSwitches = (swType.array() == 1).count();

        x_old = MatrixXcd::Zero(nx, T);
        x_old.col(0) = xo;

        swVec = brkVec;
        swVecOld = swVec;

        yswitch = VectorXcd::Zero(nSwitches);

        buildMatricesForState(A0, B0, C0, D0, swVec, swType, swOnRes, swOffRes, Ads, Bds, Cds, Dds);

        initialized = true;
    }


    if ((swType.array() != 0).any())
        swVec = brkVec;


    if ((swType.array() != 0).any() && (swVecOld.array() != swVec.array()).any())
    {
        buildMatricesForState(A0, B0, C0, D0, swVec, swType, swOnRes, swOffRes, Ads, Bds, Cds, Dds);
        swVecOld = swVec;
    }


    x = Ads * x_old + Bds * u;


    VectorXcd expVec(T);
    for (int k = 0; k < T; ++k)
        expVec(k) = std::exp(std::complex<double>(0.0, -2.0 * M_PI * f0 * dt * k));

    MatrixXcd rotMat = expVec.asDiagonal();
    MatrixXcd x2 = x * rotMat;

    x_old = x2;
    y = Cds * x2 + Dds * u;

    return y;
}

/**
 * @brief Build matrices for a given switch state.
 */
void DQsym::buildMatricesForState(const MatrixXcd& A0, const MatrixXcd& B0, const MatrixXcd& C0, const MatrixXcd& D0,
    const VectorXi& swVec, const VectorXi& swType, const VectorXd& swOnRes, const VectorXd& swOffRes, MatrixXcd& Ao, MatrixXcd& Bo,  MatrixXcd& Co, MatrixXcd& Do)
{

    Ao = A0;
    Bo = B0;
    Co = C0;
    Do = D0;


    if (!(swType.array() != 0).any() || (swVec.array() == 0).all())
        return;

    int nStates_local = A0.rows();
    int nOutputs_local = C0.rows();
    int nSwitches_local = (swType.array() == 1).count();

    VectorXcd yswitch_local(nSwitches_local);
    for (int s = 0; s < nSwitches_local; ++s)
    {
        if (swVec(s) == 1)
            yswitch_local(s) = std::complex<double>(1.0 / swOnRes(s), 0.0);
        else
            yswitch_local(s) = std::complex<double>(1.0 / swOffRes(s), 0.0);
    }

    VectorXcd DxCol(nOutputs_local);
    VectorXcd BDcol(nStates_local);

    for (int s = 0; s < nSwitches_local; ++s)
    {
        int kSw = swVec(s);
        if (kSw == 0)
            continue;

        std::complex<double> tmp = Do(s, s) * yswitch_local(s);
        std::complex<double> temp = 1.0 / (1.0 - tmp * static_cast<double>(kSw));
        std::complex<double> t2 = yswitch_local(s) * temp * static_cast<double>(kSw);

        for (int i = 0; i < nOutputs_local; ++i)
            DxCol(i) = Do(i, s) * t2;
        DxCol(s) = temp;

        for (int i = 0; i < nStates_local; ++i)
            BDcol(i) = Bo(i, s) * yswitch_local(s) * static_cast<double>(kSw);   
      
        RowVectorXcd rowC = Co.row(s);
        RowVectorXcd rowD = Do.row(s);
        Co.row(s).setZero();
        Do.row(s).setZero();

        for (int i = 0; i < nOutputs_local; ++i)
        {
            Co.row(i) += DxCol(i) * rowC;
            Do.row(i) += DxCol(i) * rowD;
        }

        for (int i = 0; i < nStates_local; ++i)
        {
            Ao.row(i) += BDcol(i) * Co.row(s);
            Bo.row(i) += BDcol(i) * Do.row(s);
        }
    }
}




void DQsym::exportCSV(const std::string& filename) const
{
    if (!hasRun_) {
        throw std::runtime_error("exportCSV() called before run().");
    }

    std::vector<Eigen::MatrixXd> values;
    values.push_back(result_.brkHistory.cast<double>());

    for (const auto& x : result_.DSSabcHist) {
        values.push_back(x);
    }

    int n = result_.MMCabcHist.size();
    for (int i = 0; i < n; ++i) {
        values.push_back(result_.MMCabcHist[i]);
    }

    std::vector<std::string> headers;
    headers.push_back("brk");

    for (int g = 0; g < static_cast<int>(result_.DSSabcHist.size()); ++g) {
        headers.push_back("DSS_abc" + std::to_string(g + 1));
    }

    headers.push_back("Uup_abc");
    headers.push_back("Ulow_abc");
    headers.push_back("Iup_abc");
    headers.push_back("Ilow_abc");
    headers.push_back("VcUp_abc");
    headers.push_back("VcLow_abc");
    headers.push_back("VoutUp_abc");
    headers.push_back("VoutLow_abc");

    write_file(result_.time, values, headers, filename);
}

void DQsym::plot() const
{
    if (!hasRun_) {
        throw std::runtime_error("plot() called before run().");
    }

    plot_abc_groups_implot(
        result_.time,
        result_.DSSabcHist,
        "DSS outputs converted to abc"
    );

    std::vector<Eigen::MatrixXd> mmcWaveforms;

    int n = result_.MMCabcHist.size();
    for (int i = 0; i < n; ++i) {
        mmcWaveforms.push_back(result_.MMCabcHist[i]);
    }

    plot_abc_groups_implot(
        result_.time,
        mmcWaveforms,
        "MMC internal abc waveforms"
    );
}

const DQsymResult& DQsym::getResult() const
{
    if (!hasRun_) {
        throw std::runtime_error("getResult() called before run().");
    }

    return result_;
}

DQsymResult DQsym::run(Config& cfg, MatrixXd Ad, MatrixXd Bd, MatrixXd Cd, MatrixXd Dd)
{
    const int N = static_cast<int>((cfg.t_end - cfg.t_start) / cfg.dt) + 1;

    DQsymResult result;
    result.time.resize(N);

    //const int nGroups = static_cast<int>(Cd.rows() / 3);

    //result.DSSabcHist.assign(nGroups, Eigen::MatrixXd::Zero(N, 3));
    //result.brkHistory = Eigen::MatrixXi::Zero(N, cfg.swType.size());

    //// -------------------------
    //// CONVERTER OUTPUT STATE ACCESS
    //// -------------------------
    //std::unordered_map<std::string, std::vector<MatrixXcd>> lastOutputs;

    //MatrixXcd VoutUpMMC = MatrixXcd::Zero(3, cfg.nKeep);
    //MatrixXcd VoutLowMMC = MatrixXcd::Zero(3, cfg.nKeep);

    //// -------------------------
    //// MAIN LOOP
    //// -------------------------
    //for (int k = 0; k < N; ++k)
    //{
    //    double t = cfg.t_start + k * cfg.dt;
    //    double theta = 2.0 * M_PI * cfg.f * t;

    //    result.time[k] = t;

    //    // -------------------------
    //    // breaker
    //    // -------------------------
    //    Eigen::VectorXi brkVec =
    //        cfg.breakerFunction ?
    //        cfg.breakerFunction(k, t) :
    //        VectorXi::Zero(cfg.swType.size());

    //    result.brkHistory.row(k) = brkVec.transpose();

    //    // -------------------------
    //    // external inputs
    //    // -------------------------
    //    std::vector<MatrixXcd> uBlocks =
    //        cfg.externalInputFunction ?
    //        cfg.externalInputFunction(k, t) :
    //        std::vector<MatrixXcd>(cfg.nInputBlocks,
    //            MatrixXcd::Zero(3, cfg.nKeep));

    //    // -------------------------
    //    // FEEDBACK ROUTING (NEW SYSTEM)
    //    // -------------------------
    //    for (const auto& route : cfg.feedbackRoutes)
    //    {
    //        auto it = converters.find(route.sourceConverter);
    //        if (it == converters.end() || !it->second)
    //            continue;

    //        const auto& outputs = it->second->getBuses();

    //        if (route.sourceSignalIndex < 0 ||
    //            route.sourceSignalIndex >= outputs.size())
    //            continue;

    //        MatrixXcd signal = outputs[route.sourceSignalIndex];

    //        if (route.invert)
    //            signal = -signal;

    //        if (route.targetBlockIndex >= 0 &&
    //            route.targetBlockIndex < uBlocks.size())
    //        {
    //            uBlocks[route.targetBlockIndex] = signal;
    //        }
    //    }

    //    // -------------------------
    //    // PACK DSSS INPUT
    //    // -------------------------
    //    MatrixXcd u(3 * cfg.nInputBlocks, cfg.nKeep);
    //    for (int b = 0; b < cfg.nInputBlocks; ++b)
    //        u.block(3 * b, 0, 3, cfg.nKeep) = uBlocks[b];

    //    // -------------------------
    //    // DSSS SYSTEM
    //    // -------------------------
    //    MatrixXcd y = DSSS(Ad, Bd, Cd, Dd, cfg.swOnRes, cfg.swOffRes, cfg.swType, brkVec,
    //        u, VectorXcd::Zero(Ad.rows()), cfg.dt, cfg.f
    //    );

    //    MatrixXcd Iup = y.block(3 * cfg.upGroupIndex, 0, 3, cfg.nKeep);
    //    MatrixXcd Ilow = y.block(3 * cfg.lowGroupIndex, 0, 3, cfg.nKeep);

    //    if (cfg.invertUpFeedback)  Iup = -Iup;
    //    if (cfg.invertLowFeedback) Ilow = -Ilow;

    //    // -------------------------
    //    // STORE LAST DSSS OUTPUTS
    //    // -------------------------
    //    MatrixXcd VcUpMMC, VcLowMMC;

    //    // -------------------------
    //    // CONVERTER LOOP (GENERIC)
    //    // -------------------------
    //    for (auto& [name, convPtr] : converters)
    //    {
    //        if (!convPtr)
    //            continue;

    //        std::vector<MatrixXcd> mmcIn = { Ilow, Iup };

    //        auto mmcOut = dynamic_cast<Converter*>(convPtr)->simulateTimeStep(mmcIn, cfg.dt, cfg.nKeep);

    //        lastOutputs[name] = mmcOut;

    //        // optional extraction if needed
    //        if (name == "MMC_UP")
    //        {
    //            VoutUpMMC = mmcOut[0];
    //            VcUpMMC = mmcOut[1];
    //        }
    //        else if (name == "MMC_LOW")
    //        {
    //            VoutLowMMC = mmcOut[0];
    //            VcLowMMC = mmcOut[1];
    //        }
    //    }

    //    // -------------------------
    //    // STORE DSSS HISTORY
    //    // -------------------------
    //    auto abcGroups = dqn2abc_groups_at_time(y, theta);

    //    for (int g = 0; g < nGroups; ++g)
    //        result.DSSabcHist[g].row(k) = abcGroups[g].transpose();

    //    // -------------------------
    //    // UPDATE CONVERTER STATE FEEDBACK CACHE
    //    // -------------------------
    //    lastIup = Iup;
    //    lastIlow = Ilow;
    //}

    result_ = result;
    return result;
}







