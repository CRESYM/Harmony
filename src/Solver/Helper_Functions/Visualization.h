#ifndef _VISUALIZATION_H_
#define _VISUALIZATION_H_

#include "../../Constants.h"

// ============================================================
// CORE CONTROL
// ============================================================

void visualization_stop();
bool visualization_is_running();

/// Block the calling thread until the user closes the window.
/// Call this at the end of main() to prevent the process from
/// exiting while the GUI is still open.
void visualization_wait();

/// Schedule a PNG capture of the named tab on the next rendered frame.
/// Written to "<tab_title>.png" in the working directory.
/// Thread-safe — can be called from any thread at any time.
void visualization_save_tab(const std::string& tab_title);

// ============================================================
// EXTEND — register a custom draw tab from external modules
// ============================================================

/// Register an arbitrary ImGui/ImPlot draw callback as a tab.
/// The GUI thread is auto-started on the first call.
/// fn() is called every frame while the tab is active.
void add_plot_tab(const std::string& title, std::function<void()> fn);

// ============================================================
// BUILT-IN PLOTS  (auto-start visualization on first call)
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

// ============================================================
// OPF VISUALISATION
// ============================================================

/// All solver outputs needed to render the AC/DC OPF network.
struct OPFVisualData {

    // ---- AC topology ----
    Eigen::MatrixXd bus_entire_ac;    // [bus_id, Pd, Qd, ..., area]
    Eigen::MatrixXd branch_entire_ac; // [from, to, ..., area]
    Eigen::MatrixXd gen_entire_ac;    // [bus_id, Pg, Qg, ..., area]

    // ---- DC topology ----
    Eigen::MatrixXd bus_dc;           // [bus_id, ...]
    Eigen::MatrixXd branch_dc;        // [from, to, ...]
    Eigen::MatrixXd conv_dc;          // [dc_bus, ac_bus, area, ...]

    // ---- DC solution ----
    Eigen::VectorXd vn2_dc_k;         // squared voltage (DC buses)
    Eigen::VectorXd ps_dc_k;          // converter active power (p.u.)
    Eigen::VectorXd qs_dc_k;          // converter reactive power (p.u.)
    Eigen::MatrixXd pij_dc_k;         // DC branch active power (p.u.)

    // ---- AC solution (per sub-grid) ----
    std::vector<int> nbuses_ac;
    std::vector<int> ngens_ac;

    std::vector<Eigen::VectorXd> vn2_ac_k;
    std::vector<Eigen::VectorXd> pgen_ac_k;
    std::vector<Eigen::VectorXd> qgen_ac_k;

    std::vector<Eigen::MatrixXd> pij_ac_k;
    std::vector<Eigen::MatrixXd> qij_ac_k;

    // ---- System parameters ----
    int    nconvs_dc;
    int    nbuses_dc;
    int    ngrids;

    double baseMVA_ac;
    double baseMW_dc;
    double pol_dc;
};

/// Registers an "AC/DC OPF" tab in the shared visualization window.
/// Returns immediately — the window stays open until the user closes it
/// or visualization_stop() / visualization_wait() is called.
void viz_opf(const OPFVisualData& data);

#endif // _VISUALIZATION_H_
