#ifndef _STABILITY_ESTIMATE_H
#define _STABILITY_ESTIMATE_H

/**
 * @file Stability_estimate.h
 * @brief Small-signal stability and impedance analysis for AC/DC areas.
 *
 * Computes equivalent admittances, closing impedances, and frequency-domain
 * transfer functions for converter-interfaced subnetworks. Supports Bode and
 * Nyquist plotting and file export over a swept frequency range.
 */

#include "../../Constants.h"
#include "../Helper_Functions/Symbolic_functions.h"
#include "../../SubNetwork.h"

class Bus;
class Element;
class Network;
class SubNetwork;

/**
 * @class StabilityEstimate
 * @brief Performs impedance-based stability assessment of network areas.
 *
 * Partitions a Network into AC grids, DC grids, and converter subnetworks,
 * then evaluates equivalent admittance parameters and transfer functions
 * at specified frequencies for stability margin analysis.
 */
class StabilityEstimate {
public:
    /** @brief Default constructor. */
	StabilityEstimate() = default;

    /** @brief Default destructor. */
	~StabilityEstimate() = default;

    /**
     * @brief Populates AC/DC area maps from the network hierarchy.
     * @param net Network whose area decomposition is used.
     */
	void add_areas(Network* net);

    /**
     * @brief Computes numeric equivalent admittance parameters for a subnetwork.
     * @param subnet Subnetwork whose Y-parameters are evaluated.
     * @param frequency Evaluation frequency in Hz.
     * @return Complex admittance matrix at @p frequency.
     */
	MatrixXcd compute_equivalent_admittance_parameters_num(SubNetwork* subnet, double frequency);

    /**
     * @brief Computes the visible impedance when other subnetwork outputs are shorted.
     * @param subnet Subnetwork under test.
     * @param location Output port identifier (modified in place if needed).
     * @param Y_param Admittance matrix of the subnetwork.
     * @param Z_param Impedance matrix of the subnetwork.
     * @return Closing impedance seen at @p location.
     */
	MatrixXcd compute_closing_impedance(SubNetwork*, string&, MatrixXcd&, MatrixXcd&);

    /**
     * @brief Evaluates a converter transfer function at a single frequency.
     * @param converter_name Name of the converter element.
     * @param location Output port or bus identifier.
     * @param frequency Evaluation frequency in Hz.
     * @return Complex transfer function value.
     */
	MatrixXcd compute_transfer_function(string converter_name, string location, double frequency);

    /**
     * @brief Writes transfer function data to a file over a frequency sweep.
     * @param converter_name Name of the converter element.
     * @param location Output port or bus identifier.
     * @param start_frequency Sweep start frequency in Hz.
     * @param end_frequency Sweep end frequency in Hz.
     * @param number_of_points Number of logarithmically spaced frequency points.
     */
	void writeFileTF(string converter_name, string location, double start_frequency, double end_frequency, int number_of_points);

    /**
     * @brief Plots the transfer function as a Bode diagram over a frequency sweep.
     * @param converter_name Name of the converter element.
     * @param location Output port or bus identifier.
     * @param start_frequency Sweep start frequency in Hz.
     * @param end_frequency Sweep end frequency in Hz.
     * @param number_of_points Number of frequency points.
     */
	void bodeplotTF(string converter_name, string location, double start_frequency, double end_frequency, int number_of_points);

    /**
     * @brief Plots the transfer function as a Nyquist diagram over a frequency sweep.
     * @param converter_name Name of the converter element.
     * @param location Output port or bus identifier.
     * @param start_frequency Sweep start frequency in Hz.
     * @param end_frequency Sweep end frequency in Hz.
     * @param number_of_points Number of frequency points.
     */
	void nyquistplotTF(string converter_name, string location, double start_frequency, double end_frequency, int number_of_points);

    /** @brief Prints a summary of identified AC/DC areas and converters. */
	void print_summary() const;

    /**
     * @brief Returns the map of AC grid subnetworks by name.
     * @return Reference to the AC grids map.
     */
	std::unordered_map<std::string, SubNetwork*>& get_ac_grids() { return ac_grids; }

    /**
     * @brief Returns the map of DC grid subnetworks by name.
     * @return Reference to the DC grids map.
     */
	std::unordered_map<std::string, SubNetwork*>& get_dc_grids() { return dc_grids; }

private:
	std::vector<std::string> ac_grid_names;
	std::vector<std::string> dc_grid_names;

	std::unordered_map<std::string, SubNetwork*> ac_grids;
	std::unordered_map<std::string, SubNetwork*> dc_grids;
	std::unordered_map<std::string, Element*> converters;

	void compute_equivalent_impedance(Network* net, std::vector<Bus*> start_buses, std::vector<Bus*> end_buses, std::vector<Element*> skip_elements);
	MatrixXcd compute_equivalent_impedance_num(Network* net, std::vector<Bus*> start_buses, std::vector<Bus*> end_buses, std::vector<Element*> skip_elements, double frequency);

};


#endif
