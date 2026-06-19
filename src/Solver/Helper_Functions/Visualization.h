#ifndef _VISUALIZATION_H_
#define _VISUALIZATION_H_

/**
 * @file Visualization.h
 * @brief Interactive ImGui/ImPlot visualization for solver results.
 *
 * Provides a shared GUI window with built-in Bode, Nyquist, eigenvalue,
 * participation-factor, and abc waveform plots, plus OPF network rendering
 * and an extension API for custom plot tabs.
 */

#include "../../Constants.h"

/**
 * @brief Closes the visualization window and stops the GUI thread.
 */
void visualization_stop();

/**
 * @brief Use the host ImGui/GLFW context (HarmonyUI) instead of a background window.
 *
 * Call before any simulation runs. Prevents a second GLFW/ImGui context that can
 * crash with ImGui focus-scope errors.
 */
void visualization_set_embedded_mode(bool enabled);

/** @brief Returns whether plot tabs are registered. */
bool visualization_has_tabs();

/** @brief Remove all registered plot tabs (e.g. before a new run). */
void visualization_clear_tabs();

/** @brief Draw plot tabs inside the current ImGui window (embedded / HarmonyUI mode). */
void visualization_draw_embedded(const char* tabBarId = "HarmonyPlotTabs");

/** @brief Capture a pending PNG save after ImGui::Render() on @p window. */
void visualization_process_pending_save(GLFWwindow* window);

/**
 * @brief Returns whether the visualization window is currently open.
 * @return True if the GUI is running.
 */
bool visualization_is_running();

/**
 * @brief Blocks until the user closes the visualization window.
 *
 * Call at the end of main() to keep the process alive while the GUI is open.
 */
void visualization_wait();

/**
 * @brief Schedules a PNG capture of the named tab on the next rendered frame.
 * @param tab_title Title of the tab to capture (saved as "<tab_title>.png").
 */
void visualization_save_tab(
	const std::string& tab_title,
	const std::filesystem::path& output_dir = {});

/** @brief Titles of tabs currently registered in the visualization window. */
std::vector<std::string> visualization_tab_titles();

/**
 * @brief Registers a custom ImGui/ImPlot draw callback as a new tab.
 *
 * The GUI thread is auto-started on the first call. @p fn is invoked every
 * frame while the tab is active.
 *
 * @param title Tab title shown in the tab bar.
 * @param fn Draw callback executed each frame.
 */
void add_plot_tab(const std::string& title, std::function<void()> fn);

/**
 * @brief Opens a Bode magnitude/phase plot in the visualization window.
 * @param freq Frequency axis (Hz).
 * @param mag_dB Magnitude in dB, one vector per trace.
 * @param phase_deg Phase in degrees, one vector per trace.
 * @param labels Trace legend labels.
 * @param title Plot window title.
 */
extern void bode_plot_implot(
    const std::vector<double>& freq,
    const std::vector<std::vector<double>>& mag_dB,
    const std::vector<std::vector<double>>& phase_deg,
    const std::vector<std::string>& labels,
    const std::string& title);

/**
 * @brief Opens a Nyquist plot in the visualization window.
 * @param H_data Complex frequency response, one vector per trace.
 * @param labels Trace legend labels.
 * @param title Plot window title.
 */
extern void nyquist_plot_implot(
    const std::vector<std::vector<std::complex<double>>>& H_data,
    const std::vector<std::string>& labels,
    const std::string& title);

/**
 * @brief Plots eigenvalues in the complex plane.
 * @param eigvals List of eigenvalues.
 * @param title Plot window title.
 */
extern void plot_eigenvalues_implot(
    const std::vector<std::complex<double>>& eigvals,
    const std::string& title);

/**
 * @brief Plots participation factor heatmap (states vs modes).
 * @param P Participation factor matrix [state][mode].
 * @param state_labels Row labels (state names).
 * @param mode_labels Column labels (mode indices/names).
 * @param title Plot window title.
 */
extern void plot_participation_factors_implot(
    const std::vector<std::vector<double>>& P,
    const std::vector<std::string>& state_labels,
    const std::vector<std::string>& mode_labels,
    const std::string& title);

/**
 * @brief Plots three-phase abc waveforms over time.
 * @param t Time axis (s).
 * @param Xabc Waveform matrix (3 rows ù N samples).
 * @param title Plot window title.
 */
extern void plot_abc_waveforms_implot(
    const std::vector<double>& t,
    const Eigen::MatrixXd& Xabc,
    const std::string& title);

/**
 * @brief Plots multiple abc waveform groups on shared axes.
 * @param t Time axis (s).
 * @param Xabc_groups One 3ùN matrix per signal group.
 * @param title Plot window title.
 */
extern void plot_abc_groups_implot(
    const std::vector<double>& t,
    const std::vector<Eigen::MatrixXd>& Xabc_groups,
    const std::string& title);

/**
 * @brief Container for all OPF topology and solution data needed by viz_opf().
 */
struct OPFVisualData {

    Eigen::MatrixXd bus_entire_ac;
    Eigen::MatrixXd branch_entire_ac;
    Eigen::MatrixXd gen_entire_ac;

    Eigen::MatrixXd bus_dc;
    Eigen::MatrixXd branch_dc;
    Eigen::MatrixXd conv_dc;

    Eigen::VectorXd vn2_dc_k;
    Eigen::VectorXd ps_dc_k;
    Eigen::VectorXd qs_dc_k;
    Eigen::MatrixXd pij_dc_k;

    std::vector<int> nbuses_ac;
    std::vector<int> ngens_ac;

    std::vector<Eigen::VectorXd> vn2_ac_k;
    std::vector<Eigen::VectorXd> pgen_ac_k;
    std::vector<Eigen::VectorXd> qgen_ac_k;

    std::vector<Eigen::MatrixXd> pij_ac_k;
    std::vector<Eigen::MatrixXd> qij_ac_k;

    int    nconvs_dc;
    int    nbuses_dc;
    int    ngrids;

    double baseMVA_ac;
    double baseMW_dc;
    double pol_dc;
};

/**
 * @brief Registers an "AC/DC OPF" network diagram tab in the visualization window.
 *
 * Returns immediately; the window stays open until the user closes it or
 * visualization_stop() / visualization_wait() is called.
 *
 * @param data OPF topology and solved bus/branch quantities.
 */
void viz_opf(const OPFVisualData& data);

#endif // _VISUALIZATION_H_
