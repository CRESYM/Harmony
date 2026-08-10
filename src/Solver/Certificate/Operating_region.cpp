/**
 * @file Operating_region.cpp
 */
#include "Operating_region.h"

#include "../../Elements/Converter/MMC.h"

#include <cmath>
#include <iostream>

namespace {

OperatingRegionSample evaluatePoint(
	MMC& mmc,
	double P,
	double Q,
	double Vm,
	double Vdc,
	const CertificateSpec& spec)
{
	OperatingRegionSample s;
	s.P = P;
	s.Q = Q;
	try {
		mmc.update_MMC(Vm, 0.0, P, Q, Vdc, P);
		mmc.solveEquilibrium();
		mmc.computeABCD();
		const CertificateSweep sw = sweepDeviceCertificate(mmc, spec);
		s.margin = sw.worst_passivity;
		s.phase_deg = sw.worst_phase_deg;
		s.certified = sweepPassesSpec(sw, spec);
	}
	catch (const std::exception& ex) {
		s.certified = false;
		s.margin = -1.0;
		s.note = ex.what();
	}
	catch (...) {
		s.certified = false;
		s.margin = -1.0;
		s.note = "equilibrium_failed";
	}
	return s;
}

} // namespace

OperatingRegionResult certifyOperatingRegion(
	MMC& mmc,
	const OperatingRegionGrid& grid,
	const CertificateSpec& spec,
	const std::pair<double, double>* opf_PQ)
{
	OperatingRegionResult out;
	if (opf_PQ) {
		out.P_opf = opf_PQ->first;
		out.Q_opf = opf_PQ->second;
	}

	const int nP = std::max(grid.n_P, 1);
	const int nQ = std::max(grid.n_Q, 1);
	out.samples.reserve(static_cast<size_t>(nP * nQ));

	for (int i = 0; i < nP; ++i) {
		const double P = (nP == 1)
			? 0.5 * (grid.P_min + grid.P_max)
			: grid.P_min + (grid.P_max - grid.P_min) * i / (nP - 1);
		for (int j = 0; j < nQ; ++j) {
			const double Q = (nQ == 1)
				? 0.5 * (grid.Q_min + grid.Q_max)
				: grid.Q_min + (grid.Q_max - grid.Q_min) * j / (nQ - 1);
			auto s = evaluatePoint(mmc, P, Q, grid.Vm, grid.Vdc, spec);
			if (s.certified)
				++out.n_certified;
			out.samples.push_back(std::move(s));
		}
	}

	if (opf_PQ) {
		auto s_opf = evaluatePoint(mmc, opf_PQ->first, opf_PQ->second, grid.Vm, grid.Vdc, spec);
		s_opf.note = "opf_setpoint";
		out.opf_point_certified = s_opf.certified;
		out.samples.push_back(std::move(s_opf));
		if (s_opf.certified)
			++out.n_certified;
	}
	return out;
}

OperatingRegionResult certifyOperatingPoints(
	MMC& mmc,
	const std::vector<std::pair<double, double>>& points,
	double Vm,
	double Vdc,
	const CertificateSpec& spec)
{
	OperatingRegionResult out;
	out.samples.reserve(points.size());
	for (const auto& pq : points) {
		auto s = evaluatePoint(mmc, pq.first, pq.second, Vm, Vdc, spec);
		if (s.certified)
			++out.n_certified;
		out.samples.push_back(std::move(s));
	}
	return out;
}

void reportOperatingRegion(const OperatingRegionResult& result, bool plotting_enabled)
{
	std::cout << "[OperatingRegion] certified " << result.n_certified
		<< " / " << result.samples.size() << " set-points";
	if (result.P_opf != 0.0 || result.Q_opf != 0.0) {
		std::cout << " | OPF (P,Q)=(" << result.P_opf / 1e6 << " MW, "
			<< result.Q_opf / 1e6 << " MVAr) "
			<< (result.opf_point_certified ? "CERTIFIED" : "NOT CERTIFIED");
	}
	std::cout << "\n";
	if (plotting_enabled)
		plot_certificate_operating_region(result.samples, "Certified (P,Q) region");
}
