#ifndef TUNING_ASSISTANT_H
#define TUNING_ASSISTANT_H

/**
 * @file Tuning_assistant.h
 * @brief Workflow 4 — GFM droop tuning to restore device certificates.
 */

#include "Certificate_spec.h"
#include "Stability_certificate.h"

#include <string>
#include <vector>

class MMC;

/** @brief One candidate in the GFM droop search. */
struct GfmTuningSample {
	double Kdroop_P = 0.0;
	double Kdroop_Q = 0.0;
	bool certified = false;
	double worst_passivity = 0.0;
	double worst_phase_deg = 0.0;
};

/** @brief Result of a GFM tuning search. */
struct GfmTuningResult {
	bool found = false;
	double Kdroop_P_best = 0.0;
	double Kdroop_Q_best = 0.0;
	double Kdroop_P_nominal = 0.0;
	double Kdroop_Q_nominal = 0.0;
	CertificateSweep before;
	CertificateSweep after;
	std::vector<GfmTuningSample> samples;
	std::string message;
};

/**
 * @brief Grid-search GFM P/Q droops; pick the feasible point with largest passivity margin.
 *
 * Restores the MMC to nominal droops if no feasible point is found.
 * Requires GFM enabled (@ref MMC::hasGfm).
 */
GfmTuningResult tuneGfmDroops(
	MMC& mmc,
	const CertificateSpec& spec,
	const GfmTuningBounds& bounds = {});

void reportGfmTuning(const GfmTuningResult& result, bool plotting_enabled = true);

#endif // TUNING_ASSISTANT_H
