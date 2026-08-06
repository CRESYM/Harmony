/**
 * @file Device_gate.cpp
 */
#include "Device_gate.h"

#include <iostream>
#include <sstream>

DeviceGateResult evaluateDeviceGate(
	Element& elem,
	const std::string& device_id,
	const CertificateSpec& spec)
{
	DeviceGateResult r;
	r.device_id = device_id.empty() ? spec.label : device_id;
	r.sweep = sweepDeviceCertificate(elem, spec);
	r.pass_passivity = !spec.require_passivity || r.sweep.pass_passivity;
	r.pass_phase = !spec.require_phase || r.sweep.pass_phase;
	r.pass_shifted = !spec.require_shifted || r.sweep.pass_shifted;
	r.pass_nr_phase = !spec.require_nr_phase || r.sweep.pass_nr_phase;
	r.pass_nr_zero = !spec.require_nr_zero_outside || r.sweep.pass_nr_zero;
	r.pass_small_gain = !spec.require_small_gain || r.sweep.pass_small_gain;
	r.pass_dw = !spec.require_dw || r.sweep.pass_dw;
	r.pass_sector = !spec.require_sector || r.sweep.pass_sector;
	r.allow = sweepPassesSpec(r.sweep, spec);

	std::ostringstream oss;
	if (r.allow) {
		oss << "ALLOW interconnect of '" << r.device_id << "'";
	} else {
		oss << "BLOCK '" << r.device_id << "':";
		if (!r.pass_passivity)
			oss << " passivity(worst nu=" << r.sweep.worst_passivity << ")";
		if (!r.pass_phase)
			oss << " eigenphase(worst=" << r.sweep.worst_phase_deg << "deg)";
		if (!r.pass_shifted)
			oss << " shifted(worst=" << r.sweep.worst_shifted << ")";
		if (!r.pass_nr_phase)
			oss << " NR-phase(worst=" << r.sweep.worst_nr_phase_deg << "deg)";
		if (!r.pass_nr_zero)
			oss << " 0-in-W(margin=" << r.sweep.worst_nr_zero_margin << ")";
		if (!r.pass_small_gain)
			oss << " small-gain(sig=" << r.sweep.worst_small_gain << ")";
		if (!r.pass_dw)
			oss << " DW-lite";
		if (!r.pass_sector)
			oss << " sector";
	}
	r.reason = oss.str();
	return r;
}

void reportDeviceGate(const DeviceGateResult& result, bool plotting_enabled)
{
	std::cout << "[DeviceGate] " << result.reason << "\n";
	if (plotting_enabled) {
		plot_certificate_gate(result.sweep, "Gate: " + result.device_id);
		plot_certificate_passivity(result.sweep, "Gate passivity: " + result.device_id);
		plot_certificate_phase(result.sweep, "Gate phase: " + result.device_id);
		plot_certificate_geometric_sweep(result.sweep, "Gate geometric: " + result.device_id);
	}
}
