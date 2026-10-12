/**
 * @file DQsym_Conversion_Functions.cpp
 * @brief Implementation of Dynamic-phasor arithmetic and abc ↔ dqn domain conversions.
 */
#include "DQsym_Conversion_Functions.h"
#include "Standard_functions.h"

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
MatrixXcd dq_add(const MatrixXcd& a, const MatrixXcd& b)
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
MatrixXcd dq_subtract(const MatrixXcd& a, const MatrixXcd& b)
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
struct DqMulWork {
    MatrixXcd xpad, ypad, Xp, Yp, xy;
    MatrixXd Cs, Cc;
    Vector3d C0;
};

thread_local DqMulWork g_dqMulWork;

static void dqSequenceTransforms(Matrix3cd& Sas, Matrix3cd& Ssa)
{
    static const Matrix3cd kSas = [] {
        const std::complex<double> a(-0.5, 0.8660254037844386);
        const std::complex<double> a2(-0.5, -0.8660254037844386);
        Matrix3cd S;
        S << std::complex<double>(1, 0), a, a2,
            std::complex<double>(1, 0), a2, a,
            std::complex<double>(1, 0), std::complex<double>(1, 0), std::complex<double>(1, 0);
        S /= 3.0;
        return S;
    }();
    static const Matrix3cd kSsa = kSas.inverse();
    Sas = kSas;
    Ssa = kSsa;
}

void dq_multiply_into(const MatrixXcd& x_coef1_in, const MatrixXcd& y_coef1_in,
    MatrixXcd& out, int nColsToKeep)
{
    if (x_coef1_in.rows() != 3 || y_coef1_in.rows() != 3) {
        throw std::invalid_argument("Input coefficient matrices must have 3 rows.");
    }

    Matrix3cd Sas, Ssa;
    dqSequenceTransforms(Sas, Ssa);

    const int Nx = static_cast<int>(x_coef1_in.cols()) - 1;
    const int Ny = static_cast<int>(y_coef1_in.cols()) - 1;
    const int N = std::max(Nx, Ny);
    const int L = N + 1;
    const int max_k = 2 * N;
    const int fullCols = max_k + 1;
    const int keepCols = (nColsToKeep > 0) ? nColsToKeep : fullCols;

    DqMulWork& w = g_dqMulWork;

    auto ensureC = [](MatrixXcd& M, int r, int c) {
        if (M.rows() != r || M.cols() != c)
            M.resize(r, c);
        M.setZero();
    };
    auto ensureR = [](MatrixXd& M, int r, int c) {
        if (M.rows() != r || M.cols() != c)
            M.resize(r, c);
        M.setZero();
    };

    ensureC(w.xpad, 3, L);
    ensureC(w.ypad, 3, L);
    const int nx = std::min<int>(static_cast<int>(x_coef1_in.cols()), L);
    const int ny = std::min<int>(static_cast<int>(y_coef1_in.cols()), L);
    w.xpad.leftCols(nx) = x_coef1_in.leftCols(nx);
    w.ypad.leftCols(ny) = y_coef1_in.leftCols(ny);

    w.Xp.resize(3, L);
    w.Yp.resize(3, L);
    w.Xp.noalias() = Ssa * w.xpad;
    w.Yp.noalias() = Ssa * w.ypad;

    ensureR(w.Cs, 3, keepCols);
    ensureR(w.Cc, 3, keepCols);
    w.C0.setZero();

    const MatrixXcd& Xp = w.Xp;
    const MatrixXcd& Yp = w.Yp;

    for (int m = 0; m <= N; ++m) {
        for (int n = 0; n <= N; ++n) {
            if (m == 0 && n == 0) {
                for (int i = 0; i < 3; ++i)
                    w.C0(i) += Xp(i, 0).real() * Yp(i, 0).real();
            }

            if (m == n && m > 0) {
                for (int i = 0; i < 3; ++i) {
                    const double axs = Xp(i, m).real();
                    const double axc = Xp(i, m).imag();
                    const double bxs = Yp(i, n).real();
                    const double bxc = Yp(i, n).imag();
                    w.C0(i) += 0.5 * axs * bxs + 0.5 * axc * bxc;
                }
            }

            if (m > 0 && n > 0) {
                const int k_plus = m + n;
                const int k_minus = std::abs(m - n);
                const int s = sgn(m - n);
                for (int i = 0; i < 3; ++i) {
                    const double axs = Xp(i, m).real();
                    const double axc = Xp(i, m).imag();
                    const double bxs = Yp(i, n).real();
                    const double bxc = Yp(i, n).imag();
                    if (k_plus < keepCols) {
                        w.Cs(i, k_plus) += 0.5 * axs * bxc + 0.5 * axc * bxs;
                        w.Cc(i, k_plus) += 0.5 * axc * bxc - 0.5 * axs * bxs;
                    }
                    if (k_minus > 0 && k_minus < keepCols) {
                        w.Cs(i, k_minus) += 0.5 * s * axs * bxc - 0.5 * s * axc * bxs;
                        w.Cc(i, k_minus) += 0.5 * axc * bxc + 0.5 * axs * bxs;
                    }
                }
            }

            if (m == 0 && n > 0 && n < keepCols) {
                for (int i = 0; i < 3; ++i) {
                    const double x0 = Xp(i, 0).real();
                    w.Cs(i, n) += x0 * Yp(i, n).real();
                    w.Cc(i, n) += x0 * Yp(i, n).imag();
                }
            }

            if (n == 0 && m > 0 && m < keepCols) {
                for (int i = 0; i < 3; ++i) {
                    const double y0 = Yp(i, 0).real();
                    w.Cs(i, m) += y0 * Xp(i, m).real();
                    w.Cc(i, m) += y0 * Xp(i, m).imag();
                }
            }
        }
    }

    ensureC(w.xy, 3, keepCols);
    w.xy.col(0) = w.C0.cast<std::complex<double>>();
    const int kMax = std::min(max_k, keepCols - 1);
    for (int k = 1; k <= kMax; ++k) {
        for (int i = 0; i < 3; ++i)
            w.xy(i, k) = std::complex<double>(w.Cs(i, k), w.Cc(i, k));
    }

    if (out.rows() != 3 || out.cols() != keepCols)
        out.resize(3, keepCols);
    out.noalias() = Sas * w.xy;
}

MatrixXcd dq_multiply(const MatrixXcd& x_coef1_in, const MatrixXcd& y_coef1_in)
{
    MatrixXcd Z;
    dq_multiply_into(x_coef1_in, y_coef1_in, Z, 0);
    return Z;
}

/**
 * @brief Dynamic-phasor (DQ0) integrator per harmonic order.
 */

MatrixXcd dq_integrate(MatrixXcd& Zpnz_old, MatrixXcd& Xpnz_old, const MatrixXcd& Xpnz,
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
 * @brief Convert state-space matrices into the phasor/DQ0 domain.

 */
void convertToPhasor(const MatrixXcd& A, const MatrixXcd& B,
    const MatrixXcd& C, const MatrixXcd& D,
    MatrixXcd& Adc, MatrixXcd& Bdc,
    MatrixXcd& Cdc, MatrixXcd& Ddc)
{
    // Match the historical 0.866 (not sqrt(3)/2) so existing DSSS fixtures stay bitwise-close.
    static const Matrix3cd Sas = [] {
        const std::complex<double> a(-0.5, 0.866);
        const std::complex<double> a2(-0.5, -0.866);
        Matrix3cd S;
        S << 1.0, a, a2,
            1.0, a2, a,
            1.0, 1.0, 1.0;
        S /= 3.0;
        return S;
    }();
    static const Matrix3cd SasInv = Sas.inverse();

    auto transformMatrix = [&](const MatrixXcd& Md1) {
        const int rows = static_cast<int>(Md1.rows());
        const int cols = static_cast<int>(Md1.cols());
        MatrixXcd Mdc = MatrixXcd::Zero(rows, cols);
        const int nblk_r = rows / 3;
        const int nblk_c = cols / 3;
        for (int i = 0; i < nblk_r; ++i) {
            for (int j = 0; j < nblk_c; ++j) {
                const int r0 = i * 3;
                const int c0 = j * 3;
                Mdc.block<3, 3>(r0, c0).noalias() =
                    Sas * Md1.block<3, 3>(r0, c0) * SasInv;
            }
        }
        return Mdc;
    };

    Adc = transformMatrix(A);
    Bdc = transformMatrix(B);
    Cdc = transformMatrix(C);
    Ddc = transformMatrix(D);
}

MatrixXcd truncateHarmonics(const MatrixXcd& X, int nColsToKeep)
{
    if (nColsToKeep <= 0) {
        throw std::invalid_argument("nColsToKeep must be positive.");
    }

    MatrixXcd Y = MatrixXcd::Zero(X.rows(), nColsToKeep);

    int colsToCopy = std::min(static_cast<int>(X.cols()), nColsToKeep);

    Y.leftCols(colsToCopy) = X.leftCols(colsToCopy);

    return Y;
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
static Vector3d dqn2abc_at_time(const MatrixXcd& X, int row0, double theta)
{
    if (row0 < 0 || row0 + 2 >= X.rows()) {
        throw std::runtime_error("dqn2abc: group is outside Y.");
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
    const int ncols = static_cast<int>(X.cols());

    for (int i = 0; i < ncols; ++i) {
        if (i == 0) {
            Vector3cd col0;
            col0 << X(row0, 0), X(row0 + 1, 0), X(row0 + 2, 0);
            Xabc += (Ssa * col0).real();
        }
        else {
            const int h = i;
            const double th = h * theta;

            const std::complex<double> Xp = X(row0, i);
            const std::complex<double> Xn = X(row0 + 1, i);
            const std::complex<double> Xz = X(row0 + 2, i);

            const double mag_p = std::abs(Xp);
            const double ang_p = std::arg(Xp);

            const double mag_n = std::abs(Xn);
            const double ang_n = std::arg(Xn);

            const double mag_z = std::abs(Xz);
            const double ang_z = std::arg(Xz);

            Vector3d abc3p;
            abc3p <<
                mag_p * std::sin(th + ang_p),
                mag_p * std::sin(th + ang_p - 2.0 * M_PI / 3.0),
                mag_p * std::sin(th + ang_p + 2.0 * M_PI / 3.0);

            Vector3d abc3n;
            abc3n <<
                mag_n * std::sin(th + ang_n),
                mag_n * std::sin(th + ang_n + 2.0 * M_PI / 3.0),
                mag_n * std::sin(th + ang_n - 2.0 * M_PI / 3.0);

            Vector3d abc3z;
            abc3z <<
                mag_z * std::sin(th + ang_z),
                mag_z * std::sin(th + ang_z),
                mag_z * std::sin(th + ang_z);

            Xabc += abc3p + abc3n + abc3z;
        }
    }

    return Xabc;
}

static Vector3d dqn2abc_at_time(const MatrixXcd& Xdcpnz_c, double theta)
{
    if (Xdcpnz_c.rows() != 3) {
        throw std::runtime_error("Xdcpnz_c must have 3 rows.");
    }
    return dqn2abc_at_time(Xdcpnz_c, 0, theta);
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
void dqn2abc_group_into(const MatrixXcd& Y, int group, double theta, Vector3d& out)
{
    if (group < 0)
        throw std::runtime_error("dqn2abc: negative group index.");
    out = dqn2abc_at_time(Y, 3 * group, theta);
}

std::vector<Vector3d> dqn2abc_groups_at_time(const MatrixXcd& Y, double theta,
    const std::vector<int>& groups)
{
    if (Y.rows() == 0) {
        return {};
    }

    if (groups.empty()) {
        if (Y.rows() % 3 != 0) {
            throw std::runtime_error("Y row count must be a multiple of 3.");
        }
        const int nGroups = static_cast<int>(Y.rows() / 3);
        std::vector<Vector3d> out(nGroups);
        for (int g = 0; g < nGroups; ++g)
            out[g] = dqn2abc_at_time(Y, 3 * g, theta);
        return out;
    }

    std::vector<Vector3d> out(groups.size());
    for (size_t i = 0; i < groups.size(); ++i)
        out[i] = dqn2abc_at_time(Y, 3 * groups[i], theta);
    return out;
}

std::vector<Vector3d> dqn2abc_groups_at_time(const MatrixXcd& Y, double theta)
{
    static const std::vector<int> all;
    return dqn2abc_groups_at_time(Y, theta, all);
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
ABCResult simulate_dqn2abc(const MatrixXcd& Xdcpnz_c,
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
        const double theta = 2.0 * M_PI * freq_hz * t;

        res.t[k] = t;
        res.Xabc.row(k) = dqn2abc_at_time(Xdcpnz_c, theta).transpose();
    }

    return res;
}