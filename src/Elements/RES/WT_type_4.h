#ifndef _WT_TYPE_4_H_
#define _WT_TYPE_4_H_

#include "RES_base.h"

class WTtype4 : public RES_base {
	friend class PowerFlow;
public:
	WTtype4(const string& symbol, const std::string& location, const vector<double>& parameters);

	~WTtype4() {}

private:
	// Grid parameters
	double Vm = 34.5e3;					// Grid voltage magnitude (V)
	double f1 = 50.0;					// Grid frequency (Hz)
	double Pwt = 3.2e6;					// Wind turbine power (W)
	double Vdc = 1000;					// DC link voltage (V)

	// PLL Parameters 
	double Kp_pll = 0.93;				// PLL proportional gain
	double Ki_pll = 50;					// PLL integral gain

	// Current Controller 
	double Kpi = 0.053;					// Proportional gain
	double Kii = 30.59;					// Integral gain

	// Delay
	double Tdelay = 0.001;				// Delay in seconds

	// Second-order filter parameters
	double wn = 1.23e6;					// Natural frequency (rad/s)
	double zeta = 0.707;				// Damping ratio

	// Filter Parameters - 2 values
	double Rf = 0.095;					// Filter resistance (Ohms)
	double Lf = 0.0045;					// Filter inductance (H)

	// reference values
	double Id_ref = -75.73l;			// Reference Id current (A)
	double Iq_ref = 0.0;				// Reference Iq current (A)
};

#endif