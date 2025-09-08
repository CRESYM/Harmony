#include "Examples.h"

#include "../Include_components.h"
#include "../Constants.h"

void example_PV_plant() {
	// Example usage of the PVplant class
	vector<double> pv_parameters = {
		2.8e6,		// P_pv: Rated power of the PV plant in watts
		6570,		// I_pv: Rated current of the PV plant in amperes
		2760,		// N_s: Number of series-connected modules
		720,		// N_p: Number of parallel-connected strings
		1.5,		// n: ideally factor of the diode
		2.5,		// I_sc: Short-circuit current of a single module at STC
		1e-9,		// I0: Reverse saturation current of the diode
		7.2e-3,     // C_pv: Capacitance of the PV array in farads
		900.0,		// V_dc: DC link voltage in volts
		16e-6,      // L_boost: Inductance of the boost converter in henries
		70e-3,      // C_dc: Capacitance of the DC link in farads
		4.9809e-06,	// kp_boost: Proportional gain for the boost converter voltage control loop
		4.9809e-06,	// ki_boost: Integral gain for the boost converter voltage control loop
		103e-6,		// L_1: Inductance of the filter in henries
		0.0,		// R_1: Resistance of the filter in ohms
		120e-6,		// C_f: Capacitance of the filter in farads
		0.02,		// R_c: Resistance of the filter in ohms
		55e-6,		// L_2: Grid-side inductance in henries
		380.0,		// V_g: Grid voltage in volts
		50.0,	    // f_g: Grid frequency in hertz
		10,			// K_p_dc: Proportional gain of the DC voltage controller
		500.0,		// K_i_dc: Integral gain of the DC voltage controller
		0.45,		// K_p_i: Proportional gain of the current controller
		69.7,		// K_i_i: Integral gain of the current controller
		0.5,		// K_p_pll: Proportional gain of the PLL
		1.0			// K_i_pll: Integral gain of the PLL
	};

	PVplant* pv = new PVplant("PV1", pv_parameters);
	cout << "PV plant model initialized successfully." << endl;
	// Further operations with the pv_plant object can be performed here	

	//pv->writeFile(1, 1000, 1000);

	pv->plotYParameters(1, 10000, 10000);
}