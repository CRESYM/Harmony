#include "DQsymrun.h"
#include "DQsym.h"
#include "../Helper_Functions/Helper_Functions.h"
#include "../../Elements/Converter/Simple_MMC.h"

DQsymrun::DQsymrun(
    const Eigen::MatrixXd& Ad_real,
    const Eigen::MatrixXd& Bd_real,
    const Eigen::MatrixXd& Cd_real,
    const Eigen::MatrixXd& Dd_real,
    const Config& cfg
)
    : Ad_real_(Ad_real),
    Bd_real_(Bd_real),
    Cd_real_(Cd_real),
    Dd_real_(Dd_real),
    cfg_(cfg)
{
    validateInputs();
}

void DQsymrun::validateInputs() const
{
    if (Ad_real_.rows() != Ad_real_.cols()) {
        throw std::runtime_error("Ad must be square.");
    }

    if (Bd_real_.rows() != Ad_real_.rows()) {
        throw std::runtime_error("Bd.rows() must match Ad.rows().");
    }

    if (Cd_real_.cols() != Ad_real_.cols()) {
        throw std::runtime_error("Cd.cols() must match Ad.cols().");
    }

    if (Dd_real_.rows() != Cd_real_.rows()) {
        throw std::runtime_error("Dd.rows() must match Cd.rows().");
    }

    if (Dd_real_.cols() != Bd_real_.cols()) {
        throw std::runtime_error("Dd.cols() must match Bd.cols().");
    }

    if (cfg_.Ts <= 0.0) {
        throw std::runtime_error("Ts must be positive.");
    }

    if (cfg_.tEnd < cfg_.t0) {
        throw std::runtime_error("tEnd must be greater than or equal to t0.");
    }

    if (cfg_.C <= 0.0) {
        throw std::runtime_error("C must be positive.");
    }

    if (cfg_.nKeep <= 0) {
        throw std::runtime_error("nKeep must be positive.");
    }

    if (cfg_.nArm <= 0) {
        throw std::runtime_error("nArm must be positive.");
    }

    if (cfg_.nInputBlocks <= 0) {
        throw std::runtime_error("nInputBlocks must be positive.");
    }

    if (Cd_real_.rows() % 3 != 0) {
        throw std::runtime_error("Cd.rows() must be divisible by 3 for grouped abc export.");
    }

    if (Bd_real_.cols() != 3 * cfg_.nInputBlocks) {
        throw std::runtime_error("Bd.cols() must equal 3 * nInputBlocks.");
    }

    if (cfg_.swOnRes.size() == 0 || cfg_.swOffRes.size() == 0 || cfg_.swType.size() == 0) {
        throw std::runtime_error("swOnRes, swOffRes, and swType must be provided in cfg.");
    }

    if (cfg_.swOnRes.size() != cfg_.swOffRes.size() || cfg_.swOnRes.size() != cfg_.swType.size()) {
        throw std::runtime_error("swOnRes, swOffRes, and swType must have the same size.");
    }

    if (cfg_.upGroupIndex < 0 || cfg_.lowGroupIndex < 0) {
        throw std::runtime_error("upGroupIndex and lowGroupIndex must be non-negative.");
    }

    const int nGroups = static_cast<int>(Cd_real_.rows() / 3);
    if (cfg_.upGroupIndex >= nGroups || cfg_.lowGroupIndex >= nGroups) {
        throw std::runtime_error("Selected feedback group index exceeds available DSS output groups.");
    }

    for (const auto& inj : cfg_.feedbackInjections) {
        if (inj.inputBlockIndex < 0 || inj.inputBlockIndex >= cfg_.nInputBlocks) {
            throw std::runtime_error("Feedback injection block index is out of range.");
        }
    }
}

DQsymrun::MatrixXcd DQsymrun::getInternalSignalMatrix(
    InternalSignal signal,
    const MatrixXcd& Iup,
    const MatrixXcd& Ilow,
    const MatrixXcd& VcUp,
    const MatrixXcd& VcLow,
    const MatrixXcd& VoutUp,
    const MatrixXcd& VoutLow
)
{
    switch (signal) {
    case InternalSignal::VoutUp:
        return VoutUp;
    case InternalSignal::VoutLow:
        return VoutLow;
    case InternalSignal::VcUp:
        return VcUp;
    case InternalSignal::VcLow:
        return VcLow;
    case InternalSignal::Iup:
        return Iup;
    case InternalSignal::Ilow:
        return Ilow;
    default:
        throw std::runtime_error("Unknown internal signal selection.");
    }
}

//to be updated 
//DQsymrun::Result DQsymrun::run()
//{
//    validateInputs();
//
//    DQsym dq;
//    dq.reset();
//
//    MatrixXcd Ad = Ad_real_.cast<cd>();
//    MatrixXcd Bd = Bd_real_.cast<cd>();
//    MatrixXcd Cd = Cd_real_.cast<cd>();
//    MatrixXcd Dd = Dd_real_.cast<cd>();
//
//    VectorXcd xo = VectorXcd::Zero(Ad.rows());
//
//    const int N = static_cast<int>((cfg_.tEnd - cfg_.t0) / cfg_.Ts) + 1;
//    const int nGroups = static_cast<int>(Cd.rows() / 3);
//    const int maxGroupIndex = std::max(cfg_.upGroupIndex, cfg_.lowGroupIndex);
//
//    MatrixXcd Uup = Simple_MMC::makeArmControlCoeffs(cfg_.nKeep, Simple_MMC::ArmType::Upper); // calculate_time_step
//    MatrixXcd Ulow = Simple_MMC::makeArmControlCoeffs(cfg_.nKeep, Simple_MMC::ArmType::Lower);
//
//    MatrixXcd ZupOld = MatrixXcd::Zero(3, cfg_.nArm);
//    MatrixXcd XupOld = MatrixXcd::Zero(3, cfg_.nArm);
//    MatrixXcd ZlowOld = MatrixXcd::Zero(3, cfg_.nArm);
//    MatrixXcd XlowOld = MatrixXcd::Zero(3, cfg_.nArm);
//
//    MatrixXcd VoutUpForMMC = MatrixXcd::Zero(3, cfg_.nKeep);
//    MatrixXcd VoutLowForMMC = MatrixXcd::Zero(3, cfg_.nKeep);
//
//    MatrixXcd lastIup = MatrixXcd::Zero(3, cfg_.nKeep);
//    MatrixXcd lastIlow = MatrixXcd::Zero(3, cfg_.nKeep);
//    MatrixXcd lastVcUp = MatrixXcd::Zero(3, cfg_.nKeep);
//    MatrixXcd lastVcLow = MatrixXcd::Zero(3, cfg_.nKeep);
//
//    Result result;
//    result.time.resize(N);
//
//    result.DSSabcHist.assign(nGroups, Eigen::MatrixXd::Zero(N, 3));
//
//    // Can be the vector of matrices
//    result.UupAbcHist = Eigen::MatrixXd::Zero(N, 3);
//    result.UlowAbcHist = Eigen::MatrixXd::Zero(N, 3);
//    result.IupAbcHist = Eigen::MatrixXd::Zero(N, 3);
//    result.IlowAbcHist = Eigen::MatrixXd::Zero(N, 3);
//    result.VcUpAbcHist = Eigen::MatrixXd::Zero(N, 3);
//    result.VcLowAbcHist = Eigen::MatrixXd::Zero(N, 3);
//    result.VoutUpAbcHist = Eigen::MatrixXd::Zero(N, 3);
//    result.VoutLowAbcHist = Eigen::MatrixXd::Zero(N, 3);
//
//    Eigen::VectorXd swOnRes = cfg_.swOnRes;
//    Eigen::VectorXd swOffRes = cfg_.swOffRes;
//    Eigen::VectorXi swType = cfg_.swType;
//
//    result.brkHistory = Eigen::MatrixXi::Zero(N, swType.size());
//
//    for (int k = 0; k < N; ++k)
//    {
//        const double t = cfg_.t0 + k * cfg_.Ts;
//        const double theta = 2.0 * M_PI * cfg_.f * t;
//        result.time[k] = t;
//
//        Eigen::VectorXi brkVec;
//        if (cfg_.breakerFunction) {
//            brkVec = cfg_.breakerFunction(k, t);
//        }
//        else {
//            brkVec = Eigen::VectorXi::Zero(swType.size());
//        }
//
//        if (brkVec.size() != swType.size()) {
//            throw std::runtime_error("breakerFunction returned a vector with wrong size.");
//        }
//
//        result.brkHistory.row(k) = brkVec.transpose();
//
//        std::vector<MatrixXcd> uBlocks;
//        if (cfg_.externalInputFunction) {
//            uBlocks = cfg_.externalInputFunction(k, t);
//        }
//        else {
//            uBlocks.assign(cfg_.nInputBlocks, MatrixXcd::Zero(3, cfg_.nKeep));
//        }
//
//        if (static_cast<int>(uBlocks.size()) != cfg_.nInputBlocks) {
//            throw std::runtime_error("externalInputFunction returned wrong number of input blocks.");
//        }
//
//        for (int b = 0; b < cfg_.nInputBlocks; ++b) {
//            if (uBlocks[b].rows() != 3 || uBlocks[b].cols() != cfg_.nKeep) {
//                throw std::runtime_error("Each input block must be 3 x nKeep.");
//            }
//        }
//
//        for (const auto& inj : cfg_.feedbackInjections) {
//            MatrixXcd signal = getInternalSignalMatrix(
//                inj.signal,
//                lastIup,
//                lastIlow,
//                lastVcUp,
//                lastVcLow,
//                VoutUpForMMC,
//                VoutLowForMMC
//            );
//
//            if (inj.invert) {
//                signal = -signal;
//            }
//
//            if (signal.rows() != 3 || signal.cols() != cfg_.nKeep) {
//                throw std::runtime_error("Injected internal signal must have size 3 x nKeep.");
//            }
//
//            uBlocks[inj.inputBlockIndex] = signal;
//        }
//
//        MatrixXcd u(3 * cfg_.nInputBlocks, cfg_.nKeep);
//        for (int b = 0; b < cfg_.nInputBlocks; ++b) {
//            u.block(3 * b, 0, 3, cfg_.nKeep) = uBlocks[b];
//        }
//
//        MatrixXcd y = dq.DSSS(
//            Ad, Bd, Cd, Dd,
//            swOnRes, swOffRes,
//            swType, brkVec,
//            u, xo,
//            cfg_.Ts, cfg_.f
//        );
//
//        if (y.cols() < cfg_.nKeep || y.rows() < 3 * (maxGroupIndex + 1)) {
//            throw std::runtime_error("DSSS output has unexpected dimensions.");
//        }
//
//        MatrixXcd Iup = y.block(3 * cfg_.upGroupIndex, 0, 3, cfg_.nKeep);
//        MatrixXcd Ilow = y.block(3 * cfg_.lowGroupIndex, 0, 3, cfg_.nKeep);
//
//        if (cfg_.invertUpFeedback) {
//            Iup = -Iup;
//        }
//
//        if (cfg_.invertLowFeedback) {
//            Ilow = -Ilow;
//        }
//
//        MatrixXcd ProdUp = dq.multiply(Uup, Iup);
//        MatrixXcd ProdLow = dq.multiply(Ulow, Ilow);
//
//        ProdUp = truncateHarmonics(ProdUp, cfg_.nArm);
//        ProdLow = truncateHarmonics(ProdLow, cfg_.nArm);
//
//        MatrixXcd XinUp = truncateHarmonics(ProdUp * (1.0 / cfg_.C), cfg_.nArm);
//        MatrixXcd XinLow = truncateHarmonics(ProdLow * (1.0 / cfg_.C), cfg_.nArm);
//
//        MatrixXcd VcUpArm = dq.integrate(ZupOld, XupOld, XinUp, cfg_.Ts, cfg_.omega);
//        MatrixXcd VcLowArm = dq.integrate(ZlowOld, XlowOld, XinLow, cfg_.Ts, cfg_.omega);
//
//        VcUpArm = truncateHarmonics(VcUpArm, cfg_.nArm);
//        VcLowArm = truncateHarmonics(VcLowArm, cfg_.nArm);
//
//        ZupOld = VcUpArm;
//        XupOld = XinUp;
//        ZlowOld = VcLowArm;
//        XlowOld = XinLow;
//
//        MatrixXcd VoutUpFull = dq.multiply(VcUpArm, Uup);
//        MatrixXcd VoutLowFull = dq.multiply(VcLowArm, Ulow);
//
//        VoutUpForMMC = truncateHarmonics(VoutUpFull, cfg_.nKeep);
//        VoutLowForMMC = truncateHarmonics(VoutLowFull, cfg_.nKeep);
//
//        lastIup = Iup;
//        lastIlow = Ilow;
//        lastVcUp = truncateHarmonics(VcUpArm, cfg_.nKeep);
//        lastVcLow = truncateHarmonics(VcLowArm, cfg_.nKeep);
//
//        std::vector<Eigen::Vector3d> abcGroups = dq.dqn2abc_groups_at_time(y, theta);
//        if (static_cast<int>(abcGroups.size()) != nGroups) {
//            throw std::runtime_error("dqn2abc_groups_at_time returned unexpected number of groups.");
//        }
//
//        for (int g = 0; g < nGroups; ++g) {
//            result.DSSabcHist[g].row(k) = abcGroups[g].transpose();
//        }
//
//        result.UupAbcHist.row(k) = dq.dqn2abc_at_time(Uup, theta).transpose();
//        result.UlowAbcHist.row(k) = dq.dqn2abc_at_time(Ulow, theta).transpose();
//
//        result.IupAbcHist.row(k) = dq.dqn2abc_at_time(Iup, theta).transpose();
//        result.IlowAbcHist.row(k) = dq.dqn2abc_at_time(Ilow, theta).transpose();
//
//        result.VcUpAbcHist.row(k) = dq.dqn2abc_at_time(lastVcUp, theta).transpose();
//        result.VcLowAbcHist.row(k) = dq.dqn2abc_at_time(lastVcLow, theta).transpose();
//
//        result.VoutUpAbcHist.row(k) = dq.dqn2abc_at_time(VoutUpForMMC, theta).transpose();
//        result.VoutLowAbcHist.row(k) = dq.dqn2abc_at_time(VoutLowForMMC, theta).transpose();
//    }
//
//    result_ = result;
//    hasRun_ = true;
//    return result_;
//}

void DQsymrun::exportCSV(const std::string& filename) const
{
    if (!hasRun_) {
        throw std::runtime_error("exportCSV() called before run().");
    }

    std::vector<Eigen::MatrixXd> values;
    values.push_back(result_.brkHistory.cast<double>());

    for (const auto& x : result_.DSSabcHist) {
        values.push_back(x);
    }

    values.push_back(result_.UupAbcHist);
    values.push_back(result_.UlowAbcHist);
    values.push_back(result_.IupAbcHist);
    values.push_back(result_.IlowAbcHist);
    values.push_back(result_.VcUpAbcHist);
    values.push_back(result_.VcLowAbcHist);
    values.push_back(result_.VoutUpAbcHist);
    values.push_back(result_.VoutLowAbcHist);

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

void DQsymrun::plot() const
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
    mmcWaveforms.push_back(result_.UupAbcHist);
    mmcWaveforms.push_back(result_.UlowAbcHist);
    mmcWaveforms.push_back(result_.IupAbcHist);
    mmcWaveforms.push_back(result_.IlowAbcHist);
    mmcWaveforms.push_back(result_.VcUpAbcHist);
    mmcWaveforms.push_back(result_.VcLowAbcHist);
    mmcWaveforms.push_back(result_.VoutUpAbcHist);
    mmcWaveforms.push_back(result_.VoutLowAbcHist);

    plot_abc_groups_implot(
        result_.time,
        mmcWaveforms,
        "MMC internal abc waveforms"
    );
}

const DQsymrun::Result& DQsymrun::getResult() const
{
    if (!hasRun_) {
        throw std::runtime_error("getResult() called before run().");
    }

    return result_;
}

bool DQsymrun::hasRun() const
{
    return hasRun_;
}