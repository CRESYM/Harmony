#ifndef _VISUALIZATION_H_
#define _VISUALIZATION_H_

#include "../../Constants.h"

extern void bode_plot(const std::vector<double>& freq,
	const std::vector<std::vector<double>>& mag_dB, const std::vector<std::vector<double>>& phase_deg,
	const std::vector<std::string>& labels, const std::string& title = "Bode Plot");

extern void nyquist_plot(const std::vector<std::vector<std::complex<double>>>& H_data,
	const std::vector<std::string>& labels, const std::string& title = "Nyquist Plot");

extern void plot_eigenvalues(const std::vector<std::complex<double>>& eigvals,
	const std::string& title = "Eigenvalues");

extern void plot_eigenvalue_sequence(const std::vector<std::vector<std::complex<double>>>& eigval_sequence,
	const std::string& title = "Eigenvalus Sequence");

extern void plot_participation_factors_normalized(const std::vector<std::vector<double>>& P,
	const std::vector<std::string>& state_labels,
	const std::vector<std::string>& mode_labels,
	const std::string& title);

// Time domain waveforms for abc groups. Xabc should have 3 columns per group (xa, xb, xc).
extern void plot_abc_waveforms(const std::vector<double>& t, const Eigen::MatrixXd& Xabc, const std::string& title);
extern void plot_abc_groups(const std::vector<double>& t, const std::vector<Eigen::MatrixXd>& Xabc_groups, const std::string& title);

#endif // _VISUALIZATION_H