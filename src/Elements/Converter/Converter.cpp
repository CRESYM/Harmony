/**
 * @file Converter.cpp
 * @brief Implementation of Base class for power electronic converters with state-space models.
 */
#include "Converter.h"
#include "ui/Visualization.h"

#include <fstream>


Converter::~Converter() = default;

void Converter::dumpLinearizationOp(const std::string& id) const
{
	const double v_ln_pk = 380e3 * std::sqrt(2.0 / 3.0);
	const double vgd = V_m * std::cos(theta);
	const double vgq = -V_m * std::sin(theta);
	double id_a = 0.0, iq = 0.0, isz = 0.0;
	const int n = static_cast<int>(equilibrium_state.size());
	const int p = n - 12;
	if (p >= 0 && n >= p + 3) {
		id_a = equilibrium_state(p);
		iq = equilibrium_state(p + 1);
		isz = equilibrium_state(p + 2);
	}
	const double pac_ss = 1.5 * (vgd * id_a + vgq * iq);
	const double qac_ss = 1.5 * (vgq * id_a - vgd * iq);
	const double pdc_ss = 3.0 * V_dc * isz;
	const double vac_pu = (v_ln_pk > 0.0) ? V_m / v_ln_pk : 0.0;
	std::cout << "OP_MMC id=" << id
		<< " Pac_MW=" << P / 1e6
		<< " Qac_MVAR=" << Q / 1e6
		<< " Pdc_MW=" << P_dc / 1e6
		<< " Vac_pu=" << vac_pu
		<< " theta_deg=" << theta * 180.0 / M_PI
		<< " Vdc_kV=" << V_dc / 1e3
		<< " Vgd_kV=" << vgd / 1e3
		<< " Vgq_kV=" << vgq / 1e3
		<< " Id_A=" << id_a
		<< " Iq_A=" << iq
		<< " iSz_A=" << isz
		<< " Pac_ss_MW=" << pac_ss / 1e6
		<< " Qac_ss_MW=" << qac_ss / 1e6
		<< " Pdc_ss_MW=" << pdc_ss / 1e6
		<< "\n";
	std::ofstream csv("./files/harmony_linearization_op.csv", std::ios::app);
	if (csv)
		csv << id << "," << P / 1e6 << "," << Q / 1e6 << "," << P_dc / 1e6 << ","
			<< vac_pu << "," << theta * 180.0 / M_PI << "," << V_dc / 1e3 << ","
			<< vgd / 1e3 << "," << vgq / 1e3 << "," << id_a << "," << iq << ","
			<< isz << "," << pac_ss / 1e6 << "," << qac_ss / 1e6 << ","
			<< pdc_ss / 1e6 << "\n";
}


/**
 * @brief Check system stability by evaluating eigenvalues of the A matrix.
 *
 * Computes eigenvalues of the state matrix `A_matrix` and reports whether the
 * linearized system around the operating point is stable, unstable, or
 * marginally stable. A system is considered unstable if any eigenvalue has a
 * positive real part. The function prints a short summary to stdout.
 */
void Converter::checkStability() const {
    Eigen::EigenSolver<Eigen::MatrixXd> es(A_matrix);

    bool stable = true;
    for (int i = 0; i < es.eigenvalues().size(); ++i) {
        if (es.eigenvalues()(i).real() > 0) {
            stable = false;
            break;
        }
    }

    if (stable) {
        std::cout << "System is STABLE around this operating point.\n";
    }
    else if (es.eigenvalues().real().maxCoeff() > 0) {
        std::cout << "System is UNSTABLE around this operating point.\n";
    }
    else {
        std::cout << "System is MARGINALLY STABLE or needs further analysis.\n";
    }
}

/**
 * @brief Print eigenvalues of the A matrix.
 *
 * Calculates and prints the full set of eigenvalues of `A_matrix` to stdout.
 */
void Converter::printEigenvalues() const {
    Eigen::EigenSolver<Eigen::MatrixXd> es(A_matrix);
    std::cout << "Eigenvalues:\n" << es.eigenvalues() << "\n";
}


/**
 * @brief Plot eigenvalues in the complex plane.
 *
 * Collects eigenvalues of `A_matrix` and calls the helper `plot_eigenvalues`
 * to render them in a graphical or saved-output form. The plot title
 * contains the converter symbol for identification.
 */
void Converter::plotEigenvalues() {
    std::vector<std::complex<double>> eigvals;
    Eigen::EigenSolver<Eigen::MatrixXd> es(A_matrix);
    for (int i = 0; i < es.eigenvalues().size(); ++i) {
        eigvals.push_back(es.eigenvalues()(i));
    }
    plot_eigenvalues_implot(eigvals, "Eigenvalues of Converter " + element_symbol);
}

void Converter::plotParticipationFactors() {
    Eigen::MatrixXd P = computeParticipationFactors(A_matrix);
    Eigen::EigenSolver<Eigen::MatrixXd> es(A_matrix);
    // Plotting code for participation factors can be added here
    //std::cout << "Participation Factors:\n" << P << "\n";

    // Make labels for states and modes
	std::vector<std::string> state_labels;
    for (const auto& control : controls) {
		int n = control.second->getNumberOfSignals();
		if (control.first == "pll") n = 2; // PLL has always 2 states
		if (control.first == "gfm") n = 3; // theta, Pac_f, Qac_f
        for (int i = 0; i < n; ++i) {
            state_labels.push_back(control.first + "_" + to_string(i + 1));
        } 
	}
    for (const auto& filter : filters) {
        for (int i = 0; i < filter.second->getFilterSize(); ++i) {
            state_labels.push_back(filter.first + "_" + to_string(i + 1));
        }
    }
    // Add delay states if applicable
    if (t_delay > 0) {
        for (int i = 0; i < 5*pade_order; ++i) {
            state_labels.push_back("t_d_" + to_string(i + 1));
        }
	}	
	state_labels.push_back(u8"iDelta_d"); state_labels.push_back(u8"iDelta_q");
	state_labels.push_back(u8"iSigma_z"); state_labels.push_back(u8"iSigma_d"); state_labels.push_back(u8"iSigma_q");
	state_labels.push_back(u8"vDelta_d"); state_labels.push_back(u8"vDelta_q"); state_labels.push_back(u8"vDelta_{Zd}"); state_labels.push_back(u8"vDelta_{Zq}");
	state_labels.push_back(u8"vSigma_d"); state_labels.push_back(u8"vSigma_q"); state_labels.push_back(u8"vSigma_z");

    std::vector<std::string> mode_labels;
    for (auto eigval : es.eigenvalues()) {
        mode_labels.push_back(std::to_string(eigval.real()) + "+" + std::to_string(eigval.imag()) + "j");
    }
	plot_participation_factors_implot(matrixToVector(P), state_labels, mode_labels, "Participation Factors of Converter " + element_symbol);
}   
