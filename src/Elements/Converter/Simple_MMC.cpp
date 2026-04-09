#include "Simple_MMC.h"

#include <algorithm>
#include <complex>
#include <stdexcept>

namespace
{
    using cd = std::complex<double>;
    constexpr double PI_SIMPLE_MMC = 3.141592653589793238462643383279502884;
}

Simple_MMC::Simple_MMC()
{
    reset();
}

void Simple_MMC::reset()
{
    dq_.reset();

    Ad_.resize(0, 0);
    Bd_.resize(0, 0);
    Cd_.resize(0, 0);
    Dd_.resize(0, 0);

    swOnRes_.resize(0);
    swOffRes_.resize(0);
    swType_.resize(0);

    xo_.resize(0);

    dt_ = 0.0;
    f0_ = 0.0;
    w_ = 0.0;
    C_ = 0.0;

    nKeepMMC_ = 5;
    nArm_ = 9;

    Uup_.resize(0, 0);
    Ulow_.resize(0, 0);

    ZupOld_.resize(0, 0);
    XupOld_.resize(0, 0);
    ZlowOld_.resize(0, 0);
    XlowOld_.resize(0, 0);

    VoutUpForMMC_.resize(0, 0);
    VoutLowForMMC_.resize(0, 0);

    matricesConfigured_ = false;
    switchesConfigured_ = false;
    paramsConfigured_ = false;
    stateConfigured_ = false;
}

void Simple_MMC::setSystemMatrices(const MatrixXcd& Ad,
    const MatrixXcd& Bd,
    const MatrixXcd& Cd,
    const MatrixXcd& Dd)
{
    if (Ad.rows() == 0 || Ad.cols() == 0 ||
        Bd.rows() == 0 || Bd.cols() == 0 ||
        Cd.rows() == 0 || Cd.cols() == 0 ||
        Dd.rows() == 0 || Dd.cols() == 0)
    {
        throw std::invalid_argument("System matrices must not be empty.");
    }

    if (Ad.rows() != Ad.cols()) {
        throw std::invalid_argument("Ad must be square.");
    }

    if (Bd.rows() != Ad.rows()) {
        throw std::invalid_argument("Bd row count must match Ad row count.");
    }

    if (Cd.cols() != Ad.cols()) {
        throw std::invalid_argument("Cd column count must match Ad column count.");
    }

    if (Dd.rows() != Cd.rows() || Dd.cols() != Bd.cols()) {
        throw std::invalid_argument("Dd dimensions must match Cd.rows x Bd.cols.");
    }

    Ad_ = Ad;
    Bd_ = Bd;
    Cd_ = Cd;
    Dd_ = Dd;

    matricesConfigured_ = true;
}

void Simple_MMC::setSwitchData(const VectorXd& swOnRes,
    const VectorXd& swOffRes,
    const VectorXi& swType)
{
    if (swOnRes.size() == 0 || swOffRes.size() == 0 || swType.size() == 0) {
        throw std::invalid_argument("Switch vectors must not be empty.");
    }

    if (swOnRes.size() != swOffRes.size() || swOnRes.size() != swType.size()) {
        throw std::invalid_argument("Switch vectors must have the same size.");
    }

    swOnRes_ = swOnRes;
    swOffRes_ = swOffRes;
    swType_ = swType;

    switchesConfigured_ = true;
}

void Simple_MMC::setParameters(double dt,
    double f0,
    double C,
    Eigen::Index nKeepMMC,
    Eigen::Index nArm)
{
    if (dt <= 0.0) {
        throw std::invalid_argument("dt must be > 0.");
    }

    if (f0 <= 0.0) {
        throw std::invalid_argument("f0 must be > 0.");
    }

    if (C <= 0.0) {
        throw std::invalid_argument("C must be > 0.");
    }

    if (nKeepMMC <= 0) {
        throw std::invalid_argument("nKeepMMC must be positive.");
    }

    if (nArm <= 0) {
        throw std::invalid_argument("nArm must be positive.");
    }

    dt_ = dt;
    f0_ = f0;
    w_ = 2.0 * PI_SIMPLE_MMC * f0_;
    C_ = C;

    nKeepMMC_ = nKeepMMC;
    nArm_ = nArm;

    Uup_ = makeUpperControlCoeffs(nKeepMMC_);
    Ulow_ = makeLowerControlCoeffs(nKeepMMC_);

    ZupOld_ = MatrixXcd::Zero(3, nArm_);
    XupOld_ = MatrixXcd::Zero(3, nArm_);
    ZlowOld_ = MatrixXcd::Zero(3, nArm_);
    XlowOld_ = MatrixXcd::Zero(3, nArm_);

    VoutUpForMMC_ = MatrixXcd::Zero(3, nKeepMMC_);
    VoutLowForMMC_ = MatrixXcd::Zero(3, nKeepMMC_);

    paramsConfigured_ = true;
}

void Simple_MMC::setInitialState(const VectorXcd& xo)
{
    if (!matricesConfigured_) {
        throw std::runtime_error("Set system matrices before setting initial state.");
    }

    if (xo.size() != Ad_.rows()) {
        throw std::invalid_argument("Initial state size must match number of states.");
    }

    xo_ = xo;
    stateConfigured_ = true;
}

const Simple_MMC::MatrixXcd& Simple_MMC::getLastVoutUp() const
{
    return VoutUpForMMC_;
}

const Simple_MMC::MatrixXcd& Simple_MMC::getLastVoutLow() const
{
    return VoutLowForMMC_;
}

const Simple_MMC::MatrixXcd& Simple_MMC::getUpperControlCoeffs() const
{
    return Uup_;
}

const Simple_MMC::MatrixXcd& Simple_MMC::getLowerControlCoeffs() const
{
    return Ulow_;
}

void Simple_MMC::validateConfigured() const
{
    if (!matricesConfigured_) {
        throw std::runtime_error("Simple_MMC system matrices are not configured.");
    }

    if (!switchesConfigured_) {
        throw std::runtime_error("Simple_MMC switch data are not configured.");
    }

    if (!paramsConfigured_) {
        throw std::runtime_error("Simple_MMC parameters are not configured.");
    }

    if (!stateConfigured_) {
        throw std::runtime_error("Simple_MMC initial state is not configured.");
    }
}

Simple_MMC::MatrixXcd Simple_MMC::stack_u_4x_3xN(const MatrixXcd& u1,
    const MatrixXcd& u2,
    const MatrixXcd& u3,
    const MatrixXcd& u4) const
{
    if (u1.rows() != 3 || u2.rows() != 3 || u3.rows() != 3 || u4.rows() != 3) {
        throw std::invalid_argument("All MMC input groups must have 3 rows.");
    }

    const Eigen::Index nCols = u1.cols();

    if (u2.cols() != nCols || u3.cols() != nCols || u4.cols() != nCols) {
        throw std::invalid_argument("All MMC input groups must have the same number of columns.");
    }

    MatrixXcd u(12, nCols);
    u.block(0, 0, 3, nCols) = u1;
    u.block(3, 0, 3, nCols) = u2;
    u.block(6, 0, 3, nCols) = u3;
    u.block(9, 0, 3, nCols) = u4;

    return u;
}

Simple_MMC::MatrixXcd Simple_MMC::makeUpperControlCoeffs(Eigen::Index nCols) const
{
    if (nCols < 2) {
        throw std::invalid_argument("Upper control coefficient matrix needs at least 2 columns.");
    }

    MatrixXcd A = MatrixXcd::Zero(3, nCols);
    MatrixXcd B = MatrixXcd::Zero(3, nCols);

    A(0, 1) = cd(-1.0, 0.0);
    B(2, 0) = cd(1.0, 0.0);

    return 0.5 * (A + B);
}

Simple_MMC::MatrixXcd Simple_MMC::makeLowerControlCoeffs(Eigen::Index nCols) const
{
    if (nCols < 2) {
        throw std::invalid_argument("Lower control coefficient matrix needs at least 2 columns.");
    }

    MatrixXcd A = MatrixXcd::Zero(3, nCols);
    MatrixXcd B = MatrixXcd::Zero(3, nCols);

    A(0, 1) = cd(1.0, 0.0);
    B(2, 0) = cd(1.0, 0.0);

    return 0.5 * (A + B);
}

Simple_MMC::MatrixXcd Simple_MMC::truncateHarmonics(const MatrixXcd& X,
    Eigen::Index nColsToKeep) const
{
    if (nColsToKeep <= 0) {
        throw std::invalid_argument("nColsToKeep must be positive.");
    }

    MatrixXcd Y = MatrixXcd::Zero(X.rows(), nColsToKeep);
    const Eigen::Index colsToCopy = std::min<Eigen::Index>(X.cols(), nColsToKeep);
    Y.leftCols(colsToCopy) = X.leftCols(colsToCopy);

    return Y;
}

Simple_MMC::StepResult Simple_MMC::step(const MatrixXcd& u1,
    const MatrixXcd& u2,
    const VectorXi& brkVec)
{
    validateConfigured();

    if (brkVec.size() != swType_.size()) {
        throw std::invalid_argument("brkVec size must match switch vector size.");
    }

    if (u1.rows() != 3 || u2.rows() != 3) {
        throw std::invalid_argument("u1 and u2 must have 3 rows.");
    }

    if (u1.cols() != nKeepMMC_ || u2.cols() != nKeepMMC_) {
        throw std::invalid_argument("u1 and u2 column count must match nKeepMMC.");
    }

    MatrixXcd u3 = VoutUpForMMC_;
    MatrixXcd u4 = VoutLowForMMC_;

    MatrixXcd u = stack_u_4x_3xN(u1, u2, u3, u4);

    MatrixXcd y = dq_.DSSS(
        Ad_, Bd_, Cd_, Dd_,
        swOnRes_, swOffRes_,
        swType_, brkVec,
        u, xo_,
        dt_, f0_);

    if (y.rows() < 9 || y.cols() < nKeepMMC_) {
        throw std::runtime_error("MMC DSSS output has unexpected dimensions.");
    }

    MatrixXcd Iup = y.block(3, 0, 3, nKeepMMC_);
    MatrixXcd Ilow = y.block(6, 0, 3, nKeepMMC_);

    MatrixXcd ProdUp = dq_.multiply(Uup_, Iup);
    MatrixXcd ProdLow = dq_.multiply(Ulow_, Ilow);

    ProdUp = truncateHarmonics(ProdUp, nArm_);
    ProdLow = truncateHarmonics(ProdLow, nArm_);

    MatrixXcd XinUp = truncateHarmonics(ProdUp * (1.0 / C_), nArm_);
    MatrixXcd XinLow = truncateHarmonics(ProdLow * (1.0 / C_), nArm_);

    MatrixXcd VcUp = dq_.integrate(ZupOld_, XupOld_, XinUp, dt_, w_);
    MatrixXcd VcLow = dq_.integrate(ZlowOld_, XlowOld_, XinLow, dt_, w_);

    VcUp = truncateHarmonics(VcUp, nArm_);
    VcLow = truncateHarmonics(VcLow, nArm_);

    ZupOld_ = VcUp;
    XupOld_ = XinUp;
    ZlowOld_ = VcLow;
    XlowOld_ = XinLow;

    MatrixXcd VoutUpFull = dq_.multiply(VcUp, Uup_);
    MatrixXcd VoutLowFull = dq_.multiply(VcLow, Ulow_);

    VoutUpForMMC_ = truncateHarmonics(VoutUpFull, nKeepMMC_);
    VoutLowForMMC_ = truncateHarmonics(VoutLowFull, nKeepMMC_);

    StepResult result;
    result.y_mmc = y;
    result.i_up = Iup;
    result.i_low = Ilow;
    result.vc_up = VcUp;
    result.vc_low = VcLow;
    result.vout_up = VoutUpForMMC_;
    result.vout_low = VoutLowForMMC_;

    return result;
}