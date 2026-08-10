#ifndef CERTIFICATE_SPEC_H
#define CERTIFICATE_SPEC_H

/**
 * @file Certificate_spec.h
 * @brief Shared thresholds and frequency-grid settings for device certificates.
 */

#include <string>

/** @brief Frequency-domain certificate evaluation settings. */
struct CertificateSpec {
	double f_min_Hz = 1.0;
	double f_max_Hz = 1000.0;
	int n_points = 80;
	bool ac_dq_block = true;          ///< Use 2×2 AC dq block of MMC Y when available.

	// Classical / eigenphase (legacy)
	double phase_limit_deg = 90.0;    ///< Max |arg| allowed (deg) for eigen / NR phase.
	double passivity_eps = 0.0;       ///< Require λ_min(Her Y) ≥ eps (excess passivity).
	double shift_delta = 0.0;         ///< Shifted passivity: test Her(Y + δ I).
	bool require_passivity = true;
	bool require_phase = true;        ///< Eigenphase |arg λ(Y)| gate (heuristic).
	bool require_shifted = false;     ///< Also enforce shifted passivity when δ ≠ 0.

	// Local geometric certificates (computed always; enforced if require_*)
	bool require_nr_phase = false;    ///< Numerical-range small-phase (proper phase cert).
	bool require_nr_zero_outside = false; ///< Require 0 ∉ W(Y).
	bool require_small_gain = false;  ///< Require σ_max(Y) ≤ gain_limit.
	bool require_dw = false;          ///< Require NR phase + 0∉W + (optional) small-gain.
	bool require_sector = false;      ///< Loop-shifted sector [sector_alpha, sector_beta].
	double gain_limit = 1.0;          ///< Small-gain threshold on σ_max(Y).
	double sector_alpha = -1.0;       ///< Sector lower bound (loop shift).
	double sector_beta = 1.0;         ///< Sector upper bound (loop shift).
	int nr_theta = 120;               ///< Angular samples for ∂W(Y) / DW shell.
	int dw_sphere = 16;               ///< Extra C^2 sphere density for DW fill.

	std::string label = "device";
};

/** @brief Rectangular (P,Q) sampling grid for operating-region certificates. */
struct OperatingRegionGrid {
	double P_min = 40e6;
	double P_max = 120e6;
	double Q_min = -40e6;
	double Q_max = 40e6;
	int n_P = 7;
	int n_Q = 7;
	double Vm = 100e3;
	double Vdc = 240e3;
};

/** @brief GFM droop search bounds for the tuning assistant. */
struct GfmTuningBounds {
	double Kdroop_P_min = 0.0;
	double Kdroop_P_max = 0.0; ///< If ≤ min, derived from nominal × factors.
	double Kdroop_Q_min = 0.0;
	double Kdroop_Q_max = 0.0;
	int n_P = 7;
	int n_Q = 5;
	double scale_P_lo = 0.25;  ///< Relative to nominal when absolute bounds unset.
	double scale_P_hi = 4.0;
	double scale_Q_lo = 0.25;
	double scale_Q_hi = 4.0;
};

#endif // CERTIFICATE_SPEC_H
