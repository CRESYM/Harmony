#ifndef _STABILITY_ESTIMATE_H
#define _STABILITY_ESTIMATE_H

/**
 * @file Stability_estimate.h
 * @brief Small-signal stability and impedance analysis for AC/DC areas.
 *
 * Computes multi-port equivalent admittances and MIMO transfer functions for
 * converter-interfaced subnetworks via a unified MNA that stamps both passive
 * grid elements and converter Y-parameters into a single system matrix.
 * Supports Bode and Nyquist plotting and file export over a swept frequency range.
 *
 * Algorithm (Lekic et al., CIGRE Paris 2026, eqs. 10-17):
 *   Step 1  Partition the network into AC grids, DC grids, and converters.
 *   Step 2  Build the multi-port Y-parameters of each passive AC/DC subnetwork
 *           via adjusted MNA (eq. 2 of the paper).
 *   Step 3  For every converter that is NOT the device-under-test, stamp its
 *           full Y-matrix into a unified MNA together with the passive AC grid
 *           it connects to, and solve for the equivalent DC-side admittance
 *           Y_eq,conv (generalised eqs. 10-12).
 *   Step 4  Close the DC grid around the converter under test (eq. 13-14).
 *   Step 5  Form the MIMO transfer function H = Y_n * Z_eq (eqs. 15-17).
 *
 * The implementation is fully dimension-agnostic: all port sizes are read from
 * Bus::getPinNumber(), so 1-pin (DC scalar), 2-pin (dq AC) and 3-pin (abc AC)
 * buses are handled identically.
 */

#include "../../Constants.h"
#include "../Helper_Functions/Symbolic_functions.h"
#include "../../SubNetwork.h"

class Bus;
class Element;
class Network;
class SubNetwork;
class Converter;

/**
 * @class StabilityEstimate
 * @brief Performs impedance-based MIMO stability assessment of AC/MTDC networks.
 */
class StabilityEstimate {
public:
    StabilityEstimate() = default;
    ~StabilityEstimate() = default;

    /**
     * @brief Populate AC/DC area maps from the network hierarchy.
     * @param net Network whose area decomposition is used.
     */
    void add_areas(Network* net);

    /**
     * @brief Multi-port admittance parameters of a passive subnetwork.
     *
     * Implements adjusted MNA eq. (2): excites each output port in turn with
     * unit voltage (all others shorted) and reads back the resulting currents.
     * Converters connected to the subnetwork are excluded automatically.
     *
     * @param subnet  Subnetwork to analyse.
     * @param frequency  Evaluation frequency in Hz.
     * @return  Y-parameter matrix of size (sum_pins × sum_pins) where sum_pins
     *          is the total number of pins across all output buses.
     */
    MatrixXcd compute_equivalent_admittance_parameters_num(SubNetwork* subnet, double frequency);

    /**
     * @brief MIMO transfer function at a single frequency.
     *
     * Cuts the network at @p location of @p converter_name and returns
     * H(jω) = Y_n(jω) · Z_eq(jω).
     *
     * The cut side determines which port is excited:
     *   - DC location → H is (p_dc × p_dc) where p_dc = DC bus pin count.
     *   - AC location → H is (p_ac × p_ac) where p_ac = AC bus pin count.
     *
     * All other converters are eliminated into the DC closing admittance via the
     * unified AC+converter MNA (eqs. 10-12 generalised to arbitrary pin counts).
     *
     * @param converter_name  Name of the converter element.
     * @param location  Bus name or area prefix identifying the cut side.
     * @param frequency  Evaluation frequency in Hz.
     * @return  Complex transfer function matrix H(jω).
     */
    MatrixXcd compute_transfer_function(string converter_name, string location, double frequency);

    /**
     * @brief Write transfer function data to a CSV file over a frequency sweep.
     */
    void writeFileTF(string converter_name, string location,
                     double start_frequency, double end_frequency, int number_of_points);

    /**
     * @brief Bode plot of the transfer function over a frequency sweep.
     */
    void bodeplotTF(string converter_name, string location,
                    double start_frequency, double end_frequency, int number_of_points);

    /**
     * @brief Nyquist plot of the transfer function over a frequency sweep.
     */
    void nyquistplotTF(string converter_name, string location,
                       double start_frequency, double end_frequency, int number_of_points);

    /** @brief Print a summary of identified AC/DC areas and converters. */
    void print_summary() const;

    std::unordered_map<std::string, SubNetwork*>& get_ac_grids() { return ac_grids; }
    std::unordered_map<std::string, SubNetwork*>& get_dc_grids() { return dc_grids; }

private:
    std::vector<std::string> ac_grid_names;
    std::vector<std::string> dc_grid_names;

    std::unordered_map<std::string, SubNetwork*> ac_grids;
    std::unordered_map<std::string, SubNetwork*> dc_grids;
    std::unordered_map<std::string, Element*>    converters;

    /**
     * @brief Compute the equivalent DC-side admittance of one converter by
     *        stamping the converter Y-matrix into the passive AC-grid MNA.
     *
     * Generalises eqs. (10)-(12) of Lekic et al. to arbitrary port sizes:
     *   Y_eq,conv = Y_dc + B · (Y_eq,AC − Y_dq)^{-1} · A
     * where B, A, Y_dq, Y_dc are the appropriate sub-blocks of the converter
     * 3-terminal Y-matrix, and Y_eq,AC is the multi-port passive AC admittance.
     *
     * @param conv       Converter element.
     * @param ac_subnet  AC subnetwork the converter connects to.
     * @param frequency  Evaluation frequency in Hz.
     * @return  Equivalent DC-side admittance matrix (p_dc × p_dc).
     */
    MatrixXcd computeConverterDcAdmittance(Converter* conv,
                                            SubNetwork* ac_subnet,
                                            double frequency);

    /**
     * @brief Closing impedance seen at @p bus_name of the DC subnetwork when
     *        all other ports are terminated with @p Y_closing.
     *
     * Implements eqs. (13)-(14): partitions the DC Y-parameter matrix around
     * the input port and applies the Schur complement.
     *
     * @param sub         DC subnetwork.
     * @param bus_name    Name of the input (main-converter) DC bus.
     * @param Y_param     Multi-port DC admittance matrix.
     * @param Y_closing   Block-diagonal closing admittance from other converters.
     * @return  Equivalent closing impedance matrix (p × p).
     */
    MatrixXcd compute_closing_impedance(SubNetwork* sub, string& bus_name,
                                        MatrixXcd& Y_param, MatrixXcd& Y_closing);
};

#endif
