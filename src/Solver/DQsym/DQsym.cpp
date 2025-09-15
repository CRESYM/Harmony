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
 * @brief Dynamic-phasor (DQ0) integrator per harmonic order.
 *
 * Numerically integrates complex dynamic-phasor signals `Xpnz(k)` over
 * N harmonics using a trapezoidal (Tustin-like) update written in real/imag
 * form for stability. The output `Zpnz(:, n)` holds the integrated phasor
 * at harmonic index `n = 0..N` for each stacked 3-phase signal.
 *
 * @note Signal Layout:
 * Signals are stacked in 3-row blocks per physical channel (e.g., a,b,c or dq0).
 * If there are 'nrSig' channels, `Xpnz` has size (3*nrSig) x (N+1).
 * Column `n` corresponds to harmonic order `n` (with angular freq `wn = n*w`).
 *
 * @note Method:
 * For each harmonic n (1..N), define:
 * - `wn = n * w`
 * - `dt2 = dt / 2`
 * - `xi = wn * dt2`
 * - `A = 2 * dt2 / (1 + xi^2)` (input weight)
 * - `B = (1 - xi^2) / (1 + xi^2)` (previous-state weight)
 * The update is applied separately to real/imag parts for numerical robustness:
 * - `Re{Z(n)} = A*(Re{X(n)} + xi*Im{X(n)}) + B*Re{Z_old(n)} + A*wn*Im{Z_old(n)}`
 * - `Im{Z(n)} = A*(Im{X(n)} - xi*Re{X(n)}) + B*Im{Z_old(n)} - A*wn*Re{Z_old(n)}`
 * For n=0, this reduces to a standard trapezoidal step.
 *
 * @param[in,out] Zpnz_old A (3*nrSig)x(N+1) complex matrix holding the persistent
 *                         integration result from the previous step. It is updated in place.
 * @param[in,out] Xpnz_old A (3*nrSig)x(N+1) complex matrix holding the persistent
 *                         input from the previous step. It is updated in place.
 * @param[in] Xpnz A (3*nrSig)x(N+1) complex matrix of dynamic-phasor inputs.
 *                 Column 1 is n=0, col N+1 is n=N.
 * @param[in] dt The integration time step in seconds.
 * @param[in] w The base electrical angular frequency in rad/s (e.g., 2*pi*50).
 * @return A (3*nrSig)x(N+1) complex matrix of the integrated dynamic-phasor signal.
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


/**
 * @brief Discrete phasor-domain state-space solver with switch-dependent matrix updates.
 * @note This implementation does not include a half-step predictor/corrector.
 *
 * @param Ad Continuous-time state matrix (3-phase stacked).
 * @param Bd Continuous-time input matrix (3-phase stacked).
 * @param Cd Continuous-time output matrix (3-phase stacked).
 * @param Dd Continuous-time feedthrough matrix (3-phase stacked).
 * @param swOnRes Vector of on-state resistances for each switch (1xNs).
 * @param swOffRes Vector of off-state resistances for each switch (1xNs).
 * @param swType Vector indicating switch type (1 for modeled, 0 for no switching).
 * @param brkVec Vector of current switch positions (0=open, 1=closed).
 * @param u Matrix of inputs over time (nu x T).
 * @param xo Vector representing the initial state (nx x 1).
 * @return Matrix of outputs over time (ny x T).
 */
MatrixXcd DQsym::DSSS(const MatrixXcd& Ad, const MatrixXcd& Bd,
    const MatrixXcd& Cd, const MatrixXcd& Dd,
    const VectorXd& swOnRes, const VectorXd& swOffRes,
    const VectorXi& swType, const VectorXi& brkVec,
    const MatrixXcd& u, const VectorXcd& xo)
{
    int T = u.cols();

    // Preallocate
    MatrixXcd x = MatrixXcd::Zero(Ad.rows(), T);
    MatrixXcd y = MatrixXcd::Zero(Cd.rows(), T);

    // ---- Convert to phasor (placeholder, you must implement) ----
    MatrixXcd Adc = Ad, Bdc = Bd, Cdc = Cd, Ddc = Dd;
    convertToPhasor(Ad, Bd, Cd, Dd, Adc, Bdc, Cdc, Ddc);

    // ---- First-call initialization ----
    if (!initialized) {
        nStates = Ad.rows();
        nInputs = Bd.cols();
        nOutputs = Cd.rows();
        nSwitches = (swType.array() == 1).count();

        x_old = MatrixXcd::Zero(nStates, T);
        x_old.col(0) = xo;

        swVec = brkVec;
        swVecOld = swVec;

        yswitch = VectorXcd::Zero(nSwitches);

        Ads = Adc;
        Bds = Bdc;
        Cds = Cdc;
        Dds = Ddc;

        initialized = true;
    }

    // ---- Update switch vector ----
    if ((swType.array() != 0).any()) {
        swVec = brkVec;
    }

    // ---- If switch states changed ----
    if ((swType.array() != 0).any() && (swVecOld.array() != swVec.array()).any()) {
        for (int s = 0; s < nSwitches; ++s) {
            if (swVec(s) == 1) {
                yswitch(s) = complex<double>(1.0 / swOnRes(s), 0);
            }
            else {
                yswitch(s) = complex<double>(1.0 / swOffRes(s), 0);
            }
        }

        VectorXi SwitchChange = (swVecOld.array() != swVec.array()).cast<int>();
        vector<int> idxSwChng;
        for (int i = 0; i < SwitchChange.size(); i++) {
            if (SwitchChange(i) == 1) idxSwChng.push_back(i);
        }

        int nbSwChng = idxSwChng.size();

        VectorXcd DxCol(nOutputs);
        VectorXcd BDcol(nStates);

        for (int k = 0; k < nbSwChng; k++) {
            int nSw = idxSwChng[k];

            complex<double> tmp = Dds(nSw, nSw) * yswitch(nSw);
            complex<double> temp = 1.0 / (1.0 - tmp);
            complex<double> t2 = yswitch(nSw) * temp;

            for (int i = 0; i < nOutputs; i++) {
                DxCol(i) = Dds(i, nSw) * t2;
            }
            DxCol(nSw) = temp;

            t2 = yswitch(nSw);
            for (int i = 0; i < nStates; i++) {
                BDcol(i) = Bds(i, nSw) * t2;
            }

            RowVectorXcd rowC = Cds.row(nSw);
            Cds.row(nSw).setZero();

            RowVectorXcd rowD = Dds.row(nSw);
            Dds.row(nSw).setZero();

            // Update C and D
            for (int i = 0; i < nOutputs; i++) {
                Cds.row(i) += DxCol(i) * rowC;
                Dds.row(i) += DxCol(i) * rowD;
            }

            // Update A and B
            for (int i = 0; i < nStates; i++) {
                Ads.row(i) += BDcol(i) * Cds.row(nSw);
                Bds.row(i) += BDcol(i) * Dds.row(nSw);
            }
        }

        swVecOld = swVec;
    }

    // ---- State and output calculation ----
    x = Ads * x_old + Bds * u;

    // Demodulate / rotate (50 Hz, Ts = 2e-5)
    VectorXcd diagExp(T);
    for (int k = 0; k < T; k++) {
        diagExp(k) = exp(complex<double>(0, -2.0 * M_PI * 50.0 * 2e-5 * k));
    }
    MatrixXcd diagMat = diagExp.asDiagonal();
    MatrixXcd x2 = x * diagMat;

    x_old = x2;
    y = Cds * x2 + Dds * u;

    return y;
}

/**
 * @brief Convert state-space matrices into the phasor/DQ0 domain.
 *
 * Equivalent of MATLAB's convertToPhasor() + transformMatrix().
 * Currently assumes matrices are already discrete (no trapezoidal step).
 *
 * @param A Continuous/discrete state matrix
 * @param B Continuous/discrete input matrix
 * @param C Continuous/discrete output matrix
 * @param D Continuous/discrete feedthrough matrix
 * @param Adc (output) Phasor-domain state matrix
 * @param Bdc (output) Phasor-domain input matrix
 * @param Cdc (output) Phasor-domain output matrix
 * @param Ddc (output) Phasor-domain feedthrough matrix
 */
void DQsym::convertToPhasor(const MatrixXcd& A, const MatrixXcd& B,
    const MatrixXcd& C, const MatrixXcd& D,
    MatrixXcd& Adc, MatrixXcd& Bdc,
    MatrixXcd& Cdc, MatrixXcd& Ddc)
{
    // Sampling time (fixed for now, can be parameterized)
    double dt = 2e-5;

    // Using continuous matrices directly
    MatrixXcd Ad2 = A, Bd2 = B, Cd2 = C, Dd2 = D;

    // Symmetrical components (DQ0) transform
    complex<double> a(-0.5, 0.866);   // e^(j*120°)
    complex<double> a2(-0.5, -0.866); // e^(-j*120°)

    Matrix3cd Sas;
    Sas << 1.0, a, a2,
        1.0, a2, a,
        1.0, 1.0, 1.0;
    Sas /= 3.0;

    // Blockwise transform
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
                // Equivalent of Sas * sub / Sas in MATLAB
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