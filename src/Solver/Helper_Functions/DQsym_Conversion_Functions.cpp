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
MatrixXcd dq_multiply(const MatrixXcd& x_coef1_in, const MatrixXcd& y_coef1_in)
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
Vector3d dqn2abc_at_time(const MatrixXcd& Xdcpnz_c, double theta)
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
                mag_p* std::sin(th + ang_p - 2.0 * M_PI / 3.0),
                mag_p* std::sin(th + ang_p + 2.0 * M_PI / 3.0);

            Vector3d abc3n;
            abc3n <<
                mag_n * std::sin(th + ang_n),
                mag_n* std::sin(th + ang_n + 2.0 * M_PI / 3.0),
                mag_n* std::sin(th + ang_n - 2.0 * M_PI / 3.0);

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
std::vector<Vector3d> dqn2abc_groups_at_time(const MatrixXcd& Y, double theta)
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