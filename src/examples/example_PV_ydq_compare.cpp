/**
 * @file example_PV_ydq_compare.cpp
 * @brief Ydq(f) of the symbolic PVplant and the ODE PVgeneric on one operating point.
 *
 * Both use the 1 MW pack from example_PV_plant. The array is the ideal diode
 * (Rsr = 0, Rsh open), so the slope matches k_pv. PVgeneric keeps the PSCAD
 * power-mode boost and Q loop; the boost PI is the voltage-loop gain divided
 * by |dP/dV| at this power so the duty response is the same order. Current,
 * DC-voltage and PLL gains are copied unchanged, with k_ff = 0.
 */
#include "Examples.h"

#include "core/Include_components.h"
#include "core/Constants.h"

#include <cmath>
#include <filesystem>
#include <iostream>

void example_PV_ydq_compare(bool plotting_enabled /*=true*/) {
	(void)plotting_enabled;
	const std::vector<double> pv_parameters = {
		1009603.68, 2000.8, 1044.0, 656.0, 1.9267367660446415, 3.35,
		1.7250567519501773e-05, 7.2e-3, 800.0, 250e-6, 32e-3, 0.002, 0.04,
		63e-6, 0.0, 1500e-6, 0.051, 0.0, 315.0, 50.0,
		16.20032898666123, 324.0065797332246, 0.19845, 9.9225,
		0.732885617235351, 4.88590411490234
	};

	// |dP/dV| near this power is about 60 W/V, so a voltage-loop gain
	// (duty per volt) maps to a power-loop gain (duty per watt) by that factor.
	constexpr double kBoostVoltsPerWatt = 60.0;
	PVgenericParameters ode_parameters;
	ode_parameters.pv.Ns = 1044.0;
	ode_parameters.pv.Np = 656.0;
	ode_parameters.pv.n = 1.9267367660446415;
	ode_parameters.pv.Isc_ref = 3.35;
	ode_parameters.pv.Io_ref = 1.7250567519501773e-05;
	ode_parameters.pv.Tc = 298.15;
	ode_parameters.pv.Tc_ref = 298.15;
	ode_parameters.pv.G = 1000.0;
	ode_parameters.pv.G_ref = 1000.0;
	ode_parameters.pv.Rsr = 0.0;
	ode_parameters.pv.Rsh = 0.0;
	ode_parameters.C_pv = 7.2e-3;
	ode_parameters.L_boost = 250e-6;
	ode_parameters.C_dc = 32e-3;
	ode_parameters.Vdc_ref = 800.0;
	ode_parameters.Pref = 1009603.68;
	ode_parameters.Qref = 0.0;
	ode_parameters.Dmax = 0.95;
	ode_parameters.kp_boost = 0.002 / kBoostVoltsPerWatt;
	ode_parameters.ki_boost = 0.04 / kBoostVoltsPerWatt;
	ode_parameters.tau_boost = 0.0;
	ode_parameters.L1 = 63e-6;
	ode_parameters.R1 = 0.0;
	ode_parameters.Cf = 1500e-6;
	ode_parameters.Rc = 0.051;
	ode_parameters.L2 = 0.0;
	ode_parameters.Vm = 315.0 * std::sqrt(2.0 / 3.0);
	ode_parameters.theta = 0.0;
	ode_parameters.omega = 2.0 * M_PI * 50.0;
	ode_parameters.kp_dc = 16.20032898666123;
	ode_parameters.ki_dc = 324.0065797332246;
	ode_parameters.kp_q = 1.0e-5;
	ode_parameters.ki_q = 0.02;
	ode_parameters.kp_i = 0.19845;
	ode_parameters.ki_i = 9.9225;
	ode_parameters.kp_pll = 0.732885617235351;
	ode_parameters.ki_pll = 4.88590411490234;
	ode_parameters.k_ff = 0.0;

	PVplant old_model("PV_old", "AC1", pv_parameters);
	PVgeneric ode_model("PV_ode", "AC1", ode_parameters);
	ode_model.solveEquilibrium();
	ode_model.computeABCD();

	const auto point = ode_model.operatingPoint();
	std::cout << "PVgeneric equilibrium: v_pv=" << point.v_pv
		<< " V, i_pv=" << point.i_array << " A, D=" << point.duty
		<< ", g_pv=" << point.g_pv << " S, Pconv=" << point.p_conv
		<< " W, Qpcc=" << point.q_pcc << " var\n";
	std::cout << "PVplant linearises at the ideal-diode maximum-power point"
		<< " (boost on voltage, no Q loop).\n";

	std::filesystem::create_directories("files");
	constexpr int kPoints = 120;
	old_model.writeFile(1.0, 1000.0, kPoints);
	ode_model.writeFile(1.0, 1000.0, kPoints);
	std::cout << "Wrote files/PV_old.csv and files/PV_ode.csv (" << kPoints << " points).\n";
}
