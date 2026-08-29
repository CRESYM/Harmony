#ifndef STABILITY_CERTIFICATE_H
#define STABILITY_CERTIFICATE_H

/**
 * @file Stability_certificate.h
 * @brief Device-level passivity / phase / DW-style metrics and paper-style plots.
 */

#include "../../Constants.h"
#include "../Helper_Functions/Visualization.h"
#include "Certificate_spec.h"

#include <complex>
#include <string>
#include <vector>

class Element;
class StabilityEstimate;

/** @brief Frequency-domain certificate sweep for one device (or one H). */
struct CertificateSweep {
	std::vector<double> freq_Hz;
	std::vector<double> passivity_index;    ///< λ_min(Her Y) or Her(I+H)
	std::vector<double> shifted_passivity;  ///< λ_min(Her(Y+δI))
	std::vector<double> max_phase_deg;      ///< max |arg λ| (deg)
	std::vector<double> min_real_eig;       ///< min Re λ
	std::vector<double> dw_margin;          ///< Hermitian DW slice margin (= passivity_index)
	bool pass_passivity = false;
	bool pass_phase = false;
	bool pass_shifted = false;
	bool pass_dw = false;
	double worst_passivity = 0.0;
	double worst_shifted = 0.0;
	double worst_phase_deg = 0.0;
	double worst_dw = 0.0;
	double phase_limit_deg = 90.0;
	double shift_delta = 0.0;
};

/** @brief One sample of a (P,Q) operating-region certificate. */
struct OperatingRegionSample {
	double P = 0.0;
	double Q = 0.0;
	bool certified = false;
	double margin = 0.0;
	double phase_deg = 0.0;
	std::string note;
};

/** @brief Hermitian passivity index λ_min((Y+Y*)/2). */
double passivityIndex(const Eigen::MatrixXcd& Y);

/** @brief Shifted passivity λ_min(Her(Y + δ I)). */
double shiftedPassivityIndex(const Eigen::MatrixXcd& Y, double delta);

/**
 * @brief Hermitian DW-slice margin: for H=Her(Y), W(H)=[λmin,λmax] ⊂ ℝ.
 * Returns λmin (positive ⇒ 0 left of W and outside if λmin>0).
 */
double dwHermitianMargin(const Eigen::MatrixXcd& Y);

/** @brief Maximum |arg(λ)| over eigenvalues of Y, in degrees. */
double maxEigenPhaseDeg(const Eigen::MatrixXcd& Y);

/** @brief Extract AC dq 2×2 block from 2×2 or 3×3 Y. */
Eigen::MatrixXcd extractAcDqBlock(const Eigen::MatrixXcd& Y);

/** @brief Numeric Y of an element at f (Hz), optionally AC dq block. */
Eigen::MatrixXcd elementAdmittance(
	Element& elem,
	double freq_Hz,
	bool ac_dq_block = true);

/** @brief Sweep device Y and evaluate certificates per @p spec. */
CertificateSweep sweepDeviceCertificate(
	Element& elem,
	const CertificateSpec& spec);

/** @brief Convenience overload with explicit grid. */
CertificateSweep sweepDeviceCertificate(
	Element& elem,
	double f_min,
	double f_max,
	int n_points,
	bool ac_block = true,
	double phase_limit_deg = 90.0);

/**
 * @brief Sweep H = Y Z_eq and report Her(I+H) passivity-style margins.
 */
CertificateSweep sweepReturnRatioCertificate(
	StabilityEstimate& stability,
	const std::string& converter_name,
	const std::string& location,
	const CertificateSpec& spec);

CertificateSweep sweepReturnRatioCertificate(
	StabilityEstimate& stability,
	const std::string& converter_name,
	const std::string& location,
	double f_min,
	double f_max,
	int n_points);

/** @brief Aggregate pass/fail from a completed sweep + spec. */
void finalizeCertificateSweep(CertificateSweep& sweep, const CertificateSpec& spec);

void writeCertificateSweepCsv(const CertificateSweep& sweep, const std::string& path);
void writeOperatingRegionCsv(
	const std::vector<OperatingRegionSample>& samples,
	const std::string& path);

// ----- Paper-style ImPlot figures -----

void plot_certificate_passivity(
	const CertificateSweep& sweep,
	const std::string& title = "Passivity index");

void plot_certificate_phase(
	const CertificateSweep& sweep,
	const std::string& title = "Device phase certificate");

void plot_certificate_gate(
	const CertificateSweep& sweep,
	const std::string& title = "Device gate certificate");

void plot_certificate_operating_region(
	const std::vector<OperatingRegionSample>& samples,
	const std::string& title = "Certified operating region (P,Q)");

void plot_certificate_local_vs_system(
	const CertificateSweep& local,
	const CertificateSweep& system_H,
	const std::string& title = "Local vs system certificate");

void plot_certificate_tuning_compare(
	const CertificateSweep& before,
	const CertificateSweep& after,
	const std::string& title = "Control tuning: passivity");

void plot_certificate_dw_slice(
	const Eigen::MatrixXcd& Y,
	double freq_Hz,
	const std::string& title = "DW-style Hermitian slice");

#endif // STABILITY_CERTIFICATE_H
