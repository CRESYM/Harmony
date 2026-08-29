/**
 * @file example_MMC_gfm.cpp
 * @brief GFM MMC mode: equilibrium, ABCD/Y, and StabilityEstimate reuse.
 */
#include "Examples.h"

#include "../network.h"
#include "../Bus.h"
#include "../Include_components.h"
#include "../Solver/Stability_Estimate/Stability_estimate.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>

void example_MMC_gfm(bool plotting_enabled /*=true*/) {
	const double f = 50.0;
	const double omega = 2.0 * M_PI * f;
	const double Vdc = 240e3;
	const double Vm = 100e3;
	const double Pac = 100e6;
	const double Qac = 0.0;
	const double Larm = 50e-3;
	const double Rarm = 1.07;
	const double Lf = 60e-3;
	const double Rf = 0.535;
	const double Reqac = Rf + Rarm / 2.0;
	const double Leqac = Lf + Larm / 2.0;

	const double Vgd = Vm;
	const double Vgq = 0.0;
	const double Id = (2.0 / 3.0) * (Pac * Vgd + Qac * Vgq) / (Vgd * Vgd + Vgq * Vgq);
	const double Iq = (2.0 / 3.0) * (Pac * Vgq - Qac * Vgd) / (Vgd * Vgd + Vgq * Vgq);
	const double vMd0 = Vgd + Reqac * Id + omega * Leqac * Iq;
	const double vMq0 = Vgq + Reqac * Iq - omega * Leqac * Id;
	const double Egfm_ref = std::hypot(vMd0, vMq0);
	const double theta_gfm0 = std::atan2(vMq0, vMd0);

	const double Kdroop_P = 2.0 * M_PI * 0.5 / 100e6;
	const double Kdroop_Q = 0.02 * Egfm_ref / 50e6;
	const double Tf_P = 20e-3;
	const double Tf_Q = 20e-3;

	const double zeta = 0.7;
	const double w0_ccc = 300.0;
	const double Ki_zcc = Larm * w0_ccc * w0_ccc;
	const double Kp_zcc = 2.0 * zeta * std::sqrt(Ki_zcc * Larm) - Rarm;

	std::vector<double> converter_params = {
		omega, Pac, Qac, 0.0, Vm, Pac, Vdc,
		Larm, Rarm, 0.01, 50, Lf, Rf, 0.0
	};

	// Controllers: pll, dc_v, P, Vac, Q, energy, zcc, occ, ccc, droop, gfm
	// MVP twin of MATLAB compare_gfm_mvp.m (energy+zcc+gfm; no OCC/CCC/virtual Z).
	std::vector<double> controller_params = {
		0, 0, 0, 0, 0,
		1, 0, 120, 400, 1, 0, // energy
		1, 0, Kp_zcc, Ki_zcc, 1, Pac / (3.0 * Vdc), // zcc
		0, // occ
		0, // ccc
		0, // droop
		1, 0, Kdroop_P, Kdroop_Q, 4, Tf_P, Tf_Q, 0.0, 0.0 // gfm
	};

	MMC* mmc = new MMC("MMC_GFM", "AC1_DC1", converter_params, controller_params);

	std::cout << "\n=== GFM MMC equilibrium ===\n";
	std::cout << std::setprecision(10)
		<< "MATLAB-aligned seed: Egfm_ref=" << Egfm_ref
		<< " theta0=" << theta_gfm0
		<< " Id=" << Id << " Iq=" << Iq << "\n";

	mmc->solveEquilibrium();
	const Eigen::VectorXd x_eq = mmc->getEquilibriumState();
	std::cout << std::setprecision(6) << "Equilibrium state size: " << x_eq.size() << "\n";

	const int plant = static_cast<int>(x_eq.size()) - 12;
	const double Id_h = x_eq(plant + 0);
	const double Iq_h = x_eq(plant + 1);
	const double iSz_h = x_eq(plant + 2);
	const double Pac_h = 1.5 * (Vgd * Id_h + Vgq * Iq_h);
	const double Qac_h = 1.5 * (-Vgd * Iq_h + Vgq * Id_h);

	// Controller order: energy(1), zcc(1), gfm(3) at indices 0..4
	const double theta_h = x_eq(2);
	const double Pac_f_h = x_eq(3);
	const double Qac_f_h = x_eq(4);
	const double Egfm_h = Egfm_ref + Kdroop_Q * (Qac_f_h - Qac);

	std::cout << std::setprecision(10)
		<< "Harmony SS: Id=" << Id_h << " Iq=" << Iq_h
		<< " iSigma_z=" << iSz_h
		<< " Pac=" << Pac_h << " Qac=" << Qac_h
		<< " theta=" << theta_h << " Egfm=" << Egfm_h << "\n";

	{
		Eigen::Vector3d u;
		u << Vdc, Vgd, Vgq;
		const Eigen::VectorXd dx = mmc->computeStateDerivatives(x_eq, u);
		std::cout << "Closed-loop residual inf-norm: " << dx.lpNorm<Eigen::Infinity>() << "\n";
	}

	std::filesystem::create_directories("files");
	{
		std::ofstream csv("files/harmony_gfm_mvp_metrics.csv");
		csv << "name,value\n";
		csv << std::setprecision(12)
			<< "Id," << Id_h << "\n"
			<< "Iq," << Iq_h << "\n"
			<< "iSigma_z," << iSz_h << "\n"
			<< "Pac," << Pac_h << "\n"
			<< "Qac," << Qac_h << "\n"
			<< "theta_gfm," << theta_h << "\n"
			<< "Egfm," << Egfm_h << "\n"
			<< "Egfm_ref," << Egfm_ref << "\n"
			<< "theta_gfm0," << theta_gfm0 << "\n"
			<< "Kdroop_P," << Kdroop_P << "\n"
			<< "Kdroop_Q," << Kdroop_Q << "\n"
			<< "Vdc," << Vdc << "\n"
			<< "Vm," << Vm << "\n";
	}

	mmc->computeABCD();
	auto A = mmc->getA();
	auto B = mmc->getB();
	std::cout << "A is " << A.rows() << "x" << A.cols() << ", B is " << B.rows() << "x" << B.cols() << "\n";

	MatrixXcd Y = vectorToMatrix(mmc->compute_y_parameters(50.0));
	std::cout << "\nY-parameters at 50 Hz:\n" << std::setprecision(6) << Y << "\n";

	mmc->checkStability();
	mmc->printEigenvalues();

	Network net;
	Bus* bus_ac = new Bus("AC1", "AC1", 3);
	Bus* bus_dc = new Bus("DC1", "DC1", 2);
	net.addBus(bus_ac);
	net.addBus(bus_dc);
	net.addElement(mmc);
	net.connectElementToBus(mmc, 1, bus_ac);
	net.connectElementToBus(mmc, 2, bus_dc);

	std::vector<double> Zsrc = { 0.1, 0.1, 0.1 };
	AC_source* src = new AC_source("G1", "AC1", 3, Vm, Zsrc);
	net.addElement(src);
	net.connectElementToBus(src, 1, bus_ac);

	net.add_areas();
	StabilityEstimate stability;
	stability.add_areas(&net);
	stability.print_summary();

	try {
		MatrixXcd TF = stability.compute_transfer_function("MMC_GFM", "AC", 100.0);
		std::cout << "\nH = Y Z_eq at 100 Hz (reuse StabilityEstimate):\n"
			<< std::setprecision(6) << TF << "\n";
		if (plotting_enabled) {
			stability.bodeplotTF("MMC_GFM", "AC", 1.0, 1000.0, 200);
		}
	}
	catch (const std::exception& ex) {
		std::cout << "StabilityEstimate note: " << ex.what() << "\n";
	}

	if (plotting_enabled) {
		mmc->plotEigenvalues();
		mmc->plotYParameters(1.0, 1000.0, 200);
	}

	std::cout << "GFM MMC example done.\n";
}
