#include "Examples.h"

#include "../Include_components.h"
#include "../Constants.h"

void example_WT_type_4() {
	vector<double> parameters = {
		690,		// Vm (Grid voltage line-to-line in V)
		50.0,		// f1 (Grid frequency in Hz)
		3.2e6,		// Pwt (Wind turbine power in W)
		1000,		// Vdc (DC link voltage in V)
		100,		// Kp_pll (PLL proportional gain)
		50,			// Ki_pll (PLL integral gain)
		10,			// Kpi (Current controller proportional gain)
		40,			// Kii (Current controller integral gain)
		1.5/2.5e3,	// Tdelay (Delay in seconds)
		1.23e6,     // wn (Natural frequency in rad/s)
		0.707,      // zeta (Damping ratio)
		0.095,      // Rf (Filter resistance in Ohms)
		0.0045,     // Lf (Filter inductance in H)
		-2000,     // Id_ref (Reference Id current in A)
		 0.0        // Iq_ref (Reference Iq current in A)
	};

	WTtype4* wt = new WTtype4("PMSG", parameters);

	//wt->printElementValues();

	wt->plotYParameters(1, 1000, 1000);
}