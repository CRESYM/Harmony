/**
 * @file PV_plant.cpp
 * @brief Implementation of Aggregated PV plant with boost converter and grid-connected inverter.
 */
#include "PV_plant.h"

PVplant::PVplant(const string& symbol, const std::string& location, const vector<double>& parameters) : RES_base(symbol, location) {
	// Default parameters for a PV plant can be set here if needed
	if (parameters.size() != 26) {
		cerr << "Error: PV plant requires exactly 26 parameters." << endl;
		exit(EXIT_FAILURE);
	}

	// Assign parameters from the input vector
	P_pv = parameters[0]; // Rated power of the PV plant in watts
	I_pv = parameters[1]; // Rated current of the PV plant in amperes
	N_s = parameters[2];  // Number of series-connected modules
	N_p = parameters[3];  // Number of parallel-connected strings
	n = parameters[4];    // ideally factor of the diode
	I_sc = parameters[5]; // Short-circuit current of a single module at standard test conditions (STC)
	I0 = parameters[6];   // Reverse saturation current of the diode
	C_pv = parameters[7]; // Capacitance of the PV array in farads

	// Parameters for the boost converter
	V_dc = parameters[8];    // DC link voltage in volts
	L_boost = parameters[9]; // Inductance of the boost converter in henries
	C_dc = parameters[10];    // Capacitance of the DC link in farads
	// Control of the boost converter is done by controlling the duty cycle D
	kp_boost = parameters[11]; // Proportional gain for the boost converter voltage control loop
	ki_boost = parameters[12]; // Integral gain for the boost converter voltage control loop

	// Parameters for the voltage source inverter (VSI)
	L_1 = parameters[13];  // Inductance of the filter in henries
	R_1 = parameters[14]; // Resistance of the filter in ohms
	C_f = parameters[15]; // Capacitance of the filter in farads
	R_c = parameters[16]; // Resistance of the filter in ohms
	L_2 = parameters[17]; // Grid-side inductance in henries
	// Grid parameters
	V_g = parameters[18]; // Grid voltage in volts, assumed to have the optimal operation of PLL
	f_g = parameters[19]; // Grid frequency in hertz
	// Control parameters
	// Parameters for the DC voltage control loop
	K_p_dc = parameters[20]; // Proportional gain of the DC voltage controller
	K_i_dc = parameters[21]; // Integral gain of the DC voltage controller
	// Parameters for the current control loop
	K_p_i = parameters[22]; // Proportional gain of the current controller
	K_i_i = parameters[23]; // Integral gain of the current controller
	// Parameters for the PLL
	K_p_pll = parameters[24]; // Proportional gain of the PLL
	K_i_pll = parameters[25]; // Integral gain of the PLL

	// Calculation of the operation point
	// Assuming operation at maximum power point (MPP), calculate V_pv and I_pv
	double V_pv = P_pv / I_pv; // Voltage at MPP
	double I_dc = P_pv / V_dc; // DC current through the boost converter
	double D = 1 - (V_pv / V_dc); // Duty cycle of the boost converter
	if (D < 0 || D > 1) {
		cerr << "Error: Calculated duty cycle D is out of bounds (0 < D < 1)." << endl;
		exit(EXIT_FAILURE);
	}
	double I_l = I_pv; // Inductor current in the boost converter

	// Further initialization and state variable setup can be done here
	double V_pccd = V_g * sqrt(2.0/3.0); // Peak grid voltage
	double V_pccq = 0.0; // Assuming operation at unity power factor
	double I_2d = 2.0/3.0 * P_pv / V_pccd; // d-axis current injection
	double I_2q = 0.0; // q-axis current injection for unity power factor
	//cout << V_pccd << " " << V_pccq << " " << I_2d << " " << I_2q << endl;

	// Initialize state variables, control states, etc.
	double omega_g = 2 * M_PI * f_g; // Angular frequency of the grid
	double Vcfd = (V_pccd - omega_g * L_2 * I_2q + pow(omega_g,2)*R_c*C_f*L_2*I_2d
		+ omega_g*R_c*C_f*V_pccq) / (1 + pow(omega_g * C_f * R_c,2)); // Filter capacitor voltage d-axis
	double Vcfq = (V_pccq + omega_g * L_2 * I_2d + pow(omega_g, 2) * R_c * C_f * L_2 * I_2q
		- omega_g * R_c * C_f * V_pccd) / (1 + pow(omega_g * C_f * R_c, 2)); // Filter capacitor voltage q-axis
	
	double I_1d = I_2d - Vcfq * omega_g * C_f; // d-axis current through the filter inductor
	double I_1q = I_2q + Vcfd * omega_g * C_f; // q-axis current through the filter inductor
	//cout << Vcfd << " " << Vcfq << " " << I_1d << " " << I_1q << endl;

	double V_sd = V_pccd - omega_g * L_1 * I_1q - R_1 * I_1d - omega_g * L_2 * I_2q;
	double V_sq = V_pccq + omega_g * L_1 * I_1d - R_1 * I_1q + omega_g * L_2 * I_2d;

	double Md0 = 2.0 * V_sd / V_dc;
	double Mq0 = 2.0 * V_sq / V_dc;
	// cout << V_sd << " " << V_sq << " " << Md0 << " " << Mq0 << endl;

	// Setting up the controllers
	// PI controller for the DC voltage control loop
	RCP<const Basic> gdc = add(real_double(K_p_dc), div(real_double(K_i_dc), s));
	DenseMatrix Gdc = createZeroMatrix(2, 2);
	Gdc.set(0, 0, gdc);

	// PI controller for the current control loop
	RCP<const Basic> gi = add(real_double(K_p_i), div(real_double(K_i_i), s));
	DenseMatrix Gi = createZeroMatrix(2, 2);
	Gi.set(0, 0, gi); // d-axis
	Gi.set(1, 1, gi); // q-axis
	DenseMatrix Gv = createZeroMatrix(2, 2);
	RCP<const Basic> kv = mul(gi, real_double(1.0 / V_dc));
	Gv.set(0, 0, kv); // d-axis
	Gv.set(1, 1, kv); // q-axis
	//cout << Gi.__str__() << endl;
	//cout << Gv.__str__() << endl;

	// PI controller for the PLL
	RCP<const Basic> Hpll = add(real_double(K_p_pll), div(real_double(K_i_pll), s));
	RCP<const Basic> Gpll = div(Hpll, add(s, mul(real_double(V_pccd), Hpll)));
	DenseMatrix Gpllm = createZeroMatrix(2, 2);
	Gpllm.set(0, 1, mul(Gpll, real_double(Mq0))); // PLL acts on the q-axis only
	Gpllm.set(1, 1, mul(Gpll, real_double(-Md0))); // PLL acts on the q-axis only
	DenseMatrix Gplli = createZeroMatrix(2, 2);
	Gplli.set(0, 1, mul(Gpll, real_double(I_2q))); // PLL acts on the q-axis only
	Gplli.set(1, 1, mul(Gpll, real_double(-I_2d))); // PLL acts on the q-axis only
	/*cout << Gpllm.__str__() << endl;
	cout << Gplli.__str__() << endl;*/

	// PV panel model, dynamical model of the PV array
	// Boost controller
	RCP<const Basic> gb = add(real_double(kp_boost), div(real_double(ki_boost), s));
	// PV coefficient
	double Tn = 298.18; // nominal temperature
	double k = 1.380625e-23; // Boltzmann's constant
	double q = 1.60217e-19; // unit electric charge
	double k_pv = -q * (N_p * I0 + N_p * I_sc - I_pv) / (N_s * n * k * Tn);
	double k_mp = pow(N_s * n * k * Tn / q, 2) / (N_p * I0 * V_pv * exp(q * V_pv / (N_s * n * k * Tn)) + I_pv / V_pv * pow(N_s * n * k * Tn / q, 2));
	double lambda = k_pv * k_mp;
	// cout << k_pv << " " << k_mp << " " << lambda << endl;

	RCP<const Basic> L_b = real_double(L_boost);
	RCP<const Basic> C_b = real_double(C_pv);
	RCP<const Basic> C_dc_b = real_double(C_dc);
	RCP<const Basic> Vdc0 = real_double(V_dc);
	RCP<const Basic> Il0 = real_double(I_l);
	RCP<const Basic> kpv_b = real_double(k_pv);
	RCP<const Basic> lambda_b = real_double(lambda);

	RCP<const Basic> h1 = mul(mul(s, L_b), sub(kpv_b, mul(s, C_b))); // h1 = s*L_b*(k_pv - s*C_b)
	RCP<const Basic> h2 = sub(kpv_b, mul(s, C_b)); // h2 = k_pv - s*C_b
	RCP<const Basic> D_minus_1 = sub(real_double(D), one);
	RCP<const Basic> gb1 = mul(gb, sub(one, lambda_b)); // gb1 = gb*(1 - lambda)

	RCP<const Basic> zdc = add(add(neg(h1), one), mul(Vdc0, gb1)); // zdc = -h1 + 1 + V_dc*gb*(1 - lambda)
	RCP<const Basic> denom = add(sub(mul(s, mul(C_dc_b, h1)), mul(s, C_dc_b)),
		add(mul(mul(Vdc0, minus_one), mul(gb1, mul(s, C_dc_b))), add(mul(mul(D_minus_1, D_minus_1), h2),
			mul(Il0, mul(gb1, D_minus_1))))); // denom = s*C_dc*h1 - s*C_dc + V_dc*(-1)*gb*(1 - lambda)*s*C_dc + (D - 1)^2*h2 + I_l*gb*(1 - lambda)*(D - 1)
	zdc = div(zdc, denom); // zdc = zdc / denom

	DenseMatrix Zdc = createZeroMatrix(2, 2);
	Zdc.set(0, 0, zdc); // DC voltage response to perturbation in the duty cycle


	// Calculation of the impedances
	DenseMatrix Nm = createZeroMatrix(2, 2);
	Nm.set(0, 0, real_double(Md0)); Nm.set(0, 1, real_double(Mq0));
	DenseMatrix Ni = createZeroMatrix(2, 2);
	Ni.set(0, 0, real_double(I_1d)); Ni.set(0, 1, real_double(I_1q));
	DenseMatrix Hm = createZeroMatrix(2, 2);
	Hm.set(0, 0, real_double(Md0 / 2.0)); Hm.set(1, 0, real_double(Mq0 / 2.0));
	DenseMatrix Hv = createZeroMatrix(2, 2);
	Hv.set(0, 0, real_double(V_dc / 2.0)); Hv.set(1, 1, real_double(V_dc / 2.0));
	// Impedances of filter and grid
	// Impedance from R1 and L1
	DenseMatrix Zrl1 = createZeroMatrix(2, 2);
	Zrl1.set(0, 0, add(real_double(R_1), mul(real_double(L_1), s)));
	Zrl1.set(0, 1, real_double(-omega_g * L_1));
	Zrl1.set(1, 0, real_double(omega_g * L_1));
	Zrl1.set(1, 1, add(real_double(R_1), mul(real_double(L_1), s)));
	//cout << Zrl1.__str__() << endl;
	// Impedance from L2
	DenseMatrix Zl2 = createZeroMatrix(2, 2);
	Zl2.set(0, 0, mul(real_double(L_2), s));
	Zl2.set(0, 1, real_double(-omega_g * L_2));
	Zl2.set(1, 0, real_double(omega_g * L_2));
	Zl2.set(1, 1, mul(real_double(L_2), s));
	// Impedance from Rc and Cf
	DenseMatrix Zrc = createZeroMatrix(2, 2);
	Zrc.set(0, 0, real_double(R_c));
	Zrc.set(1, 1, real_double(R_c));
	DenseMatrix Yc = createZeroMatrix(2, 2);
	Yc.set(0, 0, mul(real_double(C_f), s));
	Yc.set(0, 1, real_double(-omega_g * C_f));
	Yc.set(1, 0, real_double(omega_g * C_f));
	Yc.set(1, 1, mul(real_double(C_f), s));

	// Helper matrices for the calculation of the overall admittance
	DenseMatrix identity = createZeroMatrix(2, 2);
	identity.set(0, 0, one); identity.set(1, 1, one);
	DenseMatrix M = createZeroMatrix(2, 2);
	mul_dense_dense(Zrc, Yc, M);
	add_dense_dense(identity, M, M); // M = I + Z_rc*Yc 
	DenseMatrix M_inv = createZeroMatrix(2, 2);
	inverse_LU(M, M_inv); // M_inv = (I + Z_rc*Yc)^-1

	DenseMatrix M1 = createZeroMatrix(2, 2);
	mul_dense_dense(Zrl1, Yc, M1);
	mul_dense_dense(M1, M_inv, M1); // M1 = Zrl1*Yc*(I + Z_rc*Yc)^-1
	add_dense_dense(M1, identity, M1); // M1 = I + Zrl1*Yc*(I + Z_rc*Yc)^-1
	DenseMatrix M1_inv = createZeroMatrix(2, 2);	
	inverse_LU(M1, M1_inv); // M1_inv = (I + Zrl1*Yc*(I + Z_rc*Yc)^-1)^-1

	DenseMatrix M3 = createZeroMatrix(2, 2);
	mul_dense_dense(Zrc, Yc, M3);
	add_dense_dense(M3, identity, M3); // M3 = I + Zrc*Yc
	DenseMatrix M3_inv = createZeroMatrix(2, 2);
	inverse_LU(M3, M3_inv); // M3_inv = (I + Zrc*Yc)^-1
	mul_dense_dense(Yc, M3_inv, M3); // M3 = Yc*(I + Zrc*Yc)^-1
	mul_dense_dense(Nm, M3, M3); // M3 = Nm*Yc*(I + Zrc*Yc)^-1

	// Matrices for calculation of the transfer function from m_dq, i_2dq, v_dc to v_pccdq
	DenseMatrix Gvv = createZeroMatrix(2, 2);
	mul_dense_dense(M1_inv, Hm, Gvv); // Gmv = (I + Zrl1*Yc*(I + Z_rc*Yc)^-1)^-1 * Hv
	DenseMatrix Giv = createZeroMatrix(2, 2);
	mul_dense_dense(M1, Zl2, M1); // M1 = (Zrl1*Yc*(I + Z_rc*Yc)^-1 + I)*Zl2
	add_dense_dense(M1, Zrl1, M1); // M1 = Zrl1 + Zl2 + Zrl1*Yc*(I + Z_rc*Yc)^-1*Zrl2
	mul_dense_scalar(M1, real_double(-1.0), M1); // M1 = -(Zrl1 + Zl2 + Zrl1*Yc*(I + Z_rc*Yc)^-1*Zrl2)
	mul_dense_dense(M1_inv, M1, Giv); // Giv = M1_inv * M1
	DenseMatrix Gmv = createZeroMatrix(2, 2);
	mul_dense_dense(M1_inv, Hv, Gmv); // Gvv = (I + Zrl1*Yc*(I + Z_rc*Yc)^-1)^-1 * Hm

	// Matrices for calculation of the transfer function from m_dq, i_2dq, v_dc to i_dc
	DenseMatrix Gmi = Ni;
	DenseMatrix Gii = createZeroMatrix(2, 2);
	mul_dense_dense(M3, Zl2, Gii); // Gii = Nm * Yc * (I + Zrc*Yc)^-1 * Zl2
	add_dense_dense(Gii, Nm, Gii); // Gii = Nm + Nm * Yc * (I + Zrc*Yc)^-1 * Zl2
	DenseMatrix Gvi = M3; // Gvi = Nm * Yc * (I + Zrc*Yc)^-1
	// cout << Gvi.__str__() << endl;

	// Overall admittance matrix of the PV plant
	DenseMatrix Ja = createZeroMatrix(2, 2);
	DenseMatrix Fa = createZeroMatrix(2, 2);
	mul_dense_dense(Gi, Gdc, Ja); // Ja = Gi*Gdc
	mul_dense_dense(Gv, Ja, Ja); // Ja = Gv*Gi*Gdc
	mul_dense_dense(Gmi, Ja, Fa); // Fa = Gmi*Gv*Gi*Gdc
	mul_dense_dense(Gmv, Ja, Ja); // Ja = Gmv*Gv*Gi*Gdc
	add_dense_dense(Gvv, Ja, Ja); // Ja = Gvv + Gmv*Gv*Gi*Gdc
	mul_dense_dense(Zdc, Fa, Fa); // Fa = Zdc*Gmi*Gv*Gi*Gdc
	mul_dense_scalar(Fa, minus_one, Fa); // Fa = -Zdc*Gmi*Gv*Gi*Gdc
	add_dense_dense(identity, Fa, Fa); // Fa = I - Zdc*Gmi*Gv*Gi*Gdc

	DenseMatrix Jb = createZeroMatrix(2, 2);
	DenseMatrix Fb = createZeroMatrix(2, 2);
	mul_dense_dense(Gv, Gi, Jb); // Jb = Gv*Gi
	mul_dense_dense(Gmi, Jb, Fb); // Fb = Gmi*Gv*Gi
	mul_dense_dense(Gmv, Jb, Jb); // Jb = Gmv*Gv*Gi
	mul_dense_scalar(Jb, minus_one, Jb); // Jb = -Gmv*Gv*Gi
	add_dense_dense(Giv, Jb, Jb); // Jb = Giv - Gmv*Gv*Gi
	mul_dense_dense(Zdc, Fb, Fb); // Fb = Zdc*Gmi*Gv*Gi
	mul_dense_scalar(Fb, minus_one, Fb); // Fb = -Zdc*Gmi*Gv*Gi
	DenseMatrix H1 = createZeroMatrix(2, 2);
	mul_dense_dense(Zdc, Gii, H1); // H1 = Zdc*Gii
	add_dense_dense(H1, Fb, Fb); // Fb = Zdc*Gii - Zdc*Gmi*Gv*Gi

	DenseMatrix Jc = createZeroMatrix(2, 2);
	DenseMatrix Fc = createZeroMatrix(2, 2);
	mul_dense_dense(Gv, Gi, Jc); // Jc = Gv*Gi
	mul_dense_dense(Jc, Gplli, Jc); // Jc = Gv*Gi*Gplli
	add_dense_dense(Gpllm, Jc, Jc); // Jc = Gpllm + Gv*Gi*Gplli
	mul_dense_dense(Gmi, Jc, Fc); // Fc = Gmi*(Gpllm + Gv*Gi*Gplli)
	mul_dense_dense(Zdc, Fc, Fc); // Fc = Zdc*Gmi*(Gpllm + Gv*Gi*Gplli)
	mul_dense_scalar(Fc, minus_one, Fc); // Fc = -Zdc*Gmi*(Gpllm + Gv*Gi*Gplli)
	DenseMatrix H2 = createZeroMatrix(2, 2);
	mul_dense_dense(Zdc, Gvi, H2); // H2 = Zdc*Gvi
	add_dense_dense(H2, Fc, Fc); // Fc = Zdc*Gvi - Zdc*Gmi*(Gpllm + Gv*Gi*Gplli)
	mul_dense_dense(Gmv, Jc, Jc);
	add_dense_dense(Jc, identity, Jc); // Jc = I + Gmv*(Gpllm + Gv*Gi*Gplli)

	// Full admittance matrix
	DenseMatrix H4 = createZeroMatrix(2, 2);
	inverse_LU(Fa, H4); 
	mul_dense_dense(Ja, H4, H4); // H4 = Ja*Fa^-1
	mul_dense_dense(H4, Fb, H4); // H4 = Ja*Fa^-1*Fb
	add_dense_dense(H4, Jb, H4); // H4 = Ja*Fa^-1*Fb + Jb
	//inverse_LU(H4, H4); // H4 = (Ja*Fa^-1*Fb + Jb)^-1
	DenseMatrix H5 = createZeroMatrix(2, 2);
	inverse_LU(Fa, H5); 
	mul_dense_dense(Ja, H5, H5); // H5 = Ja*Fa^-1
	mul_dense_dense(H5, Fc, H5); // H5 = Ja*Fa^-1*Fc
	mul_dense_scalar(H5, minus_one, H5); // H5 = -Ja*Fa^-1*Fc
	add_dense_dense(H5, Jc, H5); // H5 = Jc - Ja*Fa^-1*Fc

	Y_matrix.resize(2, 2);
	inverse_LU(H5, H5); // H5 = (Jc - Ja*Fa^-1*Fc)^-1
	mul_dense_scalar(H5, minus_one, Y_matrix); // Y_matrix = -(Jc - Ja*Fa^-1*Fc)^-1
	mul_dense_dense(Y_matrix, H4, Y_matrix); // Y_matrix = -(Jc - Ja*Fa^-1*Fc)^-1 * (Ja*Fa^-1*Fb + Jb)
	inverse_LU(Y_matrix, Y_matrix); // Y_matrix = [-(Jc - Ja*Fa^-1*Fc)^-1 * (Ja*Fa^-1*Fb + Jb)]^-1
}
