#include "WT_type_4.h"

#include "../../Constants.h"

WTtype4::WTtype4(const string& symbol, const vector<double>& parameters)
	: RES_base(symbol) {
	// Check if the number of parameters is correct
	if (parameters.size() != 15) {
		throw std::invalid_argument("WT type-4 requires exactly 15 parameters, but got " + std::to_string(parameters.size()));
	}
	// Assigning parameters to member variables
	Vm = parameters[0];					// Grid voltage line-to-line (V)
	f1 = parameters[1];					// Grid frequency (Hz)
	Pwt = parameters[2];				// Wind turbine power (W)
	Vdc = parameters[3];				// DC link voltage (V)
	// PLL Parameters
	Kp_pll = parameters[4];				// PLL proportional gain
	Ki_pll = parameters[5];				// PLL integral gain
	// Current Controller
	Kpi = parameters[6];				// Current controller proportional gain
	Kii = parameters[7];				// Current controller integral gain
	// Delay
	Tdelay = parameters[8];				// Delay in seconds
	// Second-order filter parameters
	wn = parameters[9];				// Natural frequency (rad/s)
	zeta = parameters[10];				// Damping ratio
	// Filter Parameters
	Rf = parameters[11];				// Filter resistance (Ohms)
	Lf = parameters[12];				// Filter inductance (H)
	// Reference values
	Id_ref = parameters[13];			// Reference Id current (A)
	Iq_ref = parameters[14];			// Reference Iq current (A)

	/*
	* Calculation of dq steady-state components:
		Vd = Vdc / 2 * Md0
		Vq = Vdc / 2 * Mq0
		P = 3 / 2 * (Vd * Id + Vq * Iq);
		Q = 3 / 2 * (Vq * Id - Vd * Iq);
	*/

	double Vd = Vm;
	double Vq = 0;
	double Md0 = 1.0 / Vdc * Vd;
	double Mq0 = 2 / Vdc * Vq;

	double wg = 2 * M_PI * f1; // Angular frequency of the grid

	// Conversion to symbolic representation
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
	RCP<const Basic> Vq_b = real_double(Vq);

	// Filter
	DenseMatrix Yout = createZeroMatrix(2, 2);
	RCP<const Basic> Zf = add(Rf_b, mul(Lf_b, s)); // Zf = Rf + s * Lf
	RCP<const Basic> Lf_wg = mul(wg_b, Lf_b); // Zg = Vdc / 2
	RCP<const Basic> denom = add(mul(Zf, Zf), mul(Lf_wg, Lf_wg)); // Denominator for Yout
	Yout.set(0, 0, div(Zf, denom)); // Y11
	Yout.set(0, 1, div(Lf_wg, denom)); // Y12
	Yout.set(1, 0, neg(div(Lf_wg, denom))); // Y21
	Yout.set(1, 1, div(Zf, denom)); // Y22
	
	// Correction
	DenseMatrix Gid = createZeroMatrix(2,2);
	mul_dense_scalar(Yout, neg(Vdc_b), Gid);

	// PLL
	RCP<const Basic> Gc_PLL = add(Kp_PLL_b, div(Ki_PLL_b, s));
	RCP<const Basic> H_PLL = div(Gc_PLL, add(s, mul(Vd_b, Gc_PLL)));
	DenseMatrix Hi_PLL = createZeroMatrix(2, 2);
	Hi_PLL.set(0, 1, mul(Iq_ref_b, H_PLL)); Hi_PLL.set(1, 1, neg(mul(Id_ref_b, H_PLL)));
	DenseMatrix Hd_PLL = createZeroMatrix(2, 2);
	Hd_PLL.set(0, 0, mul((Mq0_b), H_PLL)); Hd_PLL.set(1, 0, mul(neg(Md0_b), H_PLL));

	// Second - order filter
	RCP<const Basic> TF = div(mul(wn_b, wn_b), add(add(mul(s, s), mul(wn_b, wn_b)), mul(real_double(2), mul(zeta_b, mul(wn_b, s))))); // Transfer function of the second-order filter
	DenseMatrix Gmf = createZeroMatrix(2, 2);
	Gmf.set(0, 0, TF); Gmf.set(1, 1, TF); // Gmf = [TF 0; 0 TF]

	// Delay, first-order pade approximation
	RCP<const Basic> pade_delay = mul(Td_b, mul(real_double(0.5), s));
	RCP<const Basic> Td = div(sub(one, pade_delay), add(one, pade_delay)); // Delay transfer function
	DenseMatrix Gdel = createZeroMatrix(2, 2);
	Gdel.set(0, 0, Td); Gdel.set(1, 1, Td); // Gdel = [Td 0; 0 Td]

	// PI current controller
	DenseMatrix Gcc = createZeroMatrix(2, 2);
	RCP<const Basic> coeff = (div(add(Kpi_b, div(Kii_b, s)), Vdc_b));
	RCP<const Basic> coeff2 = div(Lf_wg, Vdc_b);
	Gcc.set(0, 0, coeff); 
	Gcc.set(1, 1, coeff); 
	Gcc.set(0, 1, neg(coeff2)); 
	Gcc.set(1, 0, coeff2); // Gpi = [(Kp + Ki/s) -Lf*wg; Lf*wg (Kp + Ki/s)]/Vdc


	DenseMatrix I = createZeroMatrix(2,2);
	I.set(0, 0, one); I.set(1, 1, one); // Identity matrix I

	DenseMatrix Zdq = createZeroMatrix(2, 2); // Impedance matrix in dq frame

	DenseMatrix dummy = createZeroMatrix(2, 2); // Dummy matrix for calculations
	mul_dense_dense(Gid, Gdel, dummy); // Gid * Gdel
	mul_dense_dense(dummy, Gcc, dummy); 
	mul_dense_dense(dummy, Gmf, dummy); 
	mul_dense_scalar(dummy, neg(one), dummy); // -((Gid*Gdel) * Gcc) * Gmf
	add_dense_dense(I, dummy, dummy); // I - ((Gid*Gdel) * Gcc) * Gmf

	DenseMatrix dummy2 = createZeroMatrix(2, 2); // Another dummy matrix for calculations
	mul_dense_dense(Hi_PLL, Gcc, dummy2); // Hi_PLL * Gcc
	add_dense_dense(Hd_PLL, dummy2, dummy2); // Hd_PLL + Hi_PLL * Gcc
	mul_dense_dense(Gdel, dummy2, dummy2); // Gdel * (Hd_PLL + Hi_PLL * Gcc)
	mul_dense_dense(Gid, dummy2, dummy2); // Gid * (Gdel * (Hd_PLL + Hi_PLL * Gcc))
	mul_dense_dense(dummy2, Gmf, dummy2); // (Gid * (Gdel * (Hd_PLL + Hi_PLL * Gcc))) * Gmf
	add_dense_dense(Yout, dummy2, dummy2); // Yout + (Gid * (Gdel * (Hd_PLL + Hi_PLL * Gcc))) * Gmf
	
	inverse_LU(dummy2, dummy2);
	mul_dense_dense(dummy, dummy2, Zdq);

	Y_matrix.resize(2, 2); // Resize the admittance matrix
	inverse_gauss_jordan(Zdq, Y_matrix);
	//Zdq.inv(Y_matrix);
}