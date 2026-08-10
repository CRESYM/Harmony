/**
 * @file example_certificate_figures.cpp
 * @brief All five certificate workflows + paper-style figures.
 *
 * 1. Device gate
 * 2. PnP library
 * 3. Operating (P,Q) region (post-OPF style)
 * 4. GFM droop tuning assistant
 * 5. Local device vs system H = Y Z_eq
 */
#include "Examples.h"

#include "../network.h"
#include "../Bus.h"
#include "../Include_components.h"
#include "../Solver/Certificate/Device_gate.h"
#include "../Solver/Certificate/Local_vs_system.h"
#include "../Solver/Certificate/Operating_region.h"
#include "../Solver/Certificate/PnP_library.h"
#include "../Solver/Certificate/Stability_certificate.h"
#include "../Solver/Certificate/Tuning_assistant.h"
#include "../Solver/Stability_Estimate/Stability_estimate.h"

#include <cmath>
#include <filesystem>
#include <iostream>
#include <utility>
#include <vector>

namespace {

struct GfmPack {
	double omega = 2.0 * M_PI * 50.0;
	double Pac = 100e6;
	double Qac = 0.0;
	double Vm = 100e3;
	double Vdc = 240e3;
	double Larm = 50e-3;
	double Rarm = 1.07;
	double Lf = 60e-3;
	double Rf = 0.535;
	double Kdroop_P = 2.0 * M_PI * 0.5 / 100e6;
	double Kdroop_Q = 0.0;
	double Tf_P = 20e-3;
	double Tf_Q = 20e-3;
};

std::vector<double> converterParams(const GfmPack& p)
{
	return {
		p.omega, p.Pac, p.Qac, 0.0, p.Vm, p.Pac, p.Vdc,
		p.Larm, p.Rarm, 0.01, 50, p.Lf, p.Rf, 0.0
	};
}

std::vector<double> controllerParams(const GfmPack& p)
{
	const double zeta = 0.7;
	const double w0_ccc = 300.0;
	const double Ki_zcc = p.Larm * w0_ccc * w0_ccc;
	const double Kp_zcc = 2.0 * zeta * std::sqrt(Ki_zcc * p.Larm) - p.Rarm;
	return {
		0, 0, 0, 0, 0,
		1, 0, 120, 400, 1, 0,
		1, 0, Kp_zcc, Ki_zcc, 1, p.Pac / (3.0 * p.Vdc),
		0, 0, 0,
		1, 0, p.Kdroop_P, p.Kdroop_Q, 4, p.Tf_P, p.Tf_Q, 0.0, 0.0
	};
}

MMC* makeGfmMmc(const std::string& name, GfmPack p)
{
	const double Reqac = p.Rf + p.Rarm / 2.0;
	const double Leqac = p.Lf + p.Larm / 2.0;
	const double Id = (2.0 / 3.0) * p.Pac / p.Vm;
	const double vMd0 = p.Vm + Reqac * Id;
	const double vMq0 = -p.omega * Leqac * Id;
	p.Kdroop_Q = 0.02 * std::hypot(vMd0, vMq0) / 50e6;
	MMC* mmc = new MMC(name, "AC1_DC1", converterParams(p), controllerParams(p));
	mmc->solveEquilibrium();
	mmc->computeABCD();
	return mmc;
}

CertificateSpec defaultSpec()
{
	CertificateSpec s;
	s.f_min_Hz = 1.0;
	s.f_max_Hz = 1000.0;
	s.n_points = 60;
	s.ac_dq_block = true;
	s.phase_limit_deg = 90.0;
	s.require_passivity = true;
	s.require_phase = true;
	return s;
}

} // namespace

void example_certificate_figures(bool plotting_enabled /*=true*/)
{
	std::cout << "\n=== Certificate workflows (device gate / PnP / region / tune / local-vs-H) ===\n";
	std::filesystem::create_directories("files");

	const CertificateSpec spec = defaultSpec();
	GfmPack base;
	MMC* mmc = makeGfmMmc("MMC_CERT", base);

	// ----- 1. Device gate -----
	const DeviceGateResult gate = evaluateDeviceGate(*mmc, "MMC_CERT", spec);
	reportDeviceGate(gate, plotting_enabled);
	writeCertificateSweepCsv(gate.sweep, "files/certificate_device_sweep.csv");

	// Passiveive branch as a second library entry that should ALLOW.
	std::complex<double> Zac(1.0, 0.1);
	Impedance* line = new Impedance("LINE_PASSIVE", "AC1", 3, Zac);
	CertificateSpec line_spec = spec;
	line_spec.ac_dq_block = false;
	line_spec.n_points = 40;
	line_spec.require_phase = false; // passivity-only gate for lines
	const DeviceGateResult gate_line = evaluateDeviceGate(*line, "LINE_PASSIVE", line_spec);

	// ----- 2. PnP library -----
	PnPCertificateLibrary library;
	library.addFromGate(gate, "MMC_GFM");
	library.addFromGate(gate_line, "Impedance");
	library.saveJson("files/pnp_certificate_library.json");
	library.saveCsv("files/pnp_certificate_library.csv");
	library.printSummary();

	// ----- 3. Operating region (post-OPF style) -----
	OperatingRegionGrid grid;
	grid.P_min = 0.4 * base.Pac;
	grid.P_max = 1.2 * base.Pac;
	grid.Q_min = -0.3 * base.Pac;
	grid.Q_max = 0.3 * base.Pac;
	grid.n_P = 5;
	grid.n_Q = 5;
	grid.Vm = base.Vm;
	grid.Vdc = base.Vdc;
	const std::pair<double, double> opf_pq{ base.Pac, base.Qac };
	CertificateSpec region_spec = spec;
	region_spec.n_points = 30;
	std::cout << "Sampling (P,Q) operating region...\n";
	const OperatingRegionResult region =
		certifyOperatingRegion(*mmc, grid, region_spec, &opf_pq);
	reportOperatingRegion(region, plotting_enabled);
	writeOperatingRegionCsv(region.samples, "files/certificate_operating_region.csv");

	// Restore nominal OP after region sweep.
	mmc->update_MMC(base.Vm, 0.0, base.Pac, base.Qac, base.Vdc, base.Pac);
	mmc->solveEquilibrium();
	mmc->computeABCD();

	// ----- 4. GFM tuning assistant -----
	GfmTuningBounds bounds;
	bounds.n_P = 5;
	bounds.n_Q = 3;
	bounds.scale_P_lo = 0.25;
	bounds.scale_P_hi = 4.0;
	bounds.scale_Q_lo = 0.5;
	bounds.scale_Q_hi = 2.0;
	CertificateSpec tune_spec = spec;
	tune_spec.n_points = 40;
	const GfmTuningResult tune = tuneGfmDroops(*mmc, tune_spec, bounds);
	reportGfmTuning(tune, plotting_enabled);
	writeCertificateSweepCsv(tune.before, "files/certificate_tune_before.csv");
	writeCertificateSweepCsv(tune.after, "files/certificate_tune_after.csv");

	// ----- Network + StabilityEstimate for local vs system -----
	Network net;
	Bus* bus_ac = new Bus("AC1", "AC1", 3);
	Bus* bus_dc = new Bus("DC1", "DC1", 2);
	net.addBus(bus_ac);
	net.addBus(bus_dc);
	net.addElement(mmc);
	net.connectElementToBus(mmc, 1, bus_ac);
	net.connectElementToBus(mmc, 2, bus_dc);
	std::vector<double> Zsrc = { 0.1, 0.1, 0.1 };
	AC_source* src = new AC_source("G1", "AC1", 3, base.Vm, Zsrc);
	net.addElement(src);
	net.connectElementToBus(src, 1, bus_ac);
	net.add_areas();

	StabilityEstimate stability;
	stability.add_areas(&net);

	// ----- 5. Local vs system H -----
	try {
		const LocalVsSystemResult cmp =
			compareLocalVsSystem(*mmc, stability, "MMC_CERT", "AC", spec);
		reportLocalVsSystem(cmp, plotting_enabled);
		writeCertificateSweepCsv(cmp.system_H, "files/certificate_system_H_sweep.csv");
	}
	catch (const std::exception& ex) {
		std::cout << "[LocalVsSystem] skipped: " << ex.what() << "\n";
	}

	if (plotting_enabled) {
		const Eigen::MatrixXcd Y50 = elementAdmittance(*mmc, 50.0, true);
		plot_certificate_dw_slice(Y50, 50.0, "Numerical range W(Y) @ 50 Hz");
		plot_certificate_numerical_range(Y50, 50.0, spec.phase_limit_deg, "Small-phase NR @ 50 Hz");
		plot_certificate_dw_shell(Y50, 50.0, spec, "DW shell xz @ 50 Hz");
		plot_certificate_geometric_sweep(gate.sweep, "Geometric sweep: MMC_CERT");
	}

	delete line;
	std::cout << "Certificate workflows done"
		<< (plotting_enabled ? " (close visualization window to exit)." : ".") << "\n";
}
