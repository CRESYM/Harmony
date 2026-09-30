/**
 * @file example_PV_plant.cpp
 * @brief Runnable example: Photovoltaic plant model demonstration.
 */
#include "Examples.h"

#include "core/Include_components.h"
#include "core/Constants.h"

void example_PV_plant(bool plotting_enabled /*=true*/) {
	vector<double> pv_parameters = {
		1009603.68,	// P_pv (W), RTDS array MPP
		2000.8,		// I_pv (A), Np*Imp
		1044.0,		// N_s
		656.0,		// N_p
		1.9267367660446415, // n (MATLAB MPP diode fit)
		3.35,		// I_sc (A), module
		1.7250567519501773e-05, // I0 (A)
		7.2e-3,		// C_pv (F)
		800.0,		// V_dc (V)
		250e-6,		// L_boost (H)
		32e-3,		// C_dc (F), Zhao RTDS analytical value
		0.002,		// kp_boost
		0.04,		// ki_boost
		63e-6,		// L_1 (H)
		0.0,		// R_1 (Ohm)
		1500e-6,	// C_f (F)
		0.051,		// R_c (Ohm)
		0.0,		// L_2 (H)
		315.0,		// V_g L-L rms (V)
		50.0,		// f_g (Hz)
		16.20032898666123,	// K_p_dc
		324.0065797332246,	// K_i_dc
		0.19845,	// K_p_i
		9.9225,		// K_i_i
		0.732885617235351,	// K_p_pll
		4.88590411490234	// K_i_pll
	};

	PVplant* pv = new PVplant("PV1", "AC1", pv_parameters);
	cout << "PV plant model initialized successfully." << endl;
	// Further operations with the pv_plant object can be performed here	

	//pv->writeFile(1, 1000, 1000);

	if (plotting_enabled) {
		pv->plotYParameters(1, 10000, 500);
	}
}