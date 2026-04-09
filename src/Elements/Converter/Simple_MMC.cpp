#include "Simple_MMC.h"


void Simple_MMC::reset()
{
    dq_.reset();

    A_matrix.resize(0, 0);
    B_matrix.resize(0, 0);
    C_matrix.resize(0, 0);
    D_matrix.resize(0, 0);

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
    //A_matrix = Ad;
    //B_matrix = Bd;
    //C_matrix = Cd;
    //D_matrix = Dd;
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
    w_ = 2.0 * M_PI * f0_;
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




MatrixXcd Simple_MMC::makeUpperControlCoeffs(Eigen::Index nCols) const
{
    if (nCols < 2) {
        throw std::invalid_argument("Upper control coefficient matrix needs at least 2 columns.");
    }

    MatrixXcd A = MatrixXcd::Zero(3, nCols);
    MatrixXcd B = MatrixXcd::Zero(3, nCols);

    A(0, 1) = complex<double>(-1.0, 0.0);
    B(2, 0) = complex<double>(1.0, 0.0);

    return 0.5 * (A + B);
}

MatrixXcd Simple_MMC::makeLowerControlCoeffs(Eigen::Index nCols) const
{
    if (nCols < 2) {
        throw std::invalid_argument("Lower control coefficient matrix needs at least 2 columns.");
    }

    MatrixXcd A = MatrixXcd::Zero(3, nCols);
    MatrixXcd B = MatrixXcd::Zero(3, nCols);

    A(0, 1) = complex<double>(1.0, 0.0);
    B(2, 0) = complex<double>(1.0, 0.0);

    return 0.5 * (A + B);
}

MatrixXcd Simple_MMC::truncateHarmonics(const MatrixXcd& X,
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