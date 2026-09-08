/**
 * @file WT_type_3.cpp
 * @brief Implementation of Type 3 doubly-fed induction generator (DFIG) wind turbine model.
 */
#include "WT_type_3.h"
#include "core/Constants.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <stdexcept>
#include <utility>

namespace {

struct Wt3Op {
	std::complex<double> Ir;
	std::complex<double> Ic;
	double omega_r;
};

std::pair<std::complex<double>, double> wt3_rotor_from_Ps(
	double Ps, double Qs, double slip, double omega1,
	double Rs, double Lm, double Lls, double Rr_ref, double Llr_ref,
	const std::complex<double>& V1)
{
	const double wr = (1.0 - slip) * omega1;
	const double ws = omega1 - wr;
	const double Ls_full = Lls + Lm;
	const double Lr_full = Llr_ref + Lm;
	const std::complex<double> j(0.0, 1.0);
	const std::complex<double> Ss_abs = -std::complex<double>(Ps, Qs);
	const std::complex<double> Is1 = std::conj(Ss_abs / (1.5 * V1));
	const std::complex<double> Ir = (V1 - (Rs + j * omega1 * Ls_full) * Is1) / (j * omega1 * Lm);
	const std::complex<double> Vr = j * ws * Lm * Is1 + (Rr_ref + j * ws * Lr_full) * Ir;
	const double Protor_out = -(1.5 * Vr * std::conj(Ir)).real();
	return { Ir, Protor_out };
}

Wt3Op solve_wt3_op(
	double P, double Qs, double Qgsc, double slip, double omega1,
	double Rs, double Lm, double Lls, double Rr_ref, double Llr_ref,
	const std::complex<double>& V1)
{
	if (!(Lm > 0.0)) {
		throw std::invalid_argument("WT type-3 magnetizing inductance Lm must be positive.");
	}

	auto mismatch = [&](double Ps) {
		return Ps + wt3_rotor_from_Ps(Ps, Qs, slip, omega1, Rs, Lm, Lls, Rr_ref, Llr_ref, V1).second - P;
	};

	double lo = 0.2 * P;
	double hi = 1.8 * P;
	double m_lo = mismatch(lo);
	double m_hi = mismatch(hi);
	if (m_lo * m_hi > 0.0) {
		const double guess = (std::abs(1.0 - slip) > 1e-12) ? P / (1.0 - slip) : P;
		lo = std::min(guess, P) * 0.2;
		hi = std::max(guess, P) * 1.8;
		if (lo == hi) {
			hi = lo + std::abs(P) * 0.1 + 1.0;
		}
		m_lo = mismatch(lo);
		m_hi = mismatch(hi);
	}
	if (m_lo * m_hi > 0.0) {
		throw std::runtime_error("WT type-3 operating-point solve: no sign change in P bracket.");
	}

	for (int k = 0; k < 80; ++k) {
		const double mid = 0.5 * (lo + hi);
		const double m_mid = mismatch(mid);
		if (m_lo * m_mid <= 0.0) {
			hi = mid;
			m_hi = m_mid;
		}
		else {
			lo = mid;
			m_lo = m_mid;
		}
	}

	const double Ps = 0.5 * (lo + hi);
	const auto rotor = wt3_rotor_from_Ps(Ps, Qs, slip, omega1, Rs, Lm, Lls, Rr_ref, Llr_ref, V1);
	const double Protor_out = rotor.second;
	const std::complex<double> Sgsc(Protor_out, Qgsc);

	Wt3Op op;
	op.Ir = rotor.first;
	op.Ic = std::conj(Sgsc / (1.5 * V1));
	op.omega_r = (1.0 - slip) * omega1;
	return op;
}

} // namespace

WTtype3::WTtype3(const string& symbol, const std::string& location, const vector<double>& parameters)
	: RES_base(symbol, location) {
	if (parameters.size() != 22) {
		throw std::invalid_argument("WT type-3 requires exactly 22 parameters, but got " + std::to_string(parameters.size()));
	}
	V_LL = parameters[0];
	f1 = parameters[1];
	p = parameters[2];
	Qs = parameters[3];
	Qgsc = parameters[4];
	slip = parameters[5];
	Lm = parameters[6];
	Lr = parameters[7];
	Rr = parameters[8];
	Nsr = parameters[9];
	Rs = parameters[10];
	Ls = parameters[11];
	Kp_pll = parameters[12];
	Ki_pll = parameters[13];
	Krp = parameters[14];
	Kri = parameters[15];
	Krd = parameters[16];
	Ksp = parameters[17];
	Ksi = parameters[18];
	Ksd = parameters[19];
	Rf = parameters[20];
	Lf = parameters[21];

	double omega1 = 2.0 * M_PI * f1;
	complex<double> j(0, 1);

	double Ls_r = Ls + Lr * pow(Nsr, 2); // Lsr = Lls + a^2 Llr
	double Rr_p = Rr * pow(Nsr, 2);
	double Llr_ref = Lr * pow(Nsr, 2);

	const double V1_mag = V_LL * std::sqrt(2.0 / 3.0);
	const std::complex<double> V1(V1_mag, 0.0);
	const Wt3Op op = solve_wt3_op(p, Qs, Qgsc, slip, omega1, Rs, Lm, Ls, Rr_p, Llr_ref, V1);
	const std::complex<double> Ir = op.Ir;
	const std::complex<double> Ic = op.Ic;
	const double omega_r = op.omega_r;

	auto Rs_b = real_double(Rs);
	auto Rr_p_b = real_double(Rr_p);
	auto Ls_r_b = real_double(Ls_r);
	auto omega_r_b = real_double(omega_r);
	auto omega1_b = real_double(omega1);
	auto Nsr_2_b = real_double(pow(Nsr, 2));

	RCP<const Basic> s_p = add(s, neg(mul(I, omega1_b))); // s - j*w1
	RCP<const Basic> s_n = add(s, mul(I, omega1_b)); // s + j*w1


	// Filter
	RCP<const Basic> Z_f_s = add(real_double(Rf), mul(s, real_double(Lf))); // Frequency - dependent filter impedance
	auto Z_f_val = Rf + j* Lf * omega1; // Filter impedance at fundamental frequency

	// Sequence slip operators sigma = (s ∓ j*wr)/s
	RCP<const Basic> imag_omega_r = mul(I, omega_r_b);
	RCP<const Basic> sigma_p = div(add(s, neg(imag_omega_r)), s);
	RCP<const Basic> sigma_n = div(add(s, imag_omega_r), s);
	complex<double> sigma_val = j*(omega1 - omega_r) / (j * omega1);

	RCP<const Basic> Ir_p = complex_double(Ir);
	RCP<const Basic> Ir_conj_p = complex_double(conj(Ir));
	RCP<const Basic> Ic_p = complex_double(Ic);
	RCP<const Basic> Ic_conj_p = complex_double(conj(Ic));
	RCP<const Basic> V1_p = complex_double(V1);
	RCP<const Basic> V1_conj_p = complex_double(conj(V1));

	complex<double> Zeq = Ls_r * j * omega1 + Rs + Rr_p / sigma_val; // Equivalent impedance
	complex<double> Vrs = V1 + Ir * Zeq; // Voltage at the rotor side
	complex<double> Vgs = V1 + Ic * Z_f_val; // Grid - side terminal voltage
	RCP<const Basic> Vrs_p = complex_double(Vrs);
	RCP<const Basic> Vrs_conj_p = complex_double(conj(Vrs));
	RCP<const Basic> Vgs_p = complex_double(Vgs);
	RCP<const Basic> Vgs_conj_p = complex_double(conj(Vgs));

	// Make control loops
	RCP<const Basic> Kp_pll_b = real_double(Kp_pll);
	RCP<const Basic> Ki_pll_b = real_double(Ki_pll);
	RCP<const Basic> Krp_b = real_double(Krp);
	RCP<const Basic> Kri_b = real_double(Kri);
	RCP<const Basic> Krd_b = real_double(Krd);
	RCP<const Basic> Ksp_b = real_double(Ksp);
	RCP<const Basic> Ksi_b = real_double(Ksi);
	RCP<const Basic> Ksd_b = real_double(Ksd);

	// PLL in SI: Tpll = V1 (Kp x + Ki) / (x^2 + V1 (Kp x + Ki)), V1 peak volts
	RCP<const Basic> V1_pll_b = real_double(V1_mag);
	RCP<const Basic> Apll_p = add(mul(Kp_pll_b, s_p), Ki_pll_b);
	RCP<const Basic> Apll_n = add(mul(Kp_pll_b, s_n), Ki_pll_b);
	RCP<const Basic> Tpll_p = div(mul(V1_pll_b, Apll_p), add(mul(s_p, s_p), mul(V1_pll_b, Apll_p)));
	RCP<const Basic> Tpll_n = div(mul(V1_pll_b, Apll_n), add(mul(s_n, s_n), mul(V1_pll_b, Apll_n)));
	RCP<const Basic> Krd_ref = mul(Nsr_2_b, Krd_b);

	// GSC current controller transfer functions
	RCP<const Basic> Hsi = add(Ksp_b, div(Ksi_b, s));
	RCP<const Basic> Hsi_p = add(Ksp_b, div(Ksi_b, s_p)); // Positive seq.
	RCP<const Basic> Hsi_n = add(Ksp_b, div(Ksi_b, s_n)); // Negative seq.

	// RSC current controller transfer functions
	RCP<const Basic> Hri = add(Krp_b, div(Kri_b, s));
	RCP<const Basic> Hri_p = add(Krp_b, div(Kri_b, s_p)); // Positive seq.
	RCP<const Basic> Hri_n = add(Krp_b, div(Kri_b, s_n)); // Negative seq.

	// RSC Admittance Calculation
	// Positive sequence RSC admittance
	RCP<const Basic> num1 = add( add(mul(s, Ls_r_b), add(Rs_b, div(Rr_p_b, sigma_p))), 
		mul(Nsr_2_b, div(sub(Hri_p, mul(I, Krd_b)), sigma_p)));

	RCP<const Basic> bracket1 = add(mul(mul(div(Ir_p, V1_p), Nsr_2_b), div(sub(Hri_p, mul(I, Krd_b)), sigma_p)), div(Vrs_p, V1_p));
	RCP<const Basic> denom1 = sub(one, mul(div(Tpll_p, real_double(2)), bracket1));

	RCP<const Basic> Y_RS_p = div(denom1, num1); // RSC admittance in positive sequence

	// negative sequence RSC admittance
	RCP<const Basic> num2 = add( add(mul(s, Ls_r_b), add(Rs_b, div(Rr_p_b, sigma_n))), 
		mul(Nsr_2_b, div(add(Hri_n, mul(I, Krd_b)), sigma_n)));
	RCP<const Basic> bracket2 = add(mul(mul(div(Ir_conj_p, V1_conj_p), Nsr_2_b), div(add(Hri_n, mul(I, Krd_b)), sigma_n)), div(Vrs_conj_p, V1_conj_p));
	RCP<const Basic> denom2 = sub(one, mul(div(Tpll_n, real_double(2)), bracket2));
	
	RCP<const Basic> Y_RS_n = div(denom2, num2); // RSC admittance in negative sequence

	// GSC Admittance Calculation (PLL coupling uses Krd', numerator uses Ksd)
	// positive sequence GSC admittance
	RCP<const Basic> num3 = add(Z_f_s, sub(Hsi_p, mul(I, Ksd_b)));
	RCP<const Basic> bracket3 = add(mul(div(Ic_p, V1_p), sub(Hsi_p, mul(I, Krd_ref))), div(Vgs_p, V1_p));
	RCP<const Basic> denom3 = sub(one, mul(div(Tpll_p, real_double(2)), bracket3));

	RCP<const Basic> Y_GS_p = div(denom3, num3); // GSC admittance in positive sequence

	// negative sequence GSC admittance
	RCP<const Basic> num4 = add(Z_f_s, add(Hsi_n, mul(I, Ksd_b)));
	RCP<const Basic> bracket4 = add(mul(div(Ic_conj_p, V1_conj_p), add(Hsi_n, mul(I, Krd_ref))), div(Vgs_conj_p, V1_conj_p));
	RCP<const Basic> denom4 = sub(one, mul(div(Tpll_n, real_double(2)), bracket4));

	RCP<const Basic> Y_GS_n = div(denom4, num4); // GSC admittance in negative sequence

	// Total Admittance Calculation
	RCP<const Basic> Y_total_p = add(Y_RS_p, Y_GS_p); // Total admittance in positive sequence
	RCP<const Basic> Y_total_n = add(Y_RS_n, Y_GS_n); // Total admittance in negative sequence
	DenseMatrix Y_total = createZeroMatrix(2,2); // Total admittance
	Y_total.set(0, 0, Y_total_p); 
	Y_total.set(1, 1, Y_total_n); 

	DenseMatrix C = createZeroMatrix(2,2); 
	DenseMatrix C_inv = createZeroMatrix(2, 2);
	C.set(0, 0, one); C.set(0, 1, I); C.set(1, 0, one); C.set(1, 1, neg(I)); // Transformation matrix
	C_inv.set(0, 0, real_double(0.5)); C_inv.set(0, 1, real_double(0.5)); 
	C_inv.set(1, 0, mul(neg(I), real_double(0.5))); C_inv.set(1, 1, mul(I, real_double(0.5))); // Transformation matrix

	mul_dense_dense(C_inv, Y_total, Y_total); // Transform total admittance to abc frame
	mul_dense_dense(Y_total, C, Y_total); // Transform back to dq frame

	Y_matrix.resize(2, 2); // Resize the admittance matrix
	Y_matrix.set(0, 0, Y_total.get(0, 0)); // Set the first element
	Y_matrix.set(0, 1, Y_total.get(0, 1)); // Set the second element
	Y_matrix.set(1, 0, Y_total.get(1, 0)); // Set the third element
	Y_matrix.set(1, 1, Y_total.get(1, 1)); // Set the fourth element
}
