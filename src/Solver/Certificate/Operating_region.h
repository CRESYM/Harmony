#ifndef OPERATING_REGION_H
#define OPERATING_REGION_H

/**
 * @file Operating_region.h
 * @brief Workflow 3 — certified (P,Q) region / post-OPF screening.
 */

#include "Certificate_spec.h"
#include "Stability_certificate.h"

#include <string>
#include <utility>
#include <vector>

class MMC;

/** @brief Result of an operating-region certificate sweep. */
struct OperatingRegionResult {
	std::vector<OperatingRegionSample> samples;
	std::size_t n_certified = 0;
	double P_opf = 0.0; ///< Optional OPF setpoint (if provided).
	double Q_opf = 0.0;
	bool opf_point_certified = false;
};

/**
 * @brief Re-equilibrate MMC over a (P,Q) grid and certify each set-point.
 *
 * Post-OPF usage: set @p opf_PQ to the solved converter injection, optionally
 * center the grid around it, then check whether the OPF point is certified.
 */
OperatingRegionResult certifyOperatingRegion(
	MMC& mmc,
	const OperatingRegionGrid& grid,
	const CertificateSpec& spec,
	const std::pair<double, double>* opf_PQ = nullptr);

/**
 * @brief Certify an explicit list of (P,Q) set-points (e.g. OPF candidates).
 */
OperatingRegionResult certifyOperatingPoints(
	MMC& mmc,
	const std::vector<std::pair<double, double>>& points,
	double Vm,
	double Vdc,
	const CertificateSpec& spec);

void reportOperatingRegion(const OperatingRegionResult& result, bool plotting_enabled = true);

#endif // OPERATING_REGION_H
