#ifndef _WT_TYPE_3_H_
#define _WT_TYPE_3_H_

#include "RES_base.h"

class WTtype3 : public RES_base {
public:
	WTtype3(const string& symbol, const vector<double>& parameters);

	~WTtype3() {}

private:
	// Rotor and Stator Electrical Parameters - 8 values
	double nm_rpm = 1080;				//Mechanical speed in rpm
	double p = 3;						// Number of rotor pole pairs
	double Lr = 0.0644e-3;				// Rotor inductance(H)
	double Rr = 0.0092;					// Rotor resistance (Ohms)
	double Nsr = 1.0 / 0.33;			// Turns ratio(Stator:Rotor)
	double Rs = 0.0092;					// Stator resistance(Ohms)
	double Ls = 0.1356e-3;				// Stator inductance(H)

	// PLL Parameters - 3 values
	double Kp_pll = 48.873;				// PLL proportional gain
	double Ki_pll = 3070.1;				// PLL integral gain
	double V1_mag = 564.1;				// PCC voltage magnitude(V)
	double V1_theta_deg = -59;			// PCC voltage angle (deg)

	// Rotor Side Converter (RSC) Current Controller - 4 values
	double Krp = 0.052;					// Proportional gain
	double Kri = 0.027;					// Integral gain
	double Krd = 0.00245;				// Derivative gain
	double Ir_mag = 5.8;				// Rotor current magnitude (A)
	double Ir_theta_deg = -22;			// Current angle (deg)

	// Grid Side Converter (GSC) Current Controller - 4 values
	double Ksp = 1.452;					// Proportional gain
	double Ksi = 3384;					// Integral gain
	double Ksd = 0.025;					// Derivative gain 
	double Ic_mag = 152.3;				// GSC current magnitude (A)
	double Ic_theta_deg = 114;			// GSC current angle (deg)

	// Filter Parameters - 2 values
	double Rf = 0.0;					// Filter resistance (Ohms)
	double Lf = 0.001;					// Filter inductance (H)
};


#endif