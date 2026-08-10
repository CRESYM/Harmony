#ifndef LOCAL_VS_SYSTEM_H
#define LOCAL_VS_SYSTEM_H

/**
 * @file Local_vs_system.h
 * @brief Workflow 5 — local device certificate vs system H = Y Z_eq benchmark.
 */

#include "Certificate_spec.h"
#include "Stability_certificate.h"

#include <string>

class Element;
class StabilityEstimate;

/** @brief Side-by-side local vs system certificate comparison. */
struct LocalVsSystemResult {
	CertificateSweep local;
	CertificateSweep system_H;
	bool local_pass = false;
	bool system_pass = false;
	bool consistent = false; ///< Both agree (both pass or both fail).
	std::string message;
};

/**
 * @brief Compare decentralized device certificate with Lekić–Beerten H = Y Z_eq.
 *
 * Local uses device Y; system uses λ_min(Her(I+H)). Disagreement highlights
 * cases where local passivity is conservative / optimistic vs interconnection.
 */
LocalVsSystemResult compareLocalVsSystem(
	Element& elem,
	StabilityEstimate& stability,
	const std::string& converter_name,
	const std::string& location,
	const CertificateSpec& spec);

void reportLocalVsSystem(const LocalVsSystemResult& result, bool plotting_enabled = true);

#endif // LOCAL_VS_SYSTEM_H
