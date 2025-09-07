#ifndef PV_PLANT_H_
#define PV_PLANT_H_

#include "RES_base.h"

/*
* PV plant is considered as the two stage PV inverter with first stage being the boost DC-DC converter,
* and second stage being the voltage source inverter. At the output of the VSC is attached an LCL filter.
* The PV array is taken as an aggregated model of the entire plant.
*/
class PVplant : public RES_base {
public:
	PVplant(const string& symbol, const vector<double>& parameters);
	~PVplant() {}

private:
	// Parameters specific to the PV plant model
	double P_pv; // Rated power of the PV plant in watts
	double I_pv; // Rated current of the PV plant in amperes
	double N_s;  // Number of series-connected modules
	double N_p;  // Number of parallel-connected strings
	double n;	// ideally factor of the diode
	double I_sc; // Short-circuit current of a single module at standard test conditions (STC)
	double I0; // Reverse saturation current of the diode
	double C_pv; // Capacitance of the PV array in farads

	// Parameters for the boost converter
	double V_dc; // DC link voltage in volts
	double L_boost; // Inductance of the boost converter in henries
	double C_dc; // Capacitance of the DC link in farads
	// Control of the boost converter is done by controlling the duty cycle D
	double kp_boost; // Proportional gain for the boost converter voltage control loop
	double ki_boost; // Integral gain for the boost converter voltage control loop

	// Parameters for the voltage source inverter (VSI)
	double L_1; // Inductance of the filter in henries
	double R_1; // Resistance of the filter in ohms
	double C_f; // Capacitance of the filter in farads
	double R_c; // Resistance of the filter in ohms
	double L_2; // Grid-side inductance in henries
	double R_2; // Grid-side resistance in ohms

	// Grid parameters
	double V_g; // Grid voltage in volts, assumed to have the optimal operation of PLL
	double f_g; // Grid frequency in hertz

	// Control parameters
	// Parameters for the DC voltage control loop
	double K_p_dc; // Proportional gain of the DC voltage controller
	double K_i_dc; // Integral gain of the DC voltage controller
	// Parameters for the current control loop
	double K_p_i; // Proportional gain of the current controller
	double K_i_i; // Integral gain of the current controller
	// Parameters for the PLL
	double K_p_pll; // Proportional gain of the PLL
	double K_i_pll; // Integral gain of the PLL

};

#endif  // PV_PLANT_H_