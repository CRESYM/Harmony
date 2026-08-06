/**
 * @file Tuning_assistant.cpp
 */
#include "Tuning_assistant.h"

#include "../../Elements/Converter/MMC.h"

#include <cmath>
#include <iostream>
#include <limits>

GfmTuningResult tuneGfmDroops(
	MMC& mmc,
	const CertificateSpec& spec,
	const GfmTuningBounds& bounds)
{
	GfmTuningResult out;
	if (!mmc.hasGfm()) {
		out.message = "GFM not enabled on MMC; cannot tune droops.";
		return out;
	}

	const auto droop0 = mmc.getGfmDroops();
	out.Kdroop_P_nominal = droop0.first;
	out.Kdroop_Q_nominal = droop0.second;
	out.before = sweepDeviceCertificate(mmc, spec);

	double P_lo = bounds.Kdroop_P_min;
	double P_hi = bounds.Kdroop_P_max;
	double Q_lo = bounds.Kdroop_Q_min;
	double Q_hi = bounds.Kdroop_Q_max;
	if (!(P_hi > P_lo) && std::abs(droop0.first) > 0.0) {
		P_lo = droop0.first * bounds.scale_P_lo;
		P_hi = droop0.first * bounds.scale_P_hi;
		if (P_lo > P_hi)
			std::swap(P_lo, P_hi);
	}
	if (!(Q_hi > Q_lo) && std::abs(droop0.second) > 0.0) {
		Q_lo = droop0.second * bounds.scale_Q_lo;
		Q_hi = droop0.second * bounds.scale_Q_hi;
		if (Q_lo > Q_hi)
			std::swap(Q_lo, Q_hi);
	}
	if (!(P_hi > P_lo)) {
		P_lo = droop0.first;
		P_hi = droop0.first;
	}
	if (!(Q_hi > Q_lo)) {
		Q_lo = droop0.second;
		Q_hi = droop0.second;
	}

	const int nP = std::max(bounds.n_P, 1);
	const int nQ = std::max(bounds.n_Q, 1);
	double best_margin = -std::numeric_limits<double>::infinity();

	for (int i = 0; i < nP; ++i) {
		const double Kp = (nP == 1) ? P_lo : P_lo + (P_hi - P_lo) * i / (nP - 1);
		for (int j = 0; j < nQ; ++j) {
			const double Kq = (nQ == 1) ? Q_lo : Q_lo + (Q_hi - Q_lo) * j / (nQ - 1);
			GfmTuningSample sample;
			sample.Kdroop_P = Kp;
			sample.Kdroop_Q = Kq;
			try {
				mmc.setGfmDroops(Kp, Kq);
				mmc.solveEquilibrium();
				mmc.computeABCD();
				const CertificateSweep sw = sweepDeviceCertificate(mmc, spec);
				sample.worst_passivity = sw.worst_passivity;
				sample.worst_phase_deg = sw.worst_phase_deg;
				sample.certified = sweepPassesSpec(sw, spec);

				if (sample.certified && sample.worst_passivity > best_margin) {
					best_margin = sample.worst_passivity;
					out.found = true;
					out.Kdroop_P_best = Kp;
					out.Kdroop_Q_best = Kq;
					out.after = sw;
				}
			}
			catch (...) {
				sample.certified = false;
				sample.worst_passivity = -1.0;
			}
			out.samples.push_back(sample);
		}
	}

	if (out.found) {
		mmc.setGfmDroops(out.Kdroop_P_best, out.Kdroop_Q_best);
		mmc.solveEquilibrium();
		mmc.computeABCD();
		out.after = sweepDeviceCertificate(mmc, spec);
		out.message = "Found certified droops Kp=" + std::to_string(out.Kdroop_P_best)
			+ " Kq=" + std::to_string(out.Kdroop_Q_best);
	} else {
		mmc.setGfmDroops(out.Kdroop_P_nominal, out.Kdroop_Q_nominal);
		mmc.solveEquilibrium();
		mmc.computeABCD();
		out.after = out.before;
		out.message = "No certified droop pair in search grid; restored nominal.";
	}
	return out;
}

void reportGfmTuning(const GfmTuningResult& result, bool plotting_enabled)
{
	std::cout << "[GfmTuning] " << result.message << "\n";
	std::cout << "  nominal (Kp,Kq)=(" << result.Kdroop_P_nominal << ", "
		<< result.Kdroop_Q_nominal << ")  before nu=" << result.before.worst_passivity
		<< "  after nu=" << result.after.worst_passivity << "\n";
	if (plotting_enabled)
		plot_certificate_tuning_compare(result.before, result.after, "GFM droop tuning");
}
