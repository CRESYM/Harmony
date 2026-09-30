#ifndef _WT_TYPE_4_H_
#define _WT_TYPE_4_H_

/**
 * @file WT_type_4.h
 * @brief Type 4 full-converter wind turbine model.
 */

#include "RES_base.h"

/**
 * @class WTtype4
 * @brief Type 4 wind turbine with full-scale back-to-back converter and filter.
 * @ingroup res
 *
 * Constructor pack (14 values): Vm, f1, Pwt, Qwt, Vdc, Kp_pll, Ki_pll, Kpi,
 * Kii, Tdelay, wn, zeta, Rf, Lf. dq currents follow from P, Q at Vq = 0.
 */
class WTtype4 : public RES_base {
	friend class PowerFlow;
public:
	/**
	 * @brief Construct a Type 4 wind turbine from a parameter vector.
	 * @param symbol Element identifier.
	 * @param location Network area or location string.
	 * @param parameters Packed grid, control, and filter parameters.
	 */
	WTtype4(const string& symbol, const std::string& location, const vector<double>& parameters);

	~WTtype4() {}

private:
	double Vm = 690.0;					// Grid voltage, line-to-line rms (V)
	double f1 = 50.0;					// Grid frequency (Hz)
	double Pwt = 2.5e6;					// Wind turbine power (W)
	double Qwt = 0.0;					// Reactive power (var)
	double Vdc = 1200.0;				// DC link voltage (V)

	double Kp_pll = 0.05325027336821547;	// PLL proportional gain (1/V)
	double Ki_pll = 0.3550018224547698;	// PLL integral gain (1/(V s))

	double Kpi = 0.19044;				// Current PI proportional gain (Ohm)
	double Kii = 9.522;					// Current PI integral gain (Ohm/s)

	double Tdelay = 0.00075;			// PWM Padé delay (s)

	double wn = 1.23e6;					// Measurement-filter natural frequency (rad/s)
	double zeta = 4.74e-13;				// Measurement-filter damping ratio

	double Rf = 0.0;					// Filter resistance (Ohm)
	double Lf = 0.00012;				// Filter inductance (H)
};

#endif
