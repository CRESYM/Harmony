#include "Examples.h"

#include "../Include_components.h"
#include "../Constants.h"

void example_WT_type_3() {
	// Example parameters for WT_type_3
	vector<double> parameters = {
		1080.0,			// nm_rpm
		3,				// p
		0.0644e-3,		// Lr
		0.0092,			// Rr
		1.0 / 0.33,		// Nsr
		0.0092,			// Rs
		0.1356e-3,		// Ls
		48.873,			// Kp_pll
		3070.1,			// Ki_pll
		564.1,			// V1_mag
		-59,			// V1_theta_deg
		0.052,			// Krp
		0.027,			// Kri
		0.00245,		// Krd
		5.8,			// Ir_mag
		-22,			// Ir_theta_deg
		1.452,			// Ksp
		3384.0,			// Ksi
		0.025,			// Ksd
		152.3,			// Ic_mag
		114.0,			// Ic_theta_deg
		0.0,			// Rf
		0.001,			// Lf
		50.0			// f1 (Grid frequency in Hz)
	};
	
	WTtype3* wt = new WTtype3("DFIG", "AC1", parameters);

	wt->writeFile(1, 1000, 1000);

	wt->plotYParameters(1, 1000, 10000);
}