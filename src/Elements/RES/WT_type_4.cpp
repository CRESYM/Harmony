/**
 * @file WT_type_4.cpp
 * @brief Implementation of Type 4 full-converter wind turbine model.
 */
#include "WT_type_4.h"

#include "core/Constants.h"

#include <cmath>
#include <stdexcept>

WTtype4::WTtype4(const string& symbol, const std::string& location, const vector<double>& parameters)
	: RES_base(symbol, location) {
	if (parameters.size() != 14) {
		throw std::invalid_argument("WT type-4 requires exactly 14 parameters, but got " + std::to_string(parameters.size()));
	}
	Vm = parameters[0];
	f1 = parameters[1];
	Pwt = parameters[2];
	Qwt = parameters[3];
	Vdc = parameters[4];
	Kp_pll = parameters[5];
	Ki_pll = parameters[6];
	Kpi = parameters[7];
	Kii = parameters[8];
	Tdelay = parameters[9];
	wn = parameters[10];
	zeta = parameters[11];
	Rf = parameters[12];
	Lf = parameters[13];

	double wg = 2 * M_PI * f1;
	double Vd = Vm * sqrt(2.0 / 3.0);
	double Vq = 0.0;
	if (!(std::abs(Vd) > 0.0) || !(std::abs(Vdc) > 0.0)) {
		throw std::invalid_argument("WT type-4 requires nonzero PCC and DC voltages.");
	}
	// Generation convention: current into the converter is positive.
	double Id_ref = -Pwt / (1.5 * Vd);
	double Iq_ref = -Qwt / (1.5 * Vd);
	double Md0 = (Vd - Rf * Id_ref - wg * Lf * Iq_ref) / Vdc;
	double Mq0 = (Vq - Rf * Iq_ref + wg * Lf * Id_ref) / Vdc;

	RCP<const Basic> Rf_b = real_double(Rf);
	RCP<const Basic> Lf_b = real_double(Lf);
	RCP<const Basic> wg_b = real_double(wg);
	RCP<const Basic> Vdc_b = real_double(Vdc);
	RCP<const Basic> Kp_PLL_b = real_double(Kp_pll);
	RCP<const Basic> Ki_PLL_b = real_double(Ki_pll);
	RCP<const Basic> Id_ref_b = real_double(Id_ref);
	RCP<const Basic> Iq_ref_b = real_double(Iq_ref);
	RCP<const Basic> Md0_b = real_double(Md0);
	RCP<const Basic> Mq0_b = real_double(Mq0);
	RCP<const Basic> wn_b = real_double(wn);
	RCP<const Basic> zeta_b = real_double(zeta);
	RCP<const Basic> Td_b = real_double(Tdelay);
	RCP<const Basic> Kpi_b = real_double(Kpi);
	RCP<const Basic> Kii_b = real_double(Kii);
	RCP<const Basic> Vd_b = real_double(Vd);

	// Filter
	DenseMatrix Yout = createZeroMatrix(2, 2);
	RCP<const Basic> Zf = add(Rf_b, mul(Lf_b, s)); // Zf = Rf + s * Lf
	RCP<const Basic> Lf_wg = mul(wg_b, Lf_b);
	RCP<const Basic> denom = add(mul(Zf, Zf), mul(Lf_wg, Lf_wg));
	Yout.set(0, 0, div(Zf, denom));
	Yout.set(0, 1, div(Lf_wg, denom));
	Yout.set(1, 0, neg(div(Lf_wg, denom)));
	Yout.set(1, 1, div(Zf, denom));

	DenseMatrix Gid = createZeroMatrix(2, 2);
	mul_dense_scalar(Yout, neg(Vdc_b), Gid);

	// PLL: H = Gc / (s + Vd Gc), Vd peak volts, gains in 1/V
	RCP<const Basic> Gc_PLL = add(Kp_PLL_b, div(Ki_PLL_b, s));
	RCP<const Basic> H_PLL = div(Gc_PLL, add(s, mul(Vd_b, Gc_PLL)));
	DenseMatrix Hi_PLL = createZeroMatrix(2, 2);
	Hi_PLL.set(0, 1, mul(Iq_ref_b, H_PLL));
	Hi_PLL.set(1, 1, neg(mul(Id_ref_b, H_PLL)));
	DenseMatrix Hd_PLL = createZeroMatrix(2, 2);
	Hd_PLL.set(0, 1, mul(neg(Mq0_b), H_PLL));
	Hd_PLL.set(1, 1, mul(Md0_b, H_PLL));

	RCP<const Basic> TF = div(mul(wn_b, wn_b), add(add(mul(s, s), mul(wn_b, wn_b)), mul(real_double(2), mul(zeta_b, mul(wn_b, s)))));
	DenseMatrix Gmf = createZeroMatrix(2, 2);
	Gmf.set(0, 0, TF); Gmf.set(1, 1, TF);

	RCP<const Basic> pade_delay = mul(Td_b, mul(real_double(0.5), s));
	RCP<const Basic> Td = div(sub(one, pade_delay), add(one, pade_delay));
	DenseMatrix Gdel = createZeroMatrix(2, 2);
	Gdel.set(0, 0, Td); Gdel.set(1, 1, Td);

	// Current PI in ohms, then duty/A: Gpi = (Kpi + Kii/s)/Vdc
	DenseMatrix Gcc = createZeroMatrix(2, 2);
	RCP<const Basic> gpi = div(add(Kpi_b, div(Kii_b, s)), Vdc_b);
	RCP<const Basic> coeff = neg(gpi);
	RCP<const Basic> coeff2 = div(Lf_wg, mul(Vdc_b, Vdc_b));
	Gcc.set(0, 0, coeff);
	Gcc.set(1, 1, coeff);
	Gcc.set(0, 1, neg(coeff2));
	Gcc.set(1, 0, coeff2);

	DenseMatrix Imat = createZeroMatrix(2, 2);
	Imat.set(0, 0, one); Imat.set(1, 1, one);
	DenseMatrix dummy = createZeroMatrix(2, 2);
	mul_dense_dense(Gdel, Gid, dummy);
	mul_dense_dense(dummy, Gcc, dummy);
	mul_dense_dense(dummy, Gmf, dummy);
	add_dense_dense(Imat, dummy, dummy); // A = I + Gdel Gid Gcc Gmf
	inverse_LU(dummy, dummy); // A^{-1}

	DenseMatrix dummy2 = createZeroMatrix(2, 2);
	mul_dense_dense(Gcc, Hi_PLL, dummy2); // Gcc * Hi
	add_dense_dense(Hd_PLL, dummy2, dummy2); // Hd + Gcc Hi
	mul_dense_dense(Gdel, dummy2, dummy2);
	mul_dense_dense(Gid, dummy2, dummy2);
	mul_dense_dense(dummy2, Gmf, dummy2);
	add_dense_dense(Yout, dummy2, dummy2); // B = Yout + Gid Gdel (Hd + Gcc Hi) Gmf

	Y_matrix.resize(2, 2);
	mul_dense_dense(dummy2, dummy, Y_matrix); // Y = B A^{-1}
}
