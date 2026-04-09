#include "DQsym.h"
#include "../Helper_Functions/Helper_Functions.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <stdexcept>
#include <vector>

namespace
{
    constexpr double PI_DQSYM = 3.141592653589793238462643383279502884;
}

/**
 * @brief Adds two complex matrices element-wise, handling different sizes.
 *
 * This function computes the sum of two matrices, `a` and `b`. If the matrices
 * have different dimensions, the result matrix is sized to encompass both,
 * effectively zero-padding the smaller matrix before addition.
 *
 * @param a The first matrix operand.
 * @param b The second matrix operand.
 * @return A new matrix representing the sum of `a` and `b`.
 */
MatrixXcd DQsym::add(const MatrixXcd& a, const MatrixXcd& b)
{
    long max_rows = std::max(a.rows(), b.rows());
    long max_cols = std::max(a.cols(), b.cols());

    MatrixXcd result = MatrixXcd::Zero(max_rows, max_cols);

    result.block(0, 0, a.rows(), a.cols()) += a;
    result.block(0, 0, b.rows(), b.cols()) += b;

    return result;
}

/**
 * @brief Subtracts one complex matrix from another element-wise, handling different sizes.
 *
 * This function computes the difference of two matrices, `a - b`. If the matrices
 * have different dimensions, the result matrix is sized to encompass both,
 * effectively zero-padding the smaller matrix before subtraction.
 *
 * @param a The matrix to subtract from (minuend).
 * @param b The matrix to subtract (subtrahend).
 * @return A new matrix representing the difference `a - b`.
 */
MatrixXcd DQsym::subtract(const MatrixXcd& a, const MatrixXcd& b)
{
    long max_rows = std::max(a.rows(), b.rows());
    long max_cols = std::max(a.cols(), b.cols());

    MatrixXcd result = MatrixXcd::Zero(max_rows, max_cols);

    result.block(0, 0, a.rows(), a.cols()) += a;
    result.block(0, 0, b.rows(), b.cols()) -= b;

    return result;
}

/**
 * @brief Three-phase product of two dynamic-phasor series (harmonic convolution).
 *
 * Given the dynamic-phasor Fourier coefficients of two 3-phase signals
 * x(t) and y(t), this routine computes the dynamic-phasor coefficients of
 * their pointwise product z(t) = x(t).*y(t) in the original 3-phase basis.
 *
 * @note Signal/Basis Convention:
 * Inputs are 3x(N+1) complex matrices of harmonic coefficients in the ABC basis,
 * where rows correspond to phases a, b, and c. Column k+1 stores the k-th
 * harmonic coefficient (k=0..N), with the convention:
 * - `Re{coef(:,k+1)}` corresponds to the sine component.
 * - `Im{coef(:,k+1)}` corresponds to the cosine component.
 * Internally, the computation is performed in the symmetrical components (pnz)
 * basis and then mapped back to the ABC basis.
 * 
 * @param x_coef1_in A 3x(Nx+1) complex matrix representing the dynamic-phasor
 *                   coefficients of signal x(t) up to harmonic Nx.
 * @param y_coef1_in A 3x(Ny+1) complex matrix representing the dynamic-phasor
 *    
 coefficients of signal y(t) up to harmonic Ny.
 * @return A 3x(2N+1) complex matrix containing the dynamic-phasor coefficients
 *         of the product z(t) in the ABC basis, from harmonic 0 to 2N.
 */
//MatrixXcd DQsym::multiply(const MatrixXcd& x_coef1_in, const MatrixXcd& y_coef1_in)
//{
//    if (x_coef1_in.rows() != 3 || y_coef1_in.rows() != 3) {
//        throw std::invalid_argument("Input coefficient matrices must have 3 rows (phases a,b,c)");
//    }
//
//
//    Matrix3cd Sas;
//    const std::complex<double> a(-0.5, 0.8660254037844386);
//    const std::complex<double> a2 = std::conj(a);
//    Sas << std::complex<double>(1, 0), a, a2,
//        std::complex<double>(1, 0), a2, a,
//        std::complex<double>(1, 0), std::complex<double>(1, 0), std::complex<double>(1, 0);
//    Sas /= 3.0;
//    Matrix3cd Ssa = Sas.inverse();
//
//    int N = std::max(x_coef1_in.cols() - 1, y_coef1_in.cols() - 1);
//
//    const int max_k = 2 * N;
//    const int outCols = max_k + 1;
//
//
//    int sX = x_coef1_in.cols();
//    int sY = y_coef1_in.cols();
//    MatrixXcd x_coef1 = MatrixXcd::Zero(3, std::max(sX, N + 1));
//    MatrixXcd y_coef1 = MatrixXcd::Zero(3, std::max(sY, N + 1));
//    x_coef1.block(0, 0, 3, sX) = x_coef1_in;
//    y_coef1.block(0, 0, 3, sY) = y_coef1_in;
//
//
//
//    if (x_coef1.cols() < N + 1) x_coef1.conservativeResize(3, N + 1);
//    if (y_coef1.cols() < N + 1) y_coef1.conservativeResize(3, N + 1);
//
//    MatrixXcd X_pnz = Ssa * x_coef1;
//    MatrixXcd Y_pnz = Ssa * y_coef1;
//
//    MatrixXcd Z_pnz = MatrixXcd::Zero(3, outCols);
//    MatrixXd Cs = MatrixXd::Zero(3, outCols);
//    MatrixXd Cc = MatrixXd::Zero(3, outCols);
//    VectorXd C0 = VectorXd::Zero(3);
//
//
//
//    for (int m = 0; m <= N; ++m) {
//        VectorXd axs = X_pnz.col(m).real();
//        VectorXd axc = X_pnz.col(m).imag();
//
//        for (int n = 0; n <= N; ++n) {
//            VectorXd bxs = Y_pnz.col(n).real();
//            VectorXd bxc = Y_pnz.col(n).imag();
//
//
//            if (m == 0 && n == 0) {
//                C0 += X_pnz.col(0).real().cwiseProduct(Y_pnz.col(0).real());
//            }
//
//
//            if (m == n && m > 0) {
//                C0 += 0.5 * (axs.array() * bxs.array() + axc.array() * bxc.array()).matrix();
//            }
//
//
//            if (m > 0 && n > 0) {
//                int k_plus = m + n;
//                int k_minus = std::abs(m - n);
//
//
//                if (k_plus <= max_k) {
//                    Cs.col(k_plus) += (0.5 * axs.array() * bxc.array()).matrix();
//                }
//                if (k_minus > 0 && k_minus <= max_k) {
//                    int sign_k = sgn(m - n);
//                    Cs.col(k_minus) += (0.5 * sign_k * axs.array() * bxc.array()).matrix();
//                }
//
//
//                if (k_plus <= max_k) {
//                    Cs.col(k_plus) += (0.5 * axc.array() * bxs.array()).matrix();
//                }
//                if (k_minus > 0 && k_minus <= max_k) {
//                    int sign_k = -sgn(m - n);
//                    Cs.col(k_minus) += (0.5 * sign_k * axc.array() * bxs.array()).matrix();
//                }
//
//
//                if (k_plus <= max_k) {
//                    Cc.col(k_plus) += (0.5 * axc.array() * bxc.array()).matrix();
//                }
//                if (k_minus > 0 && k_minus <= max_k) {
//                    Cc.col(k_minus) += (0.5 * axc.array() * bxc.array()).matrix();
//                }
//
//
//                if (k_plus <= max_k) {
//                    Cc.col(k_plus) += (-0.5 * axs.array() * bxs.array()).matrix();
//                }
//                if (k_minus > 0 && k_minus <= max_k) {
//                    Cc.col(k_minus) += (0.5 * axs.array() * bxs.array()).matrix();
//                }
//            }
//
//
//            if (m == 0 && n > 0) {
//                Cs.col(n) += (x_coef1.col(0).real().array() * bxs.array()).matrix();
//                Cc.col(n) += (x_coef1.col(0).real().array() * bxc.array()).matrix();
//            }
//
//
//            if (n == 0 && m > 0) {
//                Cs.col(m) += (y_coef1.col(0).real().array() * axs.array()).matrix();
//                Cc.col(m) += (y_coef1.col(0).real().array() * axc.array()).matrix();
//            }
//        }
//    }
//
//
//    for (int i = 0; i < 3; ++i) {
//        Z_pnz(i, 0) = std::complex<double>(C0(i), 0.0);
//    }
//    for (int k = 1; k <= max_k; ++k) {
//        for (int i = 0; i < 3; ++i) {
//            Z_pnz(i, k) = std::complex<double>(Cs(i, k), Cc(i, k));
//        }
//    }
//
//    
//    MatrixXcd Zdcpnz_c = Sas * Z_pnz;
//
//    return Zdcpnz_c;
//}

/**
 * @brief Three-phase product of two dynamic-phasor series (harmonic convolution).
 *
 * C++ translation aligned with MATLAB:
 *   Zdcpnz_c = SICO_DPs_3ph(x_coef1, y_coef1, N)
 *
 * Input convention:
 * - 3 rows = abc phases
 * - column 0 = DC term
 * - column k = harmonic k
 *
 * Output:
 * - 3 x (2N+1) matrix in abc basis, where N = max(input harmonic order)
 */
MatrixXcd DQsym::multiply(const MatrixXcd& x_coef1_in, const MatrixXcd& y_coef1_in)
{
    
    if (x_coef1_in.rows() != 3 || y_coef1_in.rows() != 3) {
        throw std::invalid_argument("Input coefficient matrices must have 3 rows.");
    }


    Matrix3cd Sas;
    const std::complex<double> a(-0.5, 0.8660254037844386);
    const std::complex<double> a2(-0.5, -0.8660254037844386);

    Sas << std::complex<double>(1, 0), a, a2,
        std::complex<double>(1, 0), a2, a,
        std::complex<double>(1, 0), std::complex<double>(1, 0), std::complex<double>(1, 0);
    Sas /= 3.0;

    Matrix3cd Ssa = Sas.inverse();

    const int Nx = static_cast<int>(x_coef1_in.cols()) - 1;
    const int Ny = static_cast<int>(y_coef1_in.cols()) - 1;
    const int N = std::max(Nx, Ny);

    const int L = N + 1;
    const int max_k = 2 * N;
    const int outCols = max_k + 1;

    MatrixXcd x_coef1 = MatrixXcd::Zero(3, L);
    MatrixXcd y_coef1 = MatrixXcd::Zero(3, L);

    const int nx = std::min<int>(x_coef1_in.cols(), L);
    const int ny = std::min<int>(y_coef1_in.cols(), L);

    x_coef1.leftCols(nx) = x_coef1_in.leftCols(nx);
    y_coef1.leftCols(ny) = y_coef1_in.leftCols(ny);
    MatrixXcd X_pnz = Ssa * x_coef1;
    MatrixXcd Y_pnz = Ssa * y_coef1;


    MatrixXd Cs = MatrixXd::Zero(3, outCols);
    MatrixXd Cc = MatrixXd::Zero(3, outCols);
    VectorXd C0 = VectorXd::Zero(3);



    for (int m = 0; m <= N; ++m) {
        VectorXd axs = X_pnz.col(m).real();
        VectorXd axc = X_pnz.col(m).imag();

        for (int n = 0; n <= N; ++n) {
            VectorXd bxs = Y_pnz.col(n).real();
            VectorXd bxc = Y_pnz.col(n).imag();


            if (m == 0 && n == 0) {
                C0 += X_pnz.col(0).real().cwiseProduct(Y_pnz.col(0).real());
            }


            if (m == n && m > 0) {
                C0 += 0.5 * axs.cwiseProduct(bxs) + 0.5 * axc.cwiseProduct(bxc);
            }


            if (m > 0 && n > 0) {
                const int k_plus = m + n;
                const int k_minus = std::abs(m - n);
                const int s = sgn(m - n);

                Cs.col(k_plus) += 0.5 * axs.cwiseProduct(bxc)
                    + 0.5 * axc.cwiseProduct(bxs);

                Cc.col(k_plus) += 0.5 * axc.cwiseProduct(bxc)
                    - 0.5 * axs.cwiseProduct(bxs);

                if (k_minus > 0) {
                    Cs.col(k_minus) += 0.5 * s * axs.cwiseProduct(bxc)
                        - 0.5 * s * axc.cwiseProduct(bxs);

                    Cc.col(k_minus) += 0.5 * axc.cwiseProduct(bxc)
                        + 0.5 * axs.cwiseProduct(bxs);
                }
            }


            if (m == 0 && n > 0) {
                Cs.col(n) += X_pnz.col(0).real().cwiseProduct(bxs);
                Cc.col(n) += X_pnz.col(0).real().cwiseProduct(bxc);
            }


            if (n == 0 && m > 0) {
                Cs.col(m) += Y_pnz.col(0).real().cwiseProduct(axs);
                Cc.col(m) += Y_pnz.col(0).real().cwiseProduct(axc);
            }
        }
    }

    MatrixXcd xy_phasors = MatrixXcd::Zero(3, outCols);
    xy_phasors.col(0) = C0.cast<std::complex<double>>();
    for (int k = 1; k <= max_k; ++k) {
        for (int i = 0; i < 3; ++i) {
            xy_phasors(i, k) = std::complex<double>(Cs(i, k), Cc(i, k));
        }
    }

    MatrixXcd Zdcpnz_c = MatrixXcd::Zero(3, outCols);
    Zdcpnz_c = Sas * xy_phasors;
    return Zdcpnz_c;
}

/**
 * @brief Dynamic-phasor (DQ0) integrator per harmonic order.
 */

MatrixXcd DQsym::integrate(MatrixXcd& Zpnz_old, MatrixXcd& Xpnz_old, const MatrixXcd& Xpnz,
    double dt, double w)
{
    int N = Xpnz.cols() - 1;
    int nrSig = Xpnz.rows() / 3;



    if (Zpnz_old.rows() != nrSig * 3 || Xpnz_old.rows() != nrSig * 3 ||
        Zpnz_old.cols() != N + 1 || Xpnz_old.cols() != N + 1) {
        throw std::invalid_argument("Dimension mismatch in Int_DQN_Mat");
    }

    double dt2 = dt / 2.0;
    MatrixXcd Zpnz = MatrixXcd::Zero(nrSig * 3, N + 1);


    Zpnz.col(0) = dt2 * (Xpnz.col(0) + Xpnz_old.col(0)) + Zpnz_old.col(0);
    Xpnz_old.col(0) = Xpnz.col(0);
    Zpnz_old.col(0) = Zpnz.col(0);


    for (int i = 1; i <= N; ++i) {
        double wn = i * w;
        double wndt2 = wn * dt2;

        double A = 2.0 * dt2 / (1.0 + wndt2 * wndt2);
        double B = (1.0 - wndt2 * wndt2) / (1.0 + wndt2 * wndt2);

        const VectorXcd Xin = Xpnz.col(i);
        const VectorXcd Zin_old = Zpnz_old.col(i);


        VectorXd xr = Xin.real();
        VectorXd xi = Xin.imag();
        VectorXd zr = Zin_old.real();
        VectorXd zi = Zin_old.imag();

        VectorXd zr_new = A * (xr + wndt2 * xi) + (B * zr + A * wn * zi);
        VectorXd zi_new = A * (xi - wndt2 * xr) + (B * zi - A * wn * zr);

        Zpnz.col(i) = zr_new.cast<std::complex<double>>()
            + std::complex<double>(0, 1) * zi_new.cast<std::complex<double>>();

        Zpnz_old.col(i) = Zpnz.col(i);
    }

    return Zpnz;
}

/**
 * @brief Reset internal persistent DSSS state.
 */
void DQsym::reset()
{
    initialized = false;
    nStates = 0;
    nInputs = 0;
    nOutputs = 0;
    nSwitches = 0;

    x_old.resize(0, 0);
    Ads.resize(0, 0);
    Bds.resize(0, 0);
    Cds.resize(0, 0);
    Dds.resize(0, 0);

    swVec.resize(0);
    swVecOld.resize(0);
    yswitch.resize(0);
}

/**
* 
 * @brief Discrete phasor-domain state-space solver with switch-dependent matrix updates.
 * @note This implementation does not include a half-step predictor/corrector.
 * 
 * 
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


        buildMatricesForState(A0, B0, C0, D0,
            swVec, swType,
            swOnRes, swOffRes,
            Ads, Bds, Cds, Dds);

        initialized = true;
    }


    if ((swType.array() != 0).any())
        swVec = brkVec;


    if ((swType.array() != 0).any() && (swVecOld.array() != swVec.array()).any())
    {
        buildMatricesForState(A0, B0, C0, D0,
            swVec, swType,
            swOnRes, swOffRes,
            Ads, Bds, Cds, Dds);
        swVecOld = swVec;
    }


    x = Ads * x_old + Bds * u;


    VectorXcd expVec(T);
    for (int k = 0; k < T; ++k)
        expVec(k) = std::exp(std::complex<double>(0.0, -2.0 * PI_DQSYM * f0 * dt * k));

    MatrixXcd rotMat = expVec.asDiagonal();
    MatrixXcd x2 = x * rotMat;

    x_old = x2;
    y = Cds * x2 + Dds * u;

    return y;
}

/**
 * @brief Build matrices for a given switch state.
 */
void DQsym::buildMatricesForState(
    const MatrixXcd& A0, const MatrixXcd& B0,
    const MatrixXcd& C0, const MatrixXcd& D0,
    const VectorXi& swVec, const VectorXi& swType,
    const VectorXd& swOnRes, const VectorXd& swOffRes,
    MatrixXcd& Ao, MatrixXcd& Bo,
    MatrixXcd& Co, MatrixXcd& Do)
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


/**
 * @brief Convert state-space matrices into the phasor/DQ0 domain.

 */
void DQsym::convertToPhasor(const MatrixXcd& A, const MatrixXcd& B,
    const MatrixXcd& C, const MatrixXcd& D,
    MatrixXcd& Adc, MatrixXcd& Bdc,
    MatrixXcd& Cdc, MatrixXcd& Ddc)
{

    MatrixXcd Ad2 = A, Bd2 = B, Cd2 = C, Dd2 = D;

    std::complex<double> a(-0.5, 0.866);
    std::complex<double> a2(-0.5, -0.866);

    Matrix3cd Sas;
    Sas << 1.0, a, a2,
        1.0, a2, a,
        1.0, 1.0, 1.0;
    Sas /= 3.0;


    auto transformMatrix = [&](const MatrixXcd& Md1) {
        int rows = Md1.rows();
        int cols = Md1.cols();
        int nblk_r = rows / 3;
        int nblk_c = cols / 3;
        MatrixXcd Mdc = MatrixXcd::Zero(rows, cols);

        for (int i = 0; i < nblk_r; i++) {
            for (int j = 0; j < nblk_c; j++) {
                int r0 = i * 3;
                int c0 = j * 3;
    
            Matrix3cd sub = Md1.block(r0, c0, 3, 3);

            Mdc.block(r0, c0, 3, 3) = Sas * sub * Sas.inverse();
            }
        }
        return Mdc;
        };

    Adc = transformMatrix(Ad2);
    Bdc = transformMatrix(Bd2);
    Cdc = transformMatrix(Cd2);
    Ddc = transformMatrix(Dd2);
}



/**
 * @brief Reconstruct abc instantaneous values from dynamic phasor pnz coefficients at one angle theta.
 *
 * Input format:
 * - rows = 3 : positive, negative, zero sequence
 * - cols = harmonic orders, where col(0) is DC and col(h) is harmonic h
 *
 * @param Xdcpnz_c 3 x Nh complex coefficient matrix
 * @param theta electrical angle [rad]
 * @return Vector3d instantaneous abc values at theta
 */
Vector3d DQsym::dqn2abc_at_time(const MatrixXcd& Xdcpnz_c, double theta)
{
    if (Xdcpnz_c.rows() != 3) {
        throw std::runtime_error("Xdcpnz_c must have 3 rows.");
    }

    static bool transform_initialized = false;
    static Matrix3cd Sas;
    static Matrix3cd Ssa;

    if (!transform_initialized) {
        const std::complex<double> a(-0.5, std::sqrt(3.0) / 2.0);
        const std::complex<double> a2(-0.5, -std::sqrt(3.0) / 2.0);

        Sas <<
            std::complex<double>(1.0, 0.0), a, a2,
            std::complex<double>(1.0, 0.0), a2, a,
            std::complex<double>(1.0, 0.0), std::complex<double>(1.0, 0.0), std::complex<double>(1.0, 0.0);

        Sas /= 3.0;
        Ssa = Sas.inverse();
        transform_initialized = true;
    }

    Vector3d Xabc = Vector3d::Zero();
    const int ncols = static_cast<int>(Xdcpnz_c.cols());

    for (int i = 0; i < ncols; ++i) {
        if (i == 0) {

            Xabc += (Ssa * Xdcpnz_c.col(0)).real();
        }
        else {
            const int h = i;
            const double th = h * theta;

            const std::complex<double> Xp = Xdcpnz_c(0, i);
            const std::complex<double> Xn = Xdcpnz_c(1, i);
            const std::complex<double> Xz = Xdcpnz_c(2, i);

            const double mag_p = std::abs(Xp);
            const double ang_p = std::arg(Xp);

            const double mag_n = std::abs(Xn);
            const double ang_n = std::arg(Xn);

            const double mag_z = std::abs(Xz);
            const double ang_z = std::arg(Xz);

            Vector3d abc3p;
            abc3p <<
                mag_p * std::sin(th + ang_p),
                mag_p* std::sin(th + ang_p - 2.0 * PI_DQSYM / 3.0),
                mag_p* std::sin(th + ang_p + 2.0 * PI_DQSYM / 3.0);

            Vector3d abc3n;
            abc3n <<
                mag_n * std::sin(th + ang_n),
                mag_n* std::sin(th + ang_n + 2.0 * PI_DQSYM / 3.0),
                mag_n* std::sin(th + ang_n - 2.0 * PI_DQSYM / 3.0);

            Vector3d abc3z;
            abc3z <<
                mag_z * std::sin(th + ang_z),
                mag_z* std::sin(th + ang_z),
                mag_z* std::sin(th + ang_z);

            Xabc += abc3p + abc3n + abc3z;
        }
    }

    return Xabc;
}

/**
 * @brief Convert all 3-row output groups of Y to abc at a single electrical angle.
 *
 * Y is expected to have rows grouped as:
 * - rows 0..2   : group 1
 * - rows 3..5   : group 2
 * - rows 6..8   : group 3
 * - ...
 *
 * Each 3-row group is interpreted as a 3xH dynamic-phasor sequence matrix and
 * converted to one instantaneous abc vector at the supplied angle theta.
 *
 * @param Y A matrix with row count equal to 3 * number_of_groups.
 * @param theta Electrical angle [rad] at this one instant in time.
 * @return A vector of abc instantaneous vectors, one per 3-row group.
 */
std::vector<Vector3d> DQsym::dqn2abc_groups_at_time(const MatrixXcd& Y, double theta)
{
    if (Y.rows() == 0) {
        return {};
    }

    if (Y.rows() % 3 != 0) {
        throw std::runtime_error("Y row count must be a multiple of 3.");
    }

    const int nGroups = static_cast<int>(Y.rows() / 3);
    std::vector<Vector3d> out(nGroups);

    for (int g = 0; g < nGroups; ++g) {
        MatrixXcd block = Y.block(3 * g, 0, 3, Y.cols());
        out[g] = dqn2abc_at_time(block, theta);
    }

    return out;
}

/**
 * @brief Simulate abc waveform reconstruction over a time interval from dynamic phasor coefficients.
 *
 * @param Xdcpnz_c 3 x Nh complex coefficient matrix
 * @param freq_hz base electrical frequency [Hz]
 * @param t0 start time [s]
 * @param t1 end time [s]
 * @param Ts sample time [s]
 * @return ABCResult containing time vector and Nx3 abc waveform matrix
 */
ABCResult DQsym::simulate_dqn2abc(const MatrixXcd& Xdcpnz_c,
    double freq_hz, double t0, double t1, double Ts)
{
    if (Xdcpnz_c.rows() != 3) {
        throw std::runtime_error("Xdcpnz_c must have 3 rows.");
    }

    if (Ts <= 0.0) {
        throw std::runtime_error("Ts must be > 0.");
    }

    if (t1 < t0) {
        throw std::runtime_error("t1 must be >= t0.");
    }

    const int N = static_cast<int>((t1 - t0) / Ts) + 1;

    ABCResult res;
    res.t.resize(N);
    res.Xabc = MatrixXd::Zero(N, 3);

    for (int k = 0; k < N; ++k) {
        const double t = t0 + k * Ts;
        const double theta = 2.0 * PI_DQSYM * freq_hz * t;

        res.t[k] = t;
        res.Xabc.row(k) = dqn2abc_at_time(Xdcpnz_c, theta).transpose();
    }

    return res;
}