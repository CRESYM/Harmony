#ifndef _VISUALIZATION_H_
#define _VISUALIZATION_H_

#include "../../Constants.h"

// ============================================================
// CORE CONTROL
// ============================================================

void visualization_stop();
bool visualization_is_running();

// ============================================================
// PLOTS (AUTO-START VISUALIZATION ON FIRST CALL)
// ============================================================

extern void bode_plot_implot(
    const std::vector<double>& freq,
    const std::vector<std::vector<double>>& mag_dB,
    const std::vector<std::vector<double>>& phase_deg,
    const std::vector<std::string>& labels,
    const std::string& title);

extern void nyquist_plot_implot(
    const std::vector<std::vector<std::complex<double>>>& H_data,
    const std::vector<std::string>& labels,
    const std::string& title);

extern void plot_eigenvalues_implot(
    const std::vector<std::complex<double>>& eigvals,
    const std::string& title);

extern void plot_participation_factors_implot(
    const std::vector<std::vector<double>>& P,
    const std::vector<std::string>& state_labels,
    const std::vector<std::string>& mode_labels,
    const std::string& title);

extern void plot_abc_waveforms_implot(
    const std::vector<double>& t,
    const Eigen::MatrixXd& Xabc,
    const std::string& title);

extern void plot_abc_groups_implot(
    const std::vector<double>& t,
    const std::vector<Eigen::MatrixXd>& Xabc_groups,
    const std::string& title);


#endif // _VISUALIZATION_H