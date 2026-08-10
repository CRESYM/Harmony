#ifndef GEOMETRIC_CERTIFICATES_H
#define GEOMETRIC_CERTIFICATES_H

/**
 * @file Geometric_certificates.h
 * @brief Local geometric certificates on Y(jω):
 *   numerical range (small-phase), small-gain, excess/shortage passivity,
 *   and a sampled Davis–Wielandt (DW) shell with xz-projection margins.
 *
 * These are local sufficient screens (device-level). They are not a full
 * compositional DW/SRG interconnection theorem with network DW shells.
 */

#include "../../Constants.h"

#include <complex>
#include <vector>

/** @brief One point on the Davis–Wielandt shell DW(A) = {(v*Av, ||Av||^2)}. */
struct DwShellPoint {
	double re = 0.0;     ///< Re(v* A v)
	double im = 0.0;     ///< Im(v* A v)
	double gain2 = 0.0;  ///< ||A v||^2
};

/** @brief Numerical-range / small-phase summary for one matrix. */
struct NumericalRangeCertificate {
	std::vector<std::complex<double>> boundary; ///< Support-function samples of ∂W(A)
	double min_real = 0.0;       ///< min Re W(A) = λ_min(Her A)
	double max_phase_deg = 0.0;  ///< max |arg z| over ∂W (180 if 0 ∈ W)
	double zero_margin = 0.0;    ///< >0 ⇒ 0 ∉ W; ≤0 ⇒ 0 ∈ W (approx.)
	bool contains_zero = false;
	bool pass_phase = false;     ///< max_phase ≤ limit (and preferably 0 ∉ W)
	bool pass_zero_outside = false;
};

/** @brief Sampled DW shell + classical projections used as local certificates. */
struct DwShellCertificate {
	std::vector<DwShellPoint> samples;
	NumericalRangeCertificate nr;
	double max_singular = 0.0;   ///< σ_max(A) (small-gain)
	double excess_passivity = 0.0;
	double shortage_passivity = 0.0;
	double xz_min_re = 0.0;      ///< min Re over DW samples (≈ NR min real)
	double xz_max_gain2 = 0.0;   ///< max ||Av||^2 over samples
	bool pass_small_gain = false;
	bool pass_excess = false;
};

/** @brief Hermitian / excess passivity ν = λ_min(Her A); shortage = max(0,-ν). */
double excessPassivity(const Eigen::MatrixXcd& A);
double shortagePassivity(const Eigen::MatrixXcd& A);

/** @brief Operator (spectral) norm σ_max(A). */
double spectralNorm(const Eigen::MatrixXcd& A);

/**
 * @brief Trace ∂W(A) via the support function:
 *   z(θ) = e^{iθ} λ_max(Her(e^{-iθ} A)).
 */
std::vector<std::complex<double>> numericalRangeBoundary(
	const Eigen::MatrixXcd& A,
	int n_theta = 180);

/**
 * @brief Numerical-range small-phase certificate.
 * @param phase_limit_deg  Max allowed |arg| of W(A) (90° ≈ passivity for sectorial sets).
 */
NumericalRangeCertificate certifyNumericalRange(
	const Eigen::MatrixXcd& A,
	double phase_limit_deg = 90.0,
	int n_theta = 180);

/**
 * @brief Sample DW(A) on a dense set of unit vectors (exact NR boundary + sphere grid).
 * @param gain_limit  Require σ_max(A) ≤ gain_limit when used as small-gain gate.
 * @param excess_eps  Require λ_min(Her A) ≥ excess_eps.
 */
DwShellCertificate certifyDwShell(
	const Eigen::MatrixXcd& A,
	double phase_limit_deg = 90.0,
	double gain_limit = 1.0,
	double excess_eps = 0.0,
	int n_theta = 180,
	int n_sphere = 24);

/**
 * @brief Classic loop-shifted (sector) matrix for SISO/MIMO sector [α, β]:
 *   T = (A - α I)(β I - A)^{-1} when invertible; empty matrix on failure.
 *
 * Passivity of T is a sector certificate for A (when defined).
 */
Eigen::MatrixXcd loopShiftedSector(
	const Eigen::MatrixXcd& A,
	double alpha,
	double beta);

/** @brief True if Her(T) ≻ eps for the loop-shifted sector map (when defined). */
bool certifySector(
	const Eigen::MatrixXcd& A,
	double alpha,
	double beta,
	double eps = 0.0);

#endif // GEOMETRIC_CERTIFICATES_H
