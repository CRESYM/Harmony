/**
 * @file Local_vs_system.cpp
 */
#include "Local_vs_system.h"

#include <iostream>
#include <sstream>

LocalVsSystemResult compareLocalVsSystem(
	Element& elem,
	StabilityEstimate& stability,
	const std::string& converter_name,
	const std::string& location,
	const CertificateSpec& spec)
{
	LocalVsSystemResult r;
	r.local = sweepDeviceCertificate(elem, spec);
	r.system_H = sweepReturnRatioCertificate(stability, converter_name, location, spec);

	r.local_pass = true;
	if (spec.require_passivity && !r.local.pass_passivity)
		r.local_pass = false;
	if (spec.require_phase && !r.local.pass_phase)
		r.local_pass = false;

	// System check: Her(I+H) passivity is the primary interconnection proxy.
	r.system_pass = r.system_H.pass_passivity;
	r.consistent = (r.local_pass == r.system_pass);

	std::ostringstream oss;
	oss << "local " << (r.local_pass ? "PASS" : "FAIL")
		<< " (nu=" << r.local.worst_passivity << ")"
		<< " | system I+H " << (r.system_pass ? "PASS" : "FAIL")
		<< " (nu=" << r.system_H.worst_passivity << ")"
		<< " | " << (r.consistent ? "consistent" : "DISAGREE");
	r.message = oss.str();
	return r;
}

void reportLocalVsSystem(const LocalVsSystemResult& result, bool plotting_enabled)
{
	std::cout << "[LocalVsSystem] " << result.message << "\n";
	if (plotting_enabled)
		plot_certificate_local_vs_system(result.local, result.system_H, "Local vs system H");
}
