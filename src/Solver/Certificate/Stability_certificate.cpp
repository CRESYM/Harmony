/**
 * @file Stability_certificate.cpp
 * @brief Device certificate metrics and paper-style ImPlot figures.
 */
#include "Stability_certificate.h"

#include "../../Elements/Element.h"
#include "../Stability_Estimate/Stability_estimate.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <memory>
#include <stdexcept>

namespace {

std::vector<double> logSpace(double f0, double f1, int n)
{
	std::vector<double> f(static_cast<size_t>(std::max(n, 1)));
	if (n <= 1) {
		f[0] = f0;
		return f;
	}
	const double a = std::log10(f0);
	const double b = std::log10(f1);
	for (int i = 0; i < n; ++i)
		f[static_cast<size_t>(i)] = std::pow(10.0, a + (b - a) * i / (n - 1));
	return f;
}

/** First-frame axis range for log-frequency certificate plots (zoomable afterward). */
void setup_cert_freq_axis_limits(
	const std::vector<double>& freq,
	std::initializer_list<const std::vector<double>*> series,
	const bool applyLimits,
	const ImAxis yAxis = ImAxis_Y1)
{
	if (!applyLimits || freq.empty()) {
		return;
	}

	double f0 = freq.front();
	double f1 = freq.back();
	if (!(f0 > 0.0) || !(f1 > f0)) {
		f0 = std::max(f0, 1e-12);
		f1 = (f1 > f0) ? f1 : f0 * 10.0;
	}

	double yMin = 0.0;
	double yMax = 0.0;
	bool haveY = false;
	for (const std::vector<double>* s : series) {
		if (s == nullptr) {
			continue;
		}
		for (double v : *s) {
			if (!std::isfinite(v)) {
				continue;
			}
			if (!haveY) {
				yMin = yMax = v;
				haveY = true;
			} else {
				yMin = std::min(yMin, v);
				yMax = std::max(yMax, v);
			}
		}
	}

	const double span = haveY ? (yMax - yMin) : 0.0;
	const double pad = span > 0.0 ? 0.08 * span : 1.0;
	const double yLo = haveY ? (yMin - pad) : -1.0;
	const double yHi = haveY ? (yMax + pad) : 1.0;

	ImPlot::SetupAxisLimits(ImAxis_X1, f0, f1, ImPlotCond_Always);
	ImPlot::SetupAxisLimits(yAxis, yLo, yHi, ImPlotCond_Always);
}

void setup_cert_xy_axis_limits(
	const std::vector<double>& x,
	const std::vector<double>& y,
	const bool applyLimits)
{
	if (!applyLimits || x.empty() || y.empty()) {
		return;
	}

	auto minmax = [](const std::vector<double>& v, double& lo, double& hi) {
		lo = hi = v.front();
		for (double t : v) {
			if (!std::isfinite(t)) {
				continue;
			}
			lo = std::min(lo, t);
			hi = std::max(hi, t);
		}
	};

	double xLo = 0.0, xHi = 1.0, yLo = 0.0, yHi = 1.0;
	minmax(x, xLo, xHi);
	minmax(y, yLo, yHi);
	const double xPad = (xHi > xLo) ? 0.08 * (xHi - xLo) : 1.0;
	const double yPad = (yHi > yLo) ? 0.08 * (yHi - yLo) : 1.0;
	ImPlot::SetupAxisLimits(ImAxis_X1, xLo - xPad, xHi + xPad, ImPlotCond_Always);
	ImPlot::SetupAxisLimits(ImAxis_Y1, yLo - yPad, yHi + yPad, ImPlotCond_Always);
}

Eigen::MatrixXcd toEigenY(const std::vector<std::vector<std::complex<double>>>& Yv)
{
	const int n = static_cast<int>(Yv.size());
	Eigen::MatrixXcd Y(n, n);
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
			Y(i, j) = Yv[static_cast<size_t>(i)][static_cast<size_t>(j)];
	return Y;
}

} // namespace

double passivityIndex(const Eigen::MatrixXcd& Y)
{
	const Eigen::MatrixXcd H = 0.5 * (Y + Y.adjoint());
	Eigen::SelfAdjointEigenSolver<Eigen::MatrixXcd> es(H);
	return es.eigenvalues().minCoeff();
}

double shiftedPassivityIndex(const Eigen::MatrixXcd& Y, double delta)
{
	Eigen::MatrixXcd Ys = Y;
	Ys.diagonal().array() += std::complex<double>(delta, 0.0);
	return passivityIndex(Ys);
}

double dwHermitianMargin(const Eigen::MatrixXcd& Y)
{
	return passivityIndex(Y);
}

double maxEigenPhaseDeg(const Eigen::MatrixXcd& Y)
{
	Eigen::ComplexEigenSolver<Eigen::MatrixXcd> es(Y);
	double max_abs_arg = 0.0;
	for (int i = 0; i < es.eigenvalues().size(); ++i) {
		const auto lam = es.eigenvalues()(i);
		if (std::abs(lam) < 1e-18)
			continue;
		max_abs_arg = std::max(max_abs_arg, std::abs(std::arg(lam)));
	}
	return max_abs_arg * 180.0 / M_PI;
}

Eigen::MatrixXcd extractAcDqBlock(const Eigen::MatrixXcd& Y)
{
	if (Y.rows() >= 3 && Y.cols() >= 3)
		return Y.block(1, 1, 2, 2);
	if (Y.rows() == 2 && Y.cols() == 2)
		return Y;
	throw std::invalid_argument("extractAcDqBlock: expected 2x2 or 3x3 Y.");
}

Eigen::MatrixXcd elementAdmittance(Element& elem, double freq_Hz, bool ac_dq_block)
{
	Eigen::MatrixXcd Y = toEigenY(elem.compute_y_parameters(freq_Hz));
	if (ac_dq_block)
		Y = extractAcDqBlock(Y);
	return Y;
}

void finalizeCertificateSweep(CertificateSweep& sweep, const CertificateSpec& spec)
{
	sweep.phase_limit_deg = spec.phase_limit_deg;
	sweep.shift_delta = spec.shift_delta;
	sweep.gain_limit = spec.gain_limit;
	sweep.pass_passivity = sweep.worst_passivity >= spec.passivity_eps;
	sweep.pass_phase = sweep.worst_phase_deg <= spec.phase_limit_deg + 1e-9;
	sweep.pass_shifted = sweep.worst_shifted >= spec.passivity_eps;
	sweep.pass_nr_phase = sweep.worst_nr_phase_deg <= spec.phase_limit_deg + 1e-9;
	sweep.pass_nr_zero = sweep.worst_nr_zero_margin > 1e-12;
	sweep.pass_small_gain = sweep.worst_small_gain <= spec.gain_limit + 1e-12;
	// Local DW-lite: 0 outside NR + NR phase within limit (+ optional small-gain if required separately).
	sweep.pass_dw = sweep.pass_nr_zero && sweep.pass_nr_phase;
	sweep.pass_sector = true; // filled per-frequency in sweeps when require_sector; default OK if unused
}

bool sweepPassesSpec(const CertificateSweep& sweep, const CertificateSpec& spec)
{
	if (spec.require_passivity && !sweep.pass_passivity)
		return false;
	if (spec.require_phase && !sweep.pass_phase)
		return false;
	if (spec.require_shifted && !sweep.pass_shifted)
		return false;
	if (spec.require_nr_phase && !sweep.pass_nr_phase)
		return false;
	if (spec.require_nr_zero_outside && !sweep.pass_nr_zero)
		return false;
	if (spec.require_small_gain && !sweep.pass_small_gain)
		return false;
	if (spec.require_dw && !sweep.pass_dw)
		return false;
	if (spec.require_sector && !sweep.pass_sector)
		return false;
	return true;
}

namespace {

void resizeSweepVectors(CertificateSweep& s, int n)
{
	s.passivity_index.resize(n);
	s.shifted_passivity.resize(n);
	s.max_phase_deg.resize(n);
	s.nr_phase_deg.resize(n);
	s.nr_zero_margin.resize(n);
	s.small_gain.resize(n);
	s.shortage_passivity.resize(n);
	s.min_real_eig.resize(n);
	s.dw_margin.resize(n);
	s.worst_passivity = std::numeric_limits<double>::infinity();
	s.worst_shifted = std::numeric_limits<double>::infinity();
	s.worst_phase_deg = 0.0;
	s.worst_nr_phase_deg = 0.0;
	s.worst_nr_zero_margin = std::numeric_limits<double>::infinity();
	s.worst_small_gain = 0.0;
	s.worst_shortage = 0.0;
	s.worst_dw = std::numeric_limits<double>::infinity();
}

void accumulateMatrixMetrics(
	CertificateSweep& s,
	size_t k,
	const Eigen::MatrixXcd& M,
	const CertificateSpec& spec,
	bool* sector_ok)
{
	if (!M.allFinite()) {
		// Non-finite admittance (e.g. after KINSOL convergence failure).
		// Leave all metrics at their worst-case defaults and mark sector failed.
		if (sector_ok) *sector_ok = false;
		return;
	}
	const double nu = passivityIndex(M);
	const double nu_s = shiftedPassivityIndex(M, spec.shift_delta);
	const double ph = maxEigenPhaseDeg(M);
	const NumericalRangeCertificate nr =
		certifyNumericalRange(M, spec.phase_limit_deg, spec.nr_theta);
	const double gain = spectralNorm(M);
	const double shortage = shortagePassivity(M);
	Eigen::ComplexEigenSolver<Eigen::MatrixXcd> es(M);
	const double min_re = es.eigenvalues().real().minCoeff();

	s.passivity_index[k] = nu;
	s.shifted_passivity[k] = nu_s;
	s.max_phase_deg[k] = ph;
	s.nr_phase_deg[k] = nr.max_phase_deg;
	s.nr_zero_margin[k] = nr.zero_margin;
	s.small_gain[k] = gain;
	s.shortage_passivity[k] = shortage;
	s.min_real_eig[k] = min_re;
	s.dw_margin[k] = nr.zero_margin;

	s.worst_passivity = std::min(s.worst_passivity, nu);
	s.worst_shifted = std::min(s.worst_shifted, nu_s);
	s.worst_phase_deg = std::max(s.worst_phase_deg, ph);
	s.worst_nr_phase_deg = std::max(s.worst_nr_phase_deg, nr.max_phase_deg);
	s.worst_nr_zero_margin = std::min(s.worst_nr_zero_margin, nr.zero_margin);
	s.worst_small_gain = std::max(s.worst_small_gain, gain);
	s.worst_shortage = std::max(s.worst_shortage, shortage);
	s.worst_dw = std::min(s.worst_dw, nr.zero_margin);

	if (sector_ok && spec.require_sector) {
		if (!certifySector(M, spec.sector_alpha, spec.sector_beta, spec.passivity_eps))
			*sector_ok = false;
	}
}

} // namespace

CertificateSweep sweepDeviceCertificate(Element& elem, const CertificateSpec& spec)
{
	CertificateSweep s;
	s.freq_Hz = logSpace(spec.f_min_Hz, spec.f_max_Hz, spec.n_points);
	const int n = static_cast<int>(s.freq_Hz.size());
	resizeSweepVectors(s, n);
	bool sector_ok = true;

	for (int k = 0; k < n; ++k) {
		const Eigen::MatrixXcd Y =
			elementAdmittance(elem, s.freq_Hz[static_cast<size_t>(k)], spec.ac_dq_block);
		accumulateMatrixMetrics(s, static_cast<size_t>(k), Y, spec, &sector_ok);
	}
	finalizeCertificateSweep(s, spec);
	s.pass_sector = sector_ok;
	return s;
}

CertificateSweep sweepDeviceCertificate(
	Element& elem,
	double f_min,
	double f_max,
	int n_points,
	bool ac_block,
	double phase_limit_deg)
{
	CertificateSpec spec;
	spec.f_min_Hz = f_min;
	spec.f_max_Hz = f_max;
	spec.n_points = n_points;
	spec.ac_dq_block = ac_block;
	spec.phase_limit_deg = phase_limit_deg;
	return sweepDeviceCertificate(elem, spec);
}

CertificateSweep sweepReturnRatioCertificate(
	StabilityEstimate& stability,
	const std::string& converter_name,
	const std::string& location,
	const CertificateSpec& spec)
{
	CertificateSweep s;
	s.freq_Hz = logSpace(spec.f_min_Hz, spec.f_max_Hz, spec.n_points);
	const int n = static_cast<int>(s.freq_Hz.size());
	resizeSweepVectors(s, n);
	bool sector_ok = true;

	for (int k = 0; k < n; ++k) {
		const Eigen::MatrixXcd H =
			stability.compute_transfer_function(converter_name, location, s.freq_Hz[static_cast<size_t>(k)]);
		const Eigen::MatrixXcd IpH = Eigen::MatrixXcd::Identity(H.rows(), H.cols()) + H;
		// Local-vs-system path: passivity-style metrics on I+H; gain/phase on H.
		accumulateMatrixMetrics(s, static_cast<size_t>(k), IpH, spec, &sector_ok);
		// Overwrite gain/eigenphase with H-based quantities (more meaningful for return ratio).
		s.small_gain[static_cast<size_t>(k)] = spectralNorm(H);
		s.max_phase_deg[static_cast<size_t>(k)] = maxEigenPhaseDeg(H);
		s.worst_small_gain = std::max(s.worst_small_gain, s.small_gain[static_cast<size_t>(k)]);
		s.worst_phase_deg = std::max(s.worst_phase_deg, s.max_phase_deg[static_cast<size_t>(k)]);
	}
	finalizeCertificateSweep(s, spec);
	s.pass_sector = sector_ok;
	return s;
}

CertificateSweep sweepReturnRatioCertificate(
	StabilityEstimate& stability,
	const std::string& converter_name,
	const std::string& location,
	double f_min,
	double f_max,
	int n_points)
{
	CertificateSpec spec;
	spec.f_min_Hz = f_min;
	spec.f_max_Hz = f_max;
	spec.n_points = n_points;
	return sweepReturnRatioCertificate(stability, converter_name, location, spec);
}

void writeCertificateSweepCsv(const CertificateSweep& sweep, const std::string& path)
{
	std::ofstream f(path);
	f << "freq_Hz,passivity_index,shifted_passivity,max_phase_deg,nr_phase_deg,"
	  << "nr_zero_margin,small_gain,shortage_passivity,min_real_eig,dw_margin\n";
	f << std::setprecision(12);
	for (size_t i = 0; i < sweep.freq_Hz.size(); ++i)
		f << sweep.freq_Hz[i] << ',' << sweep.passivity_index[i] << ','
		  << (i < sweep.shifted_passivity.size() ? sweep.shifted_passivity[i] : 0.0) << ','
		  << sweep.max_phase_deg[i] << ','
		  << (i < sweep.nr_phase_deg.size() ? sweep.nr_phase_deg[i] : 0.0) << ','
		  << (i < sweep.nr_zero_margin.size() ? sweep.nr_zero_margin[i] : 0.0) << ','
		  << (i < sweep.small_gain.size() ? sweep.small_gain[i] : 0.0) << ','
		  << (i < sweep.shortage_passivity.size() ? sweep.shortage_passivity[i] : 0.0) << ','
		  << sweep.min_real_eig[i] << ','
		  << (i < sweep.dw_margin.size() ? sweep.dw_margin[i] : sweep.passivity_index[i]) << '\n';
}

void writeOperatingRegionCsv(
	const std::vector<OperatingRegionSample>& samples,
	const std::string& path)
{
	std::ofstream f(path);
	f << "P,Q,certified,margin,phase_deg,note\n";
	f << std::setprecision(12);
	for (const auto& s : samples)
		f << s.P << ',' << s.Q << ',' << (s.certified ? 1 : 0) << ','
		  << s.margin << ',' << s.phase_deg << ',' << s.note << '\n';
}

// -------------------- plots --------------------

void plot_certificate_passivity(const CertificateSweep& sweep, const std::string& title)
{
	const auto applyLimits = std::make_shared<bool>(true);
	add_plot_tab(title, [=]() {
		if (sweep.freq_Hz.empty())
			return;
		const int N = static_cast<int>(sweep.freq_Hz.size());
		if (ImPlot::BeginPlot("Passivity index nu(w) = lam_min(Her Y)", ImVec2(-1, -1))) {
			ImPlot::SetupAxes("Frequency (Hz)", "lam_min(Her Y)");
			ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);
			std::vector<double> zero(N, 0.0);
			if (!sweep.shifted_passivity.empty() && sweep.shift_delta != 0.0) {
				setup_cert_freq_axis_limits(
					sweep.freq_Hz,
					{ &sweep.passivity_index, &sweep.shifted_passivity, &zero },
					*applyLimits);
			} else {
				setup_cert_freq_axis_limits(
					sweep.freq_Hz, { &sweep.passivity_index, &zero }, *applyLimits);
			}
			ImPlot::PlotLine("nu(w)", sweep.freq_Hz.data(), sweep.passivity_index.data(), N);
			if (!sweep.shifted_passivity.empty() && sweep.shift_delta != 0.0)
				ImPlot::PlotLine("shifted", sweep.freq_Hz.data(), sweep.shifted_passivity.data(), N);
			ImPlot::PlotLine("threshold 0", sweep.freq_Hz.data(), zero.data(), N);
			ImPlot::EndPlot();
		}
		ImGui::Text("worst nu = %.4g   %s",
			sweep.worst_passivity,
			sweep.pass_passivity ? "PASS" : "FAIL");
		*applyLimits = false;
	});
}

void plot_certificate_phase(const CertificateSweep& sweep, const std::string& title)
{
	const auto applyLimits = std::make_shared<bool>(true);
	add_plot_tab(title, [=]() {
		if (sweep.freq_Hz.empty())
			return;
		const int N = static_cast<int>(sweep.freq_Hz.size());
		if (ImPlot::BeginPlot("Device phase |arg lam(Y)|", ImVec2(-1, -1))) {
			ImPlot::SetupAxes("Frequency (Hz)", "max |arg lam| (deg)");
			ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);
			std::vector<double> lim(N, sweep.phase_limit_deg);
			setup_cert_freq_axis_limits(
				sweep.freq_Hz, { &sweep.max_phase_deg, &lim }, *applyLimits);
			ImPlot::PlotLine("max |phase|", sweep.freq_Hz.data(), sweep.max_phase_deg.data(), N);
			ImPlot::PlotLine("limit", sweep.freq_Hz.data(), lim.data(), N);
			ImPlot::EndPlot();
		}
		ImGui::Text("worst phase = %.2f deg (limit %.1f)   %s",
			sweep.worst_phase_deg, sweep.phase_limit_deg,
			sweep.pass_phase ? "PASS" : "FAIL");
		*applyLimits = false;
	});
}

void plot_certificate_gate(const CertificateSweep& sweep, const std::string& title)
{
	const auto applyLimits = std::make_shared<bool>(true);
	add_plot_tab(title, [=]() {
		if (sweep.freq_Hz.empty())
			return;
		const int N = static_cast<int>(sweep.freq_Hz.size());
		ImGui::BeginChild(
			"GateLayout",
			ImVec2(0, -ImGui::GetTextLineHeightWithSpacing() * 2.2f),
			ImGuiChildFlags_None,
			ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		const float h = std::max(320.0f, 0.55f * ImGui::GetContentRegionAvail().y);
		if (ImPlot::BeginPlot("Passivity", ImVec2(-1, h))) {
			ImPlot::SetupAxes("Frequency (Hz)", "nu(w)");
			ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);
			std::vector<double> z(N, 0.0);
			setup_cert_freq_axis_limits(
				sweep.freq_Hz, { &sweep.passivity_index, &z }, *applyLimits);
			ImPlot::PlotLine("nu", sweep.freq_Hz.data(), sweep.passivity_index.data(), N);
			ImPlot::PlotLine("0", sweep.freq_Hz.data(), z.data(), N);
			ImPlot::EndPlot();
		}
		if (ImPlot::BeginPlot("Phase", ImVec2(-1, h))) {
			ImPlot::SetupAxes("Frequency (Hz)", "phase (deg)");
			ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);
			std::vector<double> lim(N, sweep.phase_limit_deg);
			setup_cert_freq_axis_limits(
				sweep.freq_Hz, { &sweep.max_phase_deg, &lim }, *applyLimits);
			ImPlot::PlotLine("|phi|", sweep.freq_Hz.data(), sweep.max_phase_deg.data(), N);
			ImPlot::PlotLine("limit", sweep.freq_Hz.data(), lim.data(), N);
			ImPlot::EndPlot();
		}
		ImGui::EndChild();
		const bool ok = sweep.pass_passivity && sweep.pass_phase;
		ImGui::TextColored(
			ok ? ImVec4(0.2f, 0.8f, 0.3f, 1) : ImVec4(0.9f, 0.3f, 0.2f, 1),
			"DEVICE GATE: %s  (passivity %s, phase %s)",
			ok ? "ALLOW INTERCONNECT" : "BLOCK",
			sweep.pass_passivity ? "OK" : "FAIL",
			sweep.pass_phase ? "OK" : "FAIL");
		*applyLimits = false;
	});
}

void plot_certificate_operating_region(
	const std::vector<OperatingRegionSample>& samples,
	const std::string& title)
{
	const auto applyLimits = std::make_shared<bool>(true);
	add_plot_tab(title, [=]() {
		std::vector<double> p_ok, q_ok, p_bad, q_bad;
		for (const auto& s : samples) {
			if (s.certified) {
				p_ok.push_back(s.P / 1e6);
				q_ok.push_back(s.Q / 1e6);
			} else {
				p_bad.push_back(s.P / 1e6);
				q_bad.push_back(s.Q / 1e6);
			}
		}
		if (ImPlot::BeginPlot("Certified (P,Q) region", ImVec2(-1, -1))) {
			ImPlot::SetupAxes("P (MW)", "Q (MVAr)");
			std::vector<double> px = p_ok;
			px.insert(px.end(), p_bad.begin(), p_bad.end());
			std::vector<double> qy = q_ok;
			qy.insert(qy.end(), q_bad.begin(), q_bad.end());
			setup_cert_xy_axis_limits(px, qy, *applyLimits);
			if (!p_ok.empty())
				ImPlot::PlotScatter("certified", p_ok.data(), q_ok.data(), (int)p_ok.size());
			if (!p_bad.empty())
				ImPlot::PlotScatter("rejected", p_bad.data(), q_bad.data(), (int)p_bad.size());
			ImPlot::EndPlot();
		}
		ImGui::Text("certified %zu / %zu set-points", p_ok.size(), samples.size());
		*applyLimits = false;
	});
}

void plot_certificate_local_vs_system(
	const CertificateSweep& local,
	const CertificateSweep& system_H,
	const std::string& title)
{
	const auto applyLimits = std::make_shared<bool>(true);
	add_plot_tab(title, [=]() {
		const int Nl = static_cast<int>(local.freq_Hz.size());
		const int Ns = static_cast<int>(system_H.freq_Hz.size());
		ImGui::BeginChild(
			"LocalVsSys",
			ImVec2(0, -ImGui::GetTextLineHeightWithSpacing() * 2.2f),
			ImGuiChildFlags_None,
			ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		const float h = std::max(320.0f, 0.55f * ImGui::GetContentRegionAvail().y);
		if (ImPlot::BeginPlot("Local device nu(w)", ImVec2(-1, h))) {
			ImPlot::SetupAxes("Frequency (Hz)", "nu_device");
			ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);
			if (Nl > 0) {
				std::vector<double> z(Nl, 0.0);
				setup_cert_freq_axis_limits(
					local.freq_Hz, { &local.passivity_index, &z }, *applyLimits);
				ImPlot::PlotLine("device", local.freq_Hz.data(), local.passivity_index.data(), Nl);
				ImPlot::PlotLine("0", local.freq_Hz.data(), z.data(), Nl);
			}
			ImPlot::EndPlot();
		}
		if (ImPlot::BeginPlot("System lam_min(Her(I+H))", ImVec2(-1, h))) {
			ImPlot::SetupAxes("Frequency (Hz)", "nu_system");
			ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);
			if (Ns > 0) {
				std::vector<double> z(Ns, 0.0);
				setup_cert_freq_axis_limits(
					system_H.freq_Hz, { &system_H.passivity_index, &z }, *applyLimits);
				ImPlot::PlotLine("I+H", system_H.freq_Hz.data(), system_H.passivity_index.data(), Ns);
				ImPlot::PlotLine("0", system_H.freq_Hz.data(), z.data(), Ns);
			}
			ImPlot::EndPlot();
		}
		ImGui::EndChild();
		ImGui::Text("Local passivity %s | System (I+H) %s",
			local.pass_passivity ? "PASS" : "FAIL",
			system_H.pass_passivity ? "PASS" : "FAIL");
		*applyLimits = false;
	});
}

void plot_certificate_tuning_compare(
	const CertificateSweep& before,
	const CertificateSweep& after,
	const std::string& title)
{
	const auto applyLimits = std::make_shared<bool>(true);
	add_plot_tab(title, [=]() {
		const int Nb = static_cast<int>(before.freq_Hz.size());
		const int Na = static_cast<int>(after.freq_Hz.size());
		if (ImPlot::BeginPlot("Passivity before / after tuning", ImVec2(-1, -1))) {
			ImPlot::SetupAxes("Frequency (Hz)", "nu(w)");
			ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);
			if (Nb > 0) {
				std::vector<double> z(Nb, 0.0);
				if (Na > 0) {
					setup_cert_freq_axis_limits(
						before.freq_Hz,
						{ &before.passivity_index, &after.passivity_index, &z },
						*applyLimits);
				} else {
					setup_cert_freq_axis_limits(
						before.freq_Hz, { &before.passivity_index, &z }, *applyLimits);
				}
				ImPlot::PlotLine("before", before.freq_Hz.data(), before.passivity_index.data(), Nb);
			}
			if (Na > 0)
				ImPlot::PlotLine("after", after.freq_Hz.data(), after.passivity_index.data(), Na);
			if (Nb > 0) {
				std::vector<double> z(Nb, 0.0);
				ImPlot::PlotLine("0", before.freq_Hz.data(), z.data(), Nb);
			}
			ImPlot::EndPlot();
		}
		ImGui::Text("before worst nu=%.4g (%s) | after worst nu=%.4g (%s)",
			before.worst_passivity, before.pass_passivity ? "PASS" : "FAIL",
			after.worst_passivity, after.pass_passivity ? "PASS" : "FAIL");
		*applyLimits = false;
	});
}

void plot_certificate_dw_slice(
	const Eigen::MatrixXcd& Y,
	double freq_Hz,
	const std::string& title)
{
	const NumericalRangeCertificate nr = certifyNumericalRange(Y, 90.0, 180);
	std::vector<double> xr, xi;
	xr.reserve(nr.boundary.size());
	xi.reserve(nr.boundary.size());
	for (const auto& z : nr.boundary) {
		xr.push_back(z.real());
		xi.push_back(z.imag());
	}

	const auto applyLimits = std::make_shared<bool>(true);
	add_plot_tab(title, [=]() {
		if (ImPlot::BeginPlot("Numerical range W(Y) (DW xy projection)", ImVec2(-1, -1))) {
			ImPlot::SetupAxes("Re", "Im");
			setup_cert_xy_axis_limits(xr, xi, *applyLimits);
			if (!xr.empty())
				ImPlot::PlotLine("W(Y)", xr.data(), xi.data(), (int)xr.size());
			const double z0 = 0.0, z1 = 0.0;
			ImPlot::PlotScatter("0", &z0, &z1, 1);
			ImPlot::EndPlot();
		}
		ImGui::Text("f = %.4g Hz | min Re W = %.4g | NR phase = %.2f deg | 0 %s W | zero_margin=%.4g",
			freq_Hz, nr.min_real, nr.max_phase_deg,
			nr.contains_zero ? "IN" : "NOT IN",
			nr.zero_margin);
		*applyLimits = false;
	});
}

void plot_certificate_numerical_range(
	const Eigen::MatrixXcd& Y,
	double freq_Hz,
	double phase_limit_deg,
	const std::string& title)
{
	const NumericalRangeCertificate nr = certifyNumericalRange(Y, phase_limit_deg, 180);
	std::vector<double> xr, xi;
	double rmax = 1.0;
	for (const auto& z : nr.boundary) {
		xr.push_back(z.real());
		xi.push_back(z.imag());
		rmax = std::max(rmax, std::abs(z));
	}
	const double ang = phase_limit_deg * M_PI / 180.0;
	const std::vector<double> rx = {0.0, rmax * std::cos(ang)};
	const std::vector<double> iy_pos = {0.0, rmax * std::sin(ang)};
	const std::vector<double> iy_neg = {0.0, -rmax * std::sin(ang)};

	const auto applyLimits = std::make_shared<bool>(true);
	add_plot_tab(title, [=]() {
		if (ImPlot::BeginPlot("Small-phase via W(Y)", ImVec2(-1, -1))) {
			ImPlot::SetupAxes("Re", "Im");
			std::vector<double> xs = xr;
			xs.insert(xs.end(), rx.begin(), rx.end());
			std::vector<double> ys = xi;
			ys.insert(ys.end(), iy_pos.begin(), iy_pos.end());
			ys.insert(ys.end(), iy_neg.begin(), iy_neg.end());
			setup_cert_xy_axis_limits(xs, ys, *applyLimits);
			if (!xr.empty())
				ImPlot::PlotLine("W(Y)", xr.data(), xi.data(), (int)xr.size());
			ImPlot::PlotLine("+phase lim", rx.data(), iy_pos.data(), 2);
			ImPlot::PlotLine("-phase lim", rx.data(), iy_neg.data(), 2);
			const double z0 = 0.0, z1 = 0.0;
			ImPlot::PlotScatter("0", &z0, &z1, 1);
			ImPlot::EndPlot();
		}
		ImGui::Text("NR phase %.2f / limit %.1f  %s | zero_margin %.4g",
			nr.max_phase_deg, phase_limit_deg,
			nr.pass_phase ? "PASS" : "FAIL",
			nr.zero_margin);
		*applyLimits = false;
	});
}

void plot_certificate_dw_shell(
	const Eigen::MatrixXcd& Y,
	double freq_Hz,
	const CertificateSpec& spec,
	const std::string& title)
{
	const DwShellCertificate dw = certifyDwShell(
		Y, spec.phase_limit_deg, spec.gain_limit, spec.passivity_eps,
		spec.nr_theta, spec.dw_sphere);
	std::vector<double> re, g2;
	re.reserve(dw.samples.size());
	g2.reserve(dw.samples.size());
	for (const auto& p : dw.samples) {
		re.push_back(p.re);
		g2.push_back(p.gain2);
	}

	const auto applyLimits = std::make_shared<bool>(true);
	add_plot_tab(title, [=]() {
		if (ImPlot::BeginPlot("DW shell xz projection (Re, ||Yv||^2)", ImVec2(-1, -1))) {
			ImPlot::SetupAxes("Re(v* Y v)", "||Y v||^2");
			setup_cert_xy_axis_limits(re, g2, *applyLimits);
			if (!re.empty())
				ImPlot::PlotScatter("DW samples", re.data(), g2.data(), (int)re.size());
			const double z0 = 0.0, z1 = 0.0;
			ImPlot::PlotScatter("Re=0", &z0, &z1, 1);
			ImPlot::EndPlot();
		}
		ImGui::Text(
			"f=%.4g | excess=%.4g shortage=%.4g | sig_max=%.4g (lim %.4g) %s | NR phase %.2f %s",
			freq_Hz, dw.excess_passivity, dw.shortage_passivity,
			dw.max_singular, spec.gain_limit,
			dw.pass_small_gain ? "OK" : "FAIL",
			dw.nr.max_phase_deg,
			dw.nr.pass_phase ? "OK" : "FAIL");
		*applyLimits = false;
	});
}

void plot_certificate_geometric_sweep(
	const CertificateSweep& sweep,
	const std::string& title)
{
	const auto applyLimits = std::make_shared<bool>(true);
	add_plot_tab(title, [=]() {
		if (sweep.freq_Hz.empty())
			return;
		const int N = static_cast<int>(sweep.freq_Hz.size());
		// Scrollable column: each plot can be taller than the window.
		ImGui::BeginChild(
			"GeometricSweep",
			ImVec2(0, -ImGui::GetTextLineHeightWithSpacing() * 2.2f),
			ImGuiChildFlags_None,
			ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		// Each plot ~full window tall; scroll to see the rest.
		const float h = std::max(320.0f, 0.55f * ImGui::GetContentRegionAvail().y);

		if (ImPlot::BeginPlot("NR phase vs eigenphase", ImVec2(-1, h))) {
			ImPlot::SetupAxes("Frequency (Hz)", "phase (deg)");
			ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);
			std::vector<double> lim(N, sweep.phase_limit_deg);
			setup_cert_freq_axis_limits(
				sweep.freq_Hz,
				{ &sweep.nr_phase_deg, &sweep.max_phase_deg, &lim },
				*applyLimits);
			ImPlot::PlotLine("NR phase", sweep.freq_Hz.data(), sweep.nr_phase_deg.data(), N);
			ImPlot::PlotLine("eigenphase", sweep.freq_Hz.data(), sweep.max_phase_deg.data(), N);
			ImPlot::PlotLine("limit", sweep.freq_Hz.data(), lim.data(), N);
			ImPlot::EndPlot();
		}

		// Dual Y-axis: shortage nu_- is O(0.01) while sig_max is often O(1)+.
		if (ImPlot::BeginPlot("Shortage passivity / small-gain", ImVec2(-1, h))) {
			ImPlot::SetupAxis(ImAxis_X1, "Frequency (Hz)", ImPlotAxisFlags_None);
			ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);
			ImPlot::SetupAxis(ImAxis_Y1, "shortage nu_-", ImPlotAxisFlags_None);
			ImPlot::SetupAxis(ImAxis_Y2, "sig_max", ImPlotAxisFlags_Opposite);
			setup_cert_freq_axis_limits(
				sweep.freq_Hz, { &sweep.shortage_passivity }, *applyLimits, ImAxis_Y1);
			std::vector<double> gl(N, sweep.gain_limit);
			setup_cert_freq_axis_limits(
				sweep.freq_Hz, { &sweep.small_gain, &gl }, *applyLimits, ImAxis_Y2);
			ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
			ImPlot::PlotLine("shortage", sweep.freq_Hz.data(), sweep.shortage_passivity.data(), N);
			ImPlot::SetAxes(ImAxis_X1, ImAxis_Y2);
			ImPlot::PlotLine("sig_max", sweep.freq_Hz.data(), sweep.small_gain.data(), N);
			ImPlot::PlotLine("gain lim", sweep.freq_Hz.data(), gl.data(), N);
			ImPlot::EndPlot();
		}

		if (ImPlot::BeginPlot("NR zero-margin (DW-lite)", ImVec2(-1, h))) {
			ImPlot::SetupAxes("Frequency (Hz)", "zero_margin");
			ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);
			std::vector<double> z(N, 0.0);
			setup_cert_freq_axis_limits(
				sweep.freq_Hz, { &sweep.nr_zero_margin, &z }, *applyLimits);
			ImPlot::PlotLine("margin", sweep.freq_Hz.data(), sweep.nr_zero_margin.data(), N);
			ImPlot::PlotLine("0", sweep.freq_Hz.data(), z.data(), N);
			ImPlot::EndPlot();
		}
		ImGui::EndChild();
		ImGui::Text(
			"NR phase %s (%.2f) | 0-out %s (%.4g) | small-gain %s (%.4g) | DW-lite %s | worst shortage=%.4g",
			sweep.pass_nr_phase ? "PASS" : "FAIL", sweep.worst_nr_phase_deg,
			sweep.pass_nr_zero ? "PASS" : "FAIL", sweep.worst_nr_zero_margin,
			sweep.pass_small_gain ? "PASS" : "FAIL", sweep.worst_small_gain,
			sweep.pass_dw ? "PASS" : "FAIL",
			sweep.worst_shortage);
		*applyLimits = false;
	});
}
