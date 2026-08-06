#ifndef DEVICE_GATE_H
#define DEVICE_GATE_H

/**
 * @file Device_gate.h
 * @brief Workflow 1 — local device gate before interconnect / PnP.
 */

#include "Certificate_spec.h"
#include "Stability_certificate.h"

#include <string>

class Element;

/** @brief Verdict of a device-level certificate gate. */
struct DeviceGateResult {
	std::string device_id;
	CertificateSweep sweep;
	bool allow = false;
	bool pass_passivity = false;
	bool pass_phase = false;
	bool pass_shifted = false;
	bool pass_nr_phase = false;
	bool pass_nr_zero = false;
	bool pass_small_gain = false;
	bool pass_dw = false;
	bool pass_sector = false;
	std::string reason;
};

/**
 * @brief Evaluate passivity / phase / geometric certificates on device Y(jω).
 *
 * ALLOW only if all enabled checks in @p spec pass. Intended as a local
 * plug-and-play gate before interconnecting into an unknown network.
 */
DeviceGateResult evaluateDeviceGate(
	Element& elem,
	const std::string& device_id,
	const CertificateSpec& spec);

/** @brief Register gate plots and print a one-line verdict. */
void reportDeviceGate(const DeviceGateResult& result, bool plotting_enabled = true);

#endif // DEVICE_GATE_H
