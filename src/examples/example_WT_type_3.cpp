/**
 * @file example_WT_type_3.cpp
 * @brief Runnable example: Type-3 wind-turbine model demonstration.
 */
#include "Examples.h"

#include "core/Include_components.h"
#include "core/Constants.h"

void example_WT_type_3(bool plotting_enabled /*=true*/) {
	// 2.5 MW / 690 V / 50 Hz DFIG. Currents and V1 follow from P, Q, slip.
	vector<double> parameters = {
		690.0,			// V_LL, line-to-line rms (V)
		50.0,			// f1 (Hz)
		2.5e6,			// P (W)
		0.0,			// Qs (var)
		0.0,			// Qgsc (var)
		-0.35,			// slip
		0.0026357112818360907,	// Lm, stator (H)
		0.0003625396453819264,	// Lr, rotor-side leakage (H)
		0.0079498735684056,	// Rr, rotor-side (Ohm)
		1.0 / 2.6377,		// Nsr = Ns/Nr
		0.0019044,		// Rs (Ohm)
		6.183131341933791e-05,	// Ls = Lls leakage (H)
		0.05325027336821547,	// Kp_pll (1/V)
		0.3550018224547698,	// Ki_pll (1/(V s))
		0.6624894640338,	// Krp, rotor-side (Ohm)
		33.12447320169,		// Kri
		0.0,			// Krd
		0.19044,		// Ksp (Ohm)
		9.522,			// Ksi
		0.0,			// Ksd
		0.0,			// Rf
		0.0003			// Lf (H)
	};
	
	WTtype3* wt = new WTtype3("DFIG", "AC1", parameters);

	wt->writeFile(1, 1000, 1000);

	if (plotting_enabled) {
		wt->plotYParameters(1, 1000, 500);
	}
}
