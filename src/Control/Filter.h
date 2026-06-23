#ifndef FILTER_H
#define FILTER_H

#include "Control_block.h"

/**
 * @file Filter.h
 * @brief First- and second-order filter control block.
 * @ingroup control
 */

/**
 * @brief LTI filter with configurable time constant, damping, and bandwidth.
 *
 * Supports first- and second-order dynamics for vector or scalar signals.
 */
class Filter : public ControlBlock {
public:
	/**
	 * @brief Construct a filter from symbolic metadata and parameter values.
	 * @param symbol Element symbol/name.
	 * @param type Filter type identifier (e.g. `"LP"`).
	 * @param order Filter order (1 or 2).
	 * @param values Parameter vector: time constant, damping ratio, bandwidth (Hz).
	 * @param number_of_connections Number of parallel filter channels (pins).
	 */
    Filter(const std::string& symbol, const std::string& type, int order, vector<double> values, int number_of_connections)
		: ControlBlock(), filter_symbol(symbol), filter_size(number_of_connections), filter_type(type), filter_order(order) {
        if (values.size() != 3) {
            throw std::invalid_argument("Filter values must contain time constant, damping ratio, and bandwidth.");
        }
        timeConstant = values[0]; // Time constant (T)
        zeta = values[1];         // Damping ratio (ζ)
		bandwidth = values[2];    // Bandwidth
    }

    /** @brief Print filter time constant, damping ratio, and bandwidth. */
    virtual void printValues() override {
        std::cout << "Filter Parameters:"
            << "  Time Constant (T): " << timeConstant << "\n"
            << "  Damping Ratio (zeta): " << zeta << "\n"
            << "  Bandwidth: " << bandwidth << " Hz\n";
    }

	/**
	 * @brief Replace filter time constant, damping ratio, and bandwidth.
	 * @param newTimeConstant New time constant.
	 * @param newZeta New damping ratio.
	 * @param newBandwidth New bandwidth (Hz).
	 */
    void updateParameters(double newTimeConstant, double newZeta, double newBandwidth);

	/**
	 * @brief State derivatives for a vector-valued filter.
	 * @param x Filter state vector.
	 * @param u Input vector.
	 * @return Concatenated state derivatives and filter outputs.
	 */
    VectorXd define_differential_equations(const VectorXd& x, const VectorXd& u);

	/**
	 * @brief State derivatives for a scalar filter with vector state.
	 * @param x Filter state vector.
	 * @param u Scalar input.
	 * @return Concatenated state derivatives and filter outputs.
	 */
    VectorXd define_differential_equations(const VectorXd& x, const double u);

	/**
	 * @brief State derivatives for a scalar filter.
	 * @param x Scalar filter state.
	 * @param u Scalar input.
	 * @return Concatenated state derivative and filter output.
	 */
    VectorXd define_differential_equations(const double x, const double u);

	/** @brief Total number of filter states (`filter_size * filter_order`). */
    int getFilterSize() const {
        return filter_size*filter_order;
	}

private:
	string filter_symbol; // Symbol for the filter
	string filter_type = "LP";   // Type of filter (e.g., LPF, HPF, BPF)
	int filter_order;   // Filter order, for now can be only the 1st and the 2nd
	int filter_size;      // Number of pins for the filter
    double timeConstant; // Time constant
    double zeta;         // Damping ratio
    double bandwidth;    // Bandwidth
};

#endif // FILTER_H
