/**
 * @file Geometric_certificates.cpp
 * @brief Numerical-range, small-gain, and sampled DW-shell certificates.
 */
#include "Geometric_certificates.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace {

Eigen::MatrixXcd hermitianPart(const Eigen::MatrixXcd& M)
{
	return 0.5 * (M + M.adjoint());
}

double lambdaMaxHermitian(const Eigen::MatrixXcd& H)
{
	Eigen::SelfAdjointEigenSolver<Eigen::MatrixXcd> es(H);
	return es.eigenvalues().maxCoeff();
}

double lambdaMinHermitian(const Eigen::MatrixXcd& H)
{
	Eigen::SelfAdjointEigenSolver<Eigen::MatrixXcd> es(H);
	return es.eigenvalues().minCoeff();
}

DwShellPoint evaluateDwPoint(const Eigen::MatrixXcd& A, const Eigen::VectorXcd& v)
{
	DwShellPoint p;
	const Eigen::VectorXcd Av = A * v;
	const std::complex<double> rayleigh = (v.adjoint() * Av)(0);
	p.re = rayleigh.real();
	p.im = rayleigh.imag();
	p.gain2 = Av.squaredNorm();
	return p;
}

/** @brief Unit vector on C^n from real angles (n=2 specialised; else padded). */
Eigen::VectorXcd unitVector2(double theta, double phi)
{
	Eigen::VectorXcd v(2);
	const double c = std::cos(theta);
	const double s = std::sin(theta);
	v(0) = std::complex<double>(c, 0.0);
	v(1) = std::polar(s, phi);
	return v;
}

} // namespace

double excessPassivity(const Eigen::MatrixXcd& A)
{
	const double nu = lambdaMinHermitian(hermitianPart(A));
	return std::max(0.0, nu);
}

double shortagePassivity(const Eigen::MatrixXcd& A)
{
	const double nu = lambdaMinHermitian(hermitianPart(A));
	return std::max(0.0, -nu);
}

double spectralNorm(const Eigen::MatrixXcd& A)
{
	Eigen::JacobiSVD<Eigen::MatrixXcd> svd(A, Eigen::ComputeThinU | Eigen::ComputeThinV);
	return svd.singularValues()(0);
}

std::vector<std::complex<double>> numericalRangeBoundary(
	const Eigen::MatrixXcd& A,
	int n_theta)
{
	const int N = std::max(n_theta, 8);
	std::vector<std::complex<double>> boundary(static_cast<size_t>(N));
	for (int k = 0; k < N; ++k) {
		const double theta = 2.0 * M_PI * k / N;
		const std::complex<double> e = std::polar(1.0, -theta);
		const Eigen::MatrixXcd H = hermitianPart(e * A);
		Eigen::SelfAdjointEigenSolver<Eigen::MatrixXcd> es(H);
		// Boundary point = Rayleigh quotient of the max-eigendirection of Her(e^{-iθ} A).
		// (e^{iθ} λ_max is only the support value, not the contact point on ∂W.)
		const Eigen::VectorXcd v = es.eigenvectors().col(es.eigenvalues().size() - 1);
		boundary[static_cast<size_t>(k)] = (v.adjoint() * A * v)(0);
	}
	return boundary;
}

NumericalRangeCertificate certifyNumericalRange(
	const Eigen::MatrixXcd& A,
	double phase_limit_deg,
	int n_theta)
{
	NumericalRangeCertificate c;
	c.boundary = numericalRangeBoundary(A, n_theta);
	c.min_real = lambdaMinHermitian(hermitianPart(A));

	// 0 ∈ W(A) ⇔ λ_max(Her(e^{-iθ} A)) ≥ 0 for all θ (no separating half-plane).
	double min_support = std::numeric_limits<double>::infinity();
	const int N = std::max(n_theta, 8);
	for (int k = 0; k < N; ++k) {
		const double theta = 2.0 * M_PI * k / N;
		const std::complex<double> e = std::polar(1.0, -theta);
		const double lam = lambdaMaxHermitian(hermitianPart(e * A));
		min_support = std::min(min_support, lam);
	}
	c.zero_margin = -min_support; // >0 ⇒ exists θ with λ_max < 0 ⇒ 0 ∉ W
	c.contains_zero = (c.zero_margin <= 1e-12);

	if (c.contains_zero) {
		c.max_phase_deg = 180.0;
	} else {
		double max_abs_arg = 0.0;
		for (const auto& z : c.boundary) {
			if (std::abs(z) < 1e-14)
				continue;
			max_abs_arg = std::max(max_abs_arg, std::abs(std::arg(z)));
		}
		c.max_phase_deg = max_abs_arg * 180.0 / M_PI;
	}

	c.pass_zero_outside = !c.contains_zero;
	c.pass_phase = (!c.contains_zero) && (c.max_phase_deg <= phase_limit_deg + 1e-9);
	return c;
}

DwShellCertificate certifyDwShell(
	const Eigen::MatrixXcd& A,
	double phase_limit_deg,
	double gain_limit,
	double excess_eps,
	int n_theta,
	int n_sphere)
{
	DwShellCertificate d;
	d.nr = certifyNumericalRange(A, phase_limit_deg, n_theta);
	d.max_singular = spectralNorm(A);
	d.excess_passivity = excessPassivity(A);
	d.shortage_passivity = shortagePassivity(A);
	d.pass_small_gain = d.max_singular <= gain_limit + 1e-12;
	d.pass_excess = d.nr.min_real >= excess_eps - 1e-12;

	d.xz_min_re = std::numeric_limits<double>::infinity();
	d.xz_max_gain2 = 0.0;

	// NR support eigenvectors → shell points on ∂W.
	const int N = std::max(n_theta, 8);
	for (int k = 0; k < N; ++k) {
		const double theta = 2.0 * M_PI * k / N;
		const std::complex<double> e = std::polar(1.0, -theta);
		const Eigen::MatrixXcd H = hermitianPart(e * A);
		Eigen::SelfAdjointEigenSolver<Eigen::MatrixXcd> es(H);
		const Eigen::VectorXcd v = es.eigenvectors().col(es.eigenvalues().size() - 1);
		const DwShellPoint p = evaluateDwPoint(A, v);
		d.samples.push_back(p);
		d.xz_min_re = std::min(d.xz_min_re, p.re);
		d.xz_max_gain2 = std::max(d.xz_max_gain2, p.gain2);
	}

	// Extra sphere grid for interior shell fill (2×2 specialised; else NR-only).
	if (A.rows() == 2 && A.cols() == 2) {
		const int ns = std::max(n_sphere, 4);
		for (int i = 0; i <= ns; ++i) {
			const double th = 0.5 * M_PI * i / ns; // [0, π/2]
			for (int j = 0; j < 2 * ns; ++j) {
				const double ph = 2.0 * M_PI * j / (2 * ns);
				const DwShellPoint p = evaluateDwPoint(A, unitVector2(th, ph));
				d.samples.push_back(p);
				d.xz_min_re = std::min(d.xz_min_re, p.re);
				d.xz_max_gain2 = std::max(d.xz_max_gain2, p.gain2);
			}
		}
	} else if (A.rows() > 0) {
		// Random-ish deterministic directions via coordinate axes + mixes.
		const int n = static_cast<int>(A.rows());
		for (int i = 0; i < n; ++i) {
			Eigen::VectorXcd v = Eigen::VectorXcd::Zero(n);
			v(i) = 1.0;
			const DwShellPoint p = evaluateDwPoint(A, v);
			d.samples.push_back(p);
			d.xz_min_re = std::min(d.xz_min_re, p.re);
			d.xz_max_gain2 = std::max(d.xz_max_gain2, p.gain2);
		}
		for (int i = 0; i < n; ++i) {
			for (int j = i + 1; j < n; ++j) {
				Eigen::VectorXcd v = Eigen::VectorXcd::Zero(n);
				v(i) = 1.0 / std::sqrt(2.0);
				v(j) = 1.0 / std::sqrt(2.0);
				const DwShellPoint p = evaluateDwPoint(A, v);
				d.samples.push_back(p);
				d.xz_min_re = std::min(d.xz_min_re, p.re);
				d.xz_max_gain2 = std::max(d.xz_max_gain2, p.gain2);
			}
		}
	}

	if (!std::isfinite(d.xz_min_re))
		d.xz_min_re = d.nr.min_real;
	return d;
}

Eigen::MatrixXcd loopShiftedSector(
	const Eigen::MatrixXcd& A,
	double alpha,
	double beta)
{
	const int n = static_cast<int>(A.rows());
	if (n == 0 || A.cols() != n || !(beta > alpha))
		return {};

	const Eigen::MatrixXcd I = Eigen::MatrixXcd::Identity(n, n);
	const Eigen::MatrixXcd num = A - alpha * I;
	const Eigen::MatrixXcd den = beta * I - A;
	Eigen::FullPivLU<Eigen::MatrixXcd> lu(den);
	if (!lu.isInvertible())
		return {};
	return num * lu.inverse();
}

bool certifySector(
	const Eigen::MatrixXcd& A,
	double alpha,
	double beta,
	double eps)
{
	const Eigen::MatrixXcd T = loopShiftedSector(A, alpha, beta);
	if (T.size() == 0)
		return false;
	return lambdaMinHermitian(hermitianPart(T)) >= eps - 1e-12;
}
