#ifndef _WT_TYPE_3_H_
#define _WT_TYPE_3_H_

/**
 * @file WT_type_3.h
 * @brief Type 3 doubly-fed induction generator (DFIG) wind turbine model.
 */

#include "RES_base.h"

/**
 * @class WTtype3
 * @brief Type 3 wind turbine with DFIG, RSC/GSC converters, and filter.
 * @ingroup res
 *
 * Constructor pack (22 values): V_LL, f1, P, Qs, Qgsc, slip, Lm, Lr, Rr, Nsr,
 * Rs, Ls, Kp_pll, Ki_pll, Krp, Kri, Krd, Ksp, Ksi, Ksd, Rf, Lf.
 * Rotor/stator currents and PCC voltage phasors are computed from P, Q, slip.
 */
class WTtype3 : public RES_base {
	friend class PowerFlow;
public:
	/**
	 * @brief Construct a Type 3 wind turbine from a parameter vector.
	 * @param symbol Element identifier.
	 * @param location Network area or location string.
	 * @param parameters Packed electrical, control, and filter parameters.
	 */
	WTtype3(const string& symbol, const std::string& location, const vector<double>& parameters);

	~WTtype3() {}

private:
	double V_LL = 690.0;				// PCC voltage, line-to-line rms (V)
	double f1 = 50.0;					// Grid frequency (Hz)
	double p = 2.5e6;					// Operating / rated power (W); OPF reads MW as p/1e6
	double Qs = 0.0;					// Stator reactive power (var)
	double Qgsc = 0.0;					// GSC reactive power (var)
	double slip = -0.35;				// Rotor slip (fraction of stator frequency)

	double Lm = 0.0026357112818360907;	// Magnetizing inductance, stator (H)
	double Lr = 0.0003625396453819264;	// Rotor leakage, rotor side (H)
	double Rr = 0.0079498735684056;		// Rotor resistance, rotor side (Ohm)
	double Nsr = 1.0 / 2.6377;			// Turns ratio Ns/Nr
	double Rs = 0.0019044;				// Stator resistance (Ohm)
	double Ls = 6.183131341933791e-05;	// Stator leakage Lls (H)

	double Kp_pll = 0.05325027336821547;	// PLL proportional gain (1/V)
	double Ki_pll = 0.3550018224547698;	// PLL integral gain (1/(V s))

	double Krp = 0.6624894640338;		// RSC proportional gain, rotor-side ohm
	double Kri = 33.12447320169;		// RSC integral gain
	double Krd = 0.0;					// RSC decoupling gain

	double Ksp = 0.19044;				// GSC proportional gain (Ohm)
	double Ksi = 9.522;					// GSC integral gain
	double Ksd = 0.0;					// GSC decoupling gain

	double Rf = 0.0;					// Filter resistance (Ohm)
	double Lf = 0.0003;					// Filter inductance (H)
};


#endif
