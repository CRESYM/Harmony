/**
 * @file example_WT_type_4.cpp
 * @brief Runnable example: Type-4 wind-turbine model demonstration.
 */
#include "Examples.h"

#include "core/Include_components.h"
#include "core/Constants.h"

void example_WT_type_4(bool plotting_enabled /*=true*/) {
	// 2.5 MW / 690 V / 50 Hz GSC. dq currents follow from P, Q.
	vector<double> parameters = {
		690.0,			// Vm, line-to-line rms (V)
		50.0,			// f1 (Hz)
		2.5e6,			// Pwt (W)
		0.0,			// Qwt (var)
		1200.0,			// Vdc (V)
		0.05325027336821547,	// Kp_pll (1/V)
		0.3550018224547698,	// Ki_pll (1/(V s))
		0.19044,		// Kpi (Ohm)
		9.522,			// Kii (Ohm/s)
		1.5 / 2000.0,		// Tdelay (s)
		1.23e6,			// wn (rad/s)
		4.74e-13,		// zeta
		0.0,			// Rf (Ohm)
		0.00012			// Lf (H)
	};

	WTtype4* wt = new WTtype4("PMSG", "AC1", parameters);

	if (plotting_enabled){
		wt->plotYParameters(1, 100000, 500);
	}
}
