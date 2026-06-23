#ifndef PROPORTIONAL_CONTROLLER_H
#define PROPORTIONAL_CONTROLLER_H

#include "Controller.h"

/**
 * @file Proportional_controller.h
 * @brief Proportional (P) controller.
 * @ingroup control
 */

/**
 * @brief Single-gain proportional controller: output = K * (reference - u) + c.
 */
class ProportionalController : public Controller {
public:
	/**
	 * @brief Construct a P controller.
	 * @param symbol Controller symbol/name.
	 * @param value Gain vector containing Kp (exactly one element).
	 * @param number_signals Number of control channels.
	 * @param refs Reference values per channel (default zero).
	 */
	ProportionalController(const std::string& symbol, const vector<double>& value, int number_signals, std::vector<double> refs = { 0 })
		: Controller(symbol, value, number_signals, refs) {
		if (value.size() != 1) {
			throw std::invalid_argument("Proportional controller values must contain Kp.");
		}
		K = value[0]; // Proportional gain
	}

	/**
	 * @brief Vector P-controller dynamics.
	 * @param x Integrator state vector (unused for pure P).
	 * @param u Measured input vector.
	 * @param c Feed-forward output vector.
	 * @return Concatenated error states and controller outputs.
	 */
	virtual Eigen::VectorXd define_equations(const Eigen::VectorXd& x, const Eigen::VectorXd& u, const Eigen::VectorXd& c) override;

	/**
	 * @brief Scalar P-controller dynamics.
	 * @param x Integrator state (unused for pure P).
	 * @param u Measured input value.
	 * @param c Feed-forward output value.
	 * @return Vector containing error state and controller output.
	 */
	virtual Eigen::VectorXd define_equations(const double x, const double u, const double c) override;

	/** @brief Print controller type, gain K, and reference values. */
	virtual void printValues() override {
		std::cout << "  Controller Type: P\n"
			<< "  Proportional Gain (K): " << K << "\n"
			<< "  Reference Values: ";
		for (const auto& ref : reference) {
			std::cout << ref << " ";
		}
		std::cout << std::endl;
	}

	/** @brief Return the proportional gain K. */
	double getK() const {
		return K; // Get proportional gain
	}

	/** @brief Set the proportional gain K. @param newK New gain value. */
	void setK(double newK) {
		K = newK; // Set new proportional gain
	}

	/** @brief Return `{ K }`. */
	virtual vector<double> getParameters() const override {
		return { K }; // Return controller parameters
	}

	/** @brief Set K from a single-element parameter vector. @param params Vector `{ K }`. */
	virtual void setParameters(const vector<double>& params) override {
		if (params.size() != 1) {
			throw std::invalid_argument("Parameter vector must contain only K.");
		}
		K = params[0];
	}

private:
	double K; // proportional gain
};

#endif // PROPORTIONAL_CONTROLLER_H
