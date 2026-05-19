#include "DQsym.h"
#include "../../Constants.h"

#include "../../network.h"      // For access to the Network class and its members
#include "../../Include_components.h"


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
    discretizeABCD(ssm.getA(), ssm.getB(), Cd_id, Dd_z,
        cfg.dt, Ad_r, Bd_r, Cd_id, Dd_z);

    MatrixXcd AdC = Ad_r.cast<std::complex<double>>();
    MatrixXcd BdC = Bd_r.cast<std::complex<double>>();
    MatrixXcd CdC = Cd_id.cast<std::complex<double>>();
    MatrixXcd DdC = Dd_z.cast<std::complex<double>>();

    int ny = CdC.rows();
    int nGroups = ny / 3;
    VectorXcd xo = VectorXcd::Zero(nx);

    // ---- Step 2: allocate ----
    const int N = static_cast<int>((cfg.t_end - cfg.t_start) / cfg.dt) + 1;

    DQsymResult result;
    result.time.resize(N);
    result.DSSabcHist.assign(nGroups, Eigen::MatrixXd::Zero(N, 3));
    result.brkHistory = Eigen::MatrixXi::Zero(N, cfg.swType.size());

    dssState_ = DSSState{};

    // Per-element states for feedback (initialized to zero)
    std::map<std::string, std::vector<MatrixXcd>> elementStates;
    for (const auto& [name, elem] : converters) {
        int nStates = elem->getNumberOfPlantStates();
        if (nStates <= 0) continue;
        int nStateGroups = nStates / 3;
        elementStates[name] = std::vector<MatrixXcd>(
            nStateGroups, MatrixXcd::Zero(3, cfg.nKeep));
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

        // === BEGIN DQsym closed-loop control: step controllers ===
        for (const auto& [name, elem] : converters) {
            MMC* mmc = dynamic_cast<MMC*>(elem);
            if (!mmc) continue;

            // Grid voltage feedback. For now, use the AC source's nominal value.
            // (Refine later: extract from bus state if AC source exposes it.)
            Eigen::Vector2d Vg_dq(200, 0.0);  // matches your AC_source amplitude

            if (elementStates.count(name)) {
                mmc->stepControllers(cfg.dt, elementStates.at(name), Vg_dq);
            }
        }
        // === END DQsym closed-loop control: step controllers ===
        
        //add18/5]

        // 3b. Build u (nu × nKeep) — sources + MMC feedback from previous step
        MatrixXcd u = ssm.buildInputVector(cfg.nKeep, elementStates);
		//cout << "Input vector u at step " << k << ":\n" << u << "\n";

        // 3c. DSSS
        MatrixXcd y = DSSS(dssState_, AdC, BdC, CdC, DdC,
            cfg.swOnRes, cfg.swOffRes, cfg.swType, brkVec,
            u, xo, cfg.dt, cfg.f);

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

        // 3e. ABC reconstruction
        auto abcGroups = dqn2abc_groups_at_time(y, theta);
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

    plot_abc_groups_implot(result_.time, result_.DSSabcHist,
        "State-space outputs (abc)");
}

const DQsymResult& DQsym::getResult() const
{
    if (!hasRun_)
        throw std::runtime_error("getResult() before run().");
    return result_;
}
