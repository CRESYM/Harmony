/**
 * @file Converter.cpp
 * @brief Implementation of Base class for power electronic converters with state-space models.
 */
#include "Converter.h"


Converter::~Converter() {
    for (auto& [name, controller] : controls)
        delete controller;
    controls.clear();

    for (auto& [name, filter] : filters)
        delete filter;
    filters.clear();
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
    for (auto control : controls) {
		int n = control.second->getNumberOfSignals();
		if (control.first == "pll") n = 2; // PLL has always 2 states
        for (int i = 0; i < n; ++i) {
            state_labels.push_back(control.first + "_" + to_string(i + 1));
        } 
	}
    for (auto filter : filters) {
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
