/**
 * @file WT_type_3.cpp
 * @brief Implementation of Type 3 doubly-fed induction generator (DFIG) wind turbine model.
 */
#include "WT_type_3.h"
#include "../../Constants.h"

WTtype3::WTtype3(const string& symbol, const std::string& location, const vector<double>& parameters)
	: RES_base(symbol, location) {
	// Check if the number of parameters is correct
	if (parameters.size() != 24) {
		throw std::invalid_argument("WT type-3 requires exactly 24 parameters, but got " + std::to_string(parameters.size()));
	}
	// Assigning parameters to member variables
	nm_rpm = parameters[0];				// Mechanical speed in rpm
	p = static_cast<int>(parameters[1]);// Number of rotor pole pairs
	Lr = parameters[2];					// Rotor inductance (H)
	Rr = parameters[3];					// Rotor resistance (Ohms)
	Nsr = parameters[4];				// Turns ratio (Stator:Rotor)
	Rs = parameters[5];					// Stator resistance (Ohms)
	Ls = parameters[6];					// Stator inductance (H)
	// PLL Parameters
	Kp_pll = parameters[7];				// PLL proportional gain
	Ki_pll = parameters[8];				// PLL integral gain
	V1_mag = parameters[9];				// PCC voltage magnitude (V)
	V1_theta_deg = parameters[10];		// PCC voltage angle (deg)
	// Rotor Side Converter (RSC) Current Controller
	Krp = parameters[11];				// Proportional gain
	Kri = parameters[12];				// Integral gain
	Krd = parameters[13];				// Derivative gain
	Ir_mag = parameters[14];			// Rotor current magnitude (A)
	Ir_theta_deg = parameters[15];		// Current angle (deg)
	// Grid Side Converter (GSC) Current Controller
	Ksp = parameters[16];				// Proportional gain
	Ksi = parameters[17];				// Integral gain
	Ksd = parameters[18];				// Derivative gain
	Ic_mag = parameters[19];			// GSC current magnitude (A)
	Ic_theta_deg = parameters[20];		// GSC current angle (deg)
	// Filter Parameters
	Rf = parameters[21];				// Filter resistance (Ohms)
	Lf = parameters[22];				// Filter inductace (H)
	f1 = parameters[23];				// Grid frequency (Hz)


	// Rotor and Stator Electrical Parameters
	double omega1 = 2.0 * M_PI * f1; // Nominal angular frequency (rad/s)
	double omega_m = 2.0 * M_PI * nm_rpm / 60.0; // Mechanical angular speed (rad / s)
	complex<double> j(0, 1); // Imaginary unit
	
	// Rotor Parameters Referred to the Stator Side
	double Ls_r = Ls + Lr * pow(Nsr, 2); // Equivalent rotor inductance
	double Rr_p = Rr * pow(Nsr, 2); // Equivalent rotor resistance

	// Conversion to symbols
	auto Rs_b = real_double(Rs);
	auto Rr_p_b = real_double(Rr_p);
	auto Ls_r_b = real_double(Ls_r);
	auto omega_m_b = real_double(omega_m);
	auto omega1_b = real_double(omega1);
	auto Nsr_b = real_double(Nsr);
	auto Nsr_2_b = real_double(pow(Nsr, 2));

	RCP<const Basic> s_n = add(s, mul(I, omega1_b)); // Positive sequence operator
	RCP<const Basic> s_p = add(s, neg(mul(I, omega1_b))); // Negative sequence operator


	// Filter
	RCP<const Basic> Z_f_s = add(real_double(Rf), mul(s, real_double(Lf))); // Frequency - dependent filter impedance
	RCP<const Basic> Z_f_omega = add(real_double(Rf), mul(mul(I, omega1_b), real_double(Lf))); // Impedance at fundamental frequency
	auto Z_f_val = Rf + j* Lf * omega1; // Filter impedance at fundamental frequency

	// Sigma Operators (used for positive/negative sequence modeling)
	RCP<const Basic> imag_omega_m = mul(I, omega_m_b); // Imaginary part for omega_m
	RCP<const Basic> sigma_p = div(add(s, neg(imag_omega_m)), s); // Positive sequence sigma
	RCP<const Basic> sigma_n = div(add(s, imag_omega_m), s); // Negative sequence sigma
	complex<double> sigma_val = j*(omega1 - omega_m) / (j * omega1); // Convert to complex

	// Conversions to complex numbers
	double V1_theta_rad = V1_theta_deg * M_PI / 180.0; // Convert angle to radians
	double Ir_theta_rad = Ir_theta_deg * M_PI / 180.0; // Convert angle to radians
	double Ic_theta_rad = Ic_theta_deg * M_PI / 180.0; // Convert angle to radians
	
	complex<double> V1(V1_mag * cos(V1_theta_rad), V1_mag * sin(V1_theta_rad)); // Convert to complex phasor
	complex<double> Ir(Ir_mag * cos(Ir_theta_rad), Ir_mag * sin(Ir_theta_rad)); // Convert to complex phasor
	complex<double> Ic(Ic_mag * cos(Ic_theta_rad), Ic_mag * sin(Ic_theta_rad)); // Convert to complex phasor
	RCP<const Basic> Ir_p = complex_double(Ir); // Complex phasor for Ir
	RCP<const Basic> Ir_conj_p = complex_double(conj(Ir)); // Complex conjugate of Ir
	RCP<const Basic> Ic_p = complex_double(Ic); // Complex phasor for Ic
	RCP<const Basic> Ic_conj_p = complex_double(conj(Ic)); // Complex conjugate of Ic
	RCP<const Basic> V1_p = complex_double(V1); // Complex phasor for V1
	RCP<const Basic> V1_conj_p = complex_double(conj(V1)); // Complex conjugate of V1

	complex<double> Zeq = Ls_r * j * omega1 + Rs + Rr_p / sigma_val; // Equivalent impedance
	complex<double> Vrs = V1 + Ir * Zeq; // Voltage at the rotor side
	complex<double> Vgs = V1 + Ic * Z_f_val; // Grid - side terminal voltage
	RCP<const Basic> Vrs_p = complex_double(Vrs); // Complex phasor for Vrs
	RCP<const Basic> Vrs_conj_p = complex_double(conj(Vrs)); // Complex conjugate of Vrs
	RCP<const Basic> Vgs_p = complex_double(Vgs); // Complex phasor for Vgs
	RCP<const Basic> Vgs_conj_p = complex_double(conj(Vgs)); // Complex conjugate of Vgs

	// Make control loops
	RCP<const Basic> Kp_pll_b = real_double(Kp_pll);
	RCP<const Basic> Ki_pll_b = real_double(Ki_pll);
	RCP<const Basic> Krp_b = real_double(Krp);
	RCP<const Basic> Kri_b = real_double(Kri);
	RCP<const Basic> Krd_b = real_double(Krd);
	RCP<const Basic> Ksp_b = real_double(Ksp);
	RCP<const Basic> Ksi_b = real_double(Ksi);
	RCP<const Basic> Ksd_b = real_double(Ksd);

	// PLL Control Loop
	RCP<const Basic> Hpll_s = mul(real_double(V1_mag), add(Kp_pll_b, mul(Ki_pll_b, div(one, s)))); // PLL transfer function
	RCP<const Basic> Hpll_p = mul(real_double(V1_mag), add(Kp_pll_b, mul(Ki_pll_b, div(one, s_p))));
	RCP<const Basic> Hpll_n = mul(real_double(V1_mag), add(Kp_pll_b, mul(Ki_pll_b, div(one, s_n))));

	RCP<const Basic> Tpll = div(Hpll_s, add(s, Hpll_s)); // Total PLL transfer function
	RCP<const Basic> Tpll_p = div(Hpll_p, add(s_p, Hpll_p)); // Total PLL transfer function
	RCP<const Basic> Tpll_n = div(Hpll_n, add(s_n, Hpll_n)); // Total PLL transfer function

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

	// GSC Admittance Calculation
	// positive sequence GSC admittance
	RCP<const Basic> num3 = add(Z_f_s, sub(Hsi_p, mul(I, Ksd_b)));
	RCP<const Basic> bracket3 = add(mul(div(Ic_p, V1_p), sub(Hsi_p, mul(I, Ksd_b))), div(Vgs_p, V1_p));
	RCP<const Basic> denom3 = sub(one, mul(div(Tpll_p, real_double(2)), bracket3));

	RCP<const Basic> Y_GS_p = div(denom3, num3); // GSC admittance in positive sequence

	// negative sequence GSC admittance
	RCP<const Basic> num4 = add(Z_f_s, add(Hsi_n, mul(I, Ksd_b)));
	RCP<const Basic> bracket4 = add(mul(div(Ic_conj_p, V1_conj_p), add(Hsi_n, mul(I, Ksd_b))), div(Vgs_conj_p, V1_conj_p));
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