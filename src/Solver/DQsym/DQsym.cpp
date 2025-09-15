#include "DQsym.h"
#include "../Helper_Functions/Helper_Functions.h"

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
 * @note Method (Brief):
 * 1. Zero-pad inputs to harmonic order N and transform to the pnz basis:
 *    X_pnz = S^{-1}*X_abc, Y_pnz = S^{-1}*Y_abc.
 * 2. Convolve harmonics (m, n) to find components at k = m±n using
 *    trigonometric identities. DC-DC and DC-harmonic products are handled explicitly.
 * 3. Assemble the resulting phasors for each harmonic k as `Cs + j*Cc`
 *    (where Cs is sine, Cc is cosine) and transform back to the ABC basis:
 *    Z_abc = S * Z_pnz.
 *
 * @param x_coef1_in A 3x(Nx+1) complex matrix representing the dynamic-phasor
 *                   coefficients of signal x(t) up to harmonic Nx.
 * @param y_coef1_in A 3x(Ny+1) complex matrix representing the dynamic-phasor
 *                   coefficients of signal y(t) up to harmonic Ny.
 * @param N The maximum harmonic order to consider for each input. The resulting
 *          product is computed up to harmonic 2N.
 * @return A 3x(2N+1) complex matrix containing the dynamic-phasor coefficients
 *         of the product z(t) in the ABC basis, from harmonic 0 to 2N.
 */
MatrixXcd DQsym::multiply(const MatrixXcd& x_coef1_in, const MatrixXcd& y_coef1_in) {
    // Validate input rows
    if (x_coef1_in.rows() != 3 || y_coef1_in.rows() != 3) {
        throw std::invalid_argument("Input coefficient matrices must have 3 rows (phases a,b,c)");
    }

    // ------------------------ Persistent transforms ------------------------
    // Create Sas (ABC -> pnz) and its inverse Ssa (pnz -> ABC)
    Matrix3cd Sas;
    const complex<double> a(-0.5, 0.8660254037844386); // 0.866025... = sqrt(3)/2
    const complex<double> a2 = std::conj(a);
    Sas << complex<double>(1, 0), a, a2,
        complex<double>(1, 0), a2, a,
        complex<double>(1, 0), complex<double>(1, 0), complex<double>(1, 0);
    Sas /= 3.0;
    Matrix3cd Ssa = Sas.inverse();

    int N = std::max(x_coef1_in.cols() - 1, y_coef1_in.cols() - 1);

    // ------------------------ Pad inputs to length N -----------------------
    const int max_k = 2 * N; // highest harmonic index in product
    const int outCols = max_k + 1; // columns for 0..2N

    // Zero-pad columns so both inputs explicitly have harmonics 0..N
    int sX = x_coef1_in.cols();
    int sY = y_coef1_in.cols();
    MatrixXcd x_coef1 = MatrixXcd::Zero(3, std::max(sX, N + 1));
    MatrixXcd y_coef1 = MatrixXcd::Zero(3, std::max(sY, N + 1));
    x_coef1.block(0, 0, 3, sX) = x_coef1_in;
    y_coef1.block(0, 0, 3, sY) = y_coef1_in;
    // If either had fewer than N+1 columns, the remaining columns are already zero

    if (x_coef1.cols() < N + 1) x_coef1.conservativeResize(3, N + 1);
    if (y_coef1.cols() < N + 1) y_coef1.conservativeResize(3, N + 1);

    // ------------------------ Transform to pnz basis -----------------------
    // Work in pnz where phases are decoupled for symmetrical components
    MatrixXcd X_pnz = Ssa * x_coef1;   // 3 x (N+1)
    MatrixXcd Y_pnz = Ssa * y_coef1;   // 3 x (N+1)

    // ------------------------ Allocate accumulators ------------------------
    MatrixXcd Z_pnz = MatrixXcd::Zero(3, outCols); // final pnz coefficients
    MatrixXd Cs = MatrixXd::Zero(3, outCols);      // sine coefficients
    MatrixXd Cc = MatrixXd::Zero(3, outCols);      // cosine coefficients
    VectorXd C0 = VectorXd::Zero(3);               // DC term accumulator

    // ------------------------ Harmonic convolution -------------------------
    for (int m = 0; m <= N; ++m) {
        // Decompose X(m): sine/cosine parts held in Re/Im of the coefficient
        VectorXd axs = X_pnz.col(m).real(); // sin parts as in MATLAB
        VectorXd axc = X_pnz.col(m).imag(); // cos parts

        for (int n = 0; n <= N; ++n) {
            VectorXd bxs = Y_pnz.col(n).real();
            VectorXd bxc = Y_pnz.col(n).imag();

            // ---- DC × DC ----
            if (m == 0 && n == 0) {
                C0 += X_pnz.col(0).real().cwiseProduct(Y_pnz.col(0).real());
            }

            // ---- Same-order interaction adds to DC (m==n>0) ----
            if (m == n && m > 0) {
                C0 += 0.5 * (axs.array() * bxs.array() + axc.array() * bxc.array()).matrix();
            }

            // Most of following terms require m>0 && n>0
            if (m > 0 && n > 0) {
                int k_plus = m + n;
                int k_minus = std::abs(m - n);

                // sin(m)*cos(n) -> sine at k = m±n
                if (k_plus <= max_k) {
                    Cs.col(k_plus) += (0.5 * axs.array() * bxc.array()).matrix();
                }
                if (k_minus > 0 && k_minus <= max_k) {
                    int sign_k = sgn(m - n);
                    Cs.col(k_minus) += (0.5 * sign_k * axs.array() * bxc.array()).matrix();
                }

                // cos(m)*sin(n) -> sine at k = m±n (opposite sign for k_minus)
                if (k_plus <= max_k) {
                    Cs.col(k_plus) += (0.5 * axc.array() * bxs.array()).matrix();
                }
                if (k_minus > 0 && k_minus <= max_k) {
                    int sign_k = -sgn(m - n);
                    Cs.col(k_minus) += (0.5 * sign_k * axc.array() * bxs.array()).matrix();
                }

                // cos(m)*cos(n) -> cosine at k = m±n
                if (k_plus <= max_k) {
                    Cc.col(k_plus) += (0.5 * axc.array() * bxc.array()).matrix();
                }
                if (k_minus > 0 && k_minus <= max_k) {
                    Cc.col(k_minus) += (0.5 * axc.array() * bxc.array()).matrix();
                }

                // sin(m)*sin(n) -> cosine at k = m±n (note the sign for k_plus)
                if (k_plus <= max_k) {
                    Cc.col(k_plus) += (-0.5 * axs.array() * bxs.array()).matrix();
                }
                if (k_minus > 0 && k_minus <= max_k) {
                    Cc.col(k_minus) += (0.5 * axs.array() * bxs.array()).matrix();
                }
            }

            // ---- DC × harmonic (x_DC * y_n) ----
            if (m == 0 && n > 0) {
                Cs.col(n) += (x_coef1.col(0).real().array() * bxs.array()).matrix();
                Cc.col(n) += (x_coef1.col(0).real().array() * bxc.array()).matrix();
            }

            // ---- harmonic × DC (x_m * y_DC) ----
            if (n == 0 && m > 0) {
                Cs.col(m) += (y_coef1.col(0).real().array() * axs.array()).matrix();
                Cc.col(m) += (y_coef1.col(0).real().array() * axc.array()).matrix();
            }
        }
    }

    // ------------------------ Assemble pnz phasors -------------------------
    // Column 0 is DC, columns k store harmonic k
    for (int i = 0; i < 3; ++i) {
        Z_pnz(i, 0) = complex<double>(C0(i), 0.0);
    }
    for (int k = 1; k <= max_k; ++k) {
        for (int i = 0; i < 3; ++i) {
            Z_pnz(i, k) = complex<double>(Cs(i, k), Cc(i, k)); // sine + j*cosine
        }
    }

    // ------------------------ Map back to ABC basis ------------------------
    MatrixXcd Zdcpnz_c = Sas * Z_pnz;

    return Zdcpnz_c;
}

/**
 * @brief Performs one integration step for dynamic phasors.
 *
 * This function implements a single step of numerical integration for a set of
 * dynamic phasors using the trapezoidal rule, suitable for state-space models
 * in the dynamic phasor domain.
 *
 * @param[in,out] Zpnz_old A (nrSig*3 x N+1) complex matrix holding the persistent
 *                         integration result from the previous step. It is updated in place.
 * @param[in,out] Xpnz_old A (nrSig*3 x N+1) complex matrix holding the persistent
 *                         input from the previous step. It is updated in place.
 * @param[in] Xpnz A (nrSig*3 x N+1) complex matrix representing the current input phasors.
 * @param[in] N The maximum harmonic order to integrate.
 * @param[in] dt The integration timestep.
 * @param[in] w The fundamental angular frequency.
 * @return A (nrSig*3 x N+1) complex matrix representing the new integration result.
 */
MatrixXcd DQsym::integrate(MatrixXcd& Zpnz_old, MatrixXcd& Xpnz_old, const MatrixXcd& Xpnz,
    double dt, double w)
{
	int N = Xpnz.cols() - 1; // max harmonic order
    int nrSig = Xpnz.rows() / 3;
    if (Xpnz.cols() != N + 1 || Zpnz_old.rows() != nrSig * 3 || Xpnz_old.rows() != nrSig * 3) {
        throw std::invalid_argument("Dimension mismatch in Int_DQN_Mat");
    }

    double dt2 = dt / 2.0;
    MatrixXcd Zpnz = MatrixXcd::Zero(nrSig * 3, N + 1);

    // --- DC component (i = 0)
    Zpnz.col(0) = dt2 * (Xpnz.col(0) + Xpnz_old.col(0)) + Zpnz_old.col(0);
    Xpnz_old.col(0) = Xpnz.col(0);
    Zpnz_old.col(0) = Zpnz.col(0);

    // --- Harmonics (i = 1..N)
    for (int i = 1; i <= N; ++i) {
		double wn = i * w; // Angular frequency of harmonic i
		double wndt2 = wn * dt2; // wn*dt/2

        double A = 2.0 * dt2 / (1.0 + wndt2 * wndt2);
        double B = (1.0 - wndt2 * wndt2) / (1.0 + wndt2 * wndt2);

        const VectorXcd Xin = Xpnz.col(i);
        const VectorXcd Zin_old = Zpnz_old.col(i);

        // separate real and imaginary parts
        VectorXd xr = Xin.real();
        VectorXd xi = Xin.imag();
        VectorXd zr = Zin_old.real();
        VectorXd zi = Zin_old.imag();

        VectorXd zr_new = A * (xr + wndt2 * xi) + (B * zr + A * wn * zi);
        VectorXd zi_new = A * (xi - wndt2 * xr) + (B * zi - A * wn * zr);

        Zpnz.col(i) = zr_new.cast<complex<double>>()
            + complex<double>(0, 1) * zi_new.cast<complex<double>>();

        Zpnz_old.col(i) = Zpnz.col(i);
    }

    return Zpnz;
}