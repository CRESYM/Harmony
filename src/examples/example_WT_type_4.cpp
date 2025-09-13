#include "Examples.h"

#include "../Include_components.h"
#include "../Constants.h"

void example_WT_type_4() {
	vector<double> parameters = {
		690,		// Vm (Grid voltage line-to-line in V)
		60.0,		// f1 (Grid frequency in Hz)
		3.2e6,		// Pwt (Wind turbine power in W)
		1000,		// Vdc (DC link voltage in V)
		0.93,		// Kp_pll (PLL proportional gain)
		50,			// Ki_pll (PLL integral gain)
		0.053,		// Kpi (Current controller proportional gain)
		30.59,		// Kii (Current controller integral gain)
		1.5/2.5e3,	// Tdelay (Delay in seconds)
		1.23e6,     // wn (Natural frequency in rad/s)
		4.74e-13,   // zeta (Damping ratio)
		0.095,      // Rf (Filter resistance in Ohms)
		0.0045,     // Lf (Filter inductance in H)
		-75.73,     // Id_ref (Reference Id current in A)
		 0.0        // Iq_ref (Reference Iq current in A)
	};

	WTtype4* wt = new WTtype4("PMSG", "AC1", parameters);

	//wt->printElementValues();

	wt->plotYParameters(1, 100000, 10000);
}