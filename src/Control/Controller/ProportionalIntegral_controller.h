#ifndef PROPORTIONALINTEGRAL_CONTROLLER_H
#define PROPORTIONALINTEGRAL_CONTROLLER_H

#include "Controller.h"

/**
 * @file ProportionalIntegral_controller.h
 * @brief Proportional–integral (PI) controller.
 * @ingroup control
 */

/**
 * @brief PI controller with optional damping and bandwidth metadata.
 *
 * Output per channel: Kp * error + Ki * integrator_state + c.
 */
class ProportionalIntegralController : public Controller {
public:
	/**
	 * @brief Construct a PI controller.
	 * @param symbol Controller symbol/name.
	 * @param value Gain vector containing Kp and Ki (exactly two elements).
	 * @param number_signals Number of control channels.
	 * @param refs Reference values per channel (default zero).
	 */
	ProportionalIntegralController(const std::string& symbol, const std::vector<double>& value, int number_signals, std::vector<double> refs = { 0 })
		: Controller(symbol, value, number_signals, refs) {
		if (value.size() != 2) {
			throw std::invalid_argument("Controller values must contain at least Kp and Ki.");
		}
		Kp = value[0]; // Proportional gain
		Ki = value[1]; // Integral gain
		zeta = 0.0;    // Default damping ratio
		bandwidth = 0.0; // Default bandwidth
	}

	/**
	 * @brief Replace Kp, Ki, damping ratio, and bandwidth.
	 * @param newKp New proportional gain.
	 * @param newKi New integral gain.
	 * @param newZeta New damping ratio.
	 * @param newBandwidth New bandwidth (Hz).
	 */
	void updateParameters(double newKp, double newKi, double newZeta, double newBandwidth);


	/**
	 * @brief Vector PI-controller dynamics.
	 * @param x Integrator state vector.
	 * @param u Measured input vector.
	 * @param c Feed-forward output vector.
	 * @return Concatenated state derivatives and controller outputs.
	 */
	virtual Eigen::VectorXd define_equations(const Eigen::VectorXd& x, const Eigen::VectorXd& u, const Eigen::VectorXd& c) override;

	/**
	 * @brief Scalar PI-controller dynamics.
	 * @param x Integrator state.
	 * @param u Measured input value.
	 * @param c Feed-forward output value.
	 * @return Vector containing state derivative and controller output.
	 */
	virtual Eigen::VectorXd define_equations(const double x, const double u, const double c) override;

	/** @brief Print controller type, gains, damping, bandwidth, and references. */
	virtual void printValues() override {
		std::cout << "  Controller type: PI \n"
			<< "  Proportional Gain (Kp): " << Kp << "\n"
			<< "  Integral Gain (Ki): " << Ki << "\n"
			<< "  Damping Ratio (zeta): " << zeta << "\n"
			<< "  Bandwidth: " << bandwidth << " Hz\n"
			<< "  Reference Values: ";
		for (const auto& ref : reference) {
			std::cout << ref << " ";
		}
		std::cout << std::endl;
	}
	
	/** @brief Return the proportional gain Kp. */
	double getKp() const {
		return Kp; // Get proportional gain
	}

	/** @brief Return the integral gain Ki. */
	double getKi() const {
		return Ki; // Get integral gain
	}

	/** @brief Return the damping ratio. */
	double getZeta() const {
		return zeta; // Get damping ratio
	}

	/** @brief Return the bandwidth (Hz). */
	double getBandwidth() const {
		return bandwidth; // Get bandwidth
	}

	/** @brief Return `{ Kp, Ki, zeta, bandwidth }`. */
	virtual vector<double> getParameters() const override {
		return { Kp, Ki, zeta, bandwidth }; // Return controller parameters
	}

	/** @brief Set parameters from `{ Kp, Ki, zeta, bandwidth }`. @param params Four-element parameter vector. */
	virtual void setParameters(const vector<double>& params) override {
		if (params.size() != 4) {
			throw std::invalid_argument("Parameter vector must contain Kp, Ki, zeta, and bandwidth.");
		}
		Kp = params[0];
		Ki = params[1];
		zeta = params[2];
		bandwidth = params[3];
	}

private:
	double Kp;                     // Proportional gain
	double Ki;                     // Integral gain
	double zeta;                   // Damping ratio
	double bandwidth;              // Bandwidth
};

#endif // PROPORTIONALINTEGRAL_CONTROLLER_H
