#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "../Control_block.h"

/**
 * @file Controller.h
 * @brief Base class for reference-tracking controllers.
 * @ingroup control
 */

/**
 * @brief Abstract controller with per-channel reference values.
 *
 * Subclasses implement @ref define_equations and store gain parameters.
 */
class Controller : ControlBlock {
public:
	/**
	 * @brief Construct a controller with reference setpoints.
	 * @param symbol Controller symbol/name.
	 * @param value Gain parameter vector (interpreted by derived classes).
	 * @param number_signals Number of control channels.
	 * @param refs Reference values per channel (default zero).
	 */
    Controller(const std::string& symbol, const vector<double>& value, int number_signals, std::vector<double> refs = { 0 })
        : ControlBlock(), controller_symbol(symbol), reference(refs) {
        if (reference.size() != number_signals) {
            reference.resize(number_signals, 0.0); // Initialize reference values
        }
    }

	/**
	 * @brief Controller dynamics for vector states and signals.
	 * @param x Integrator or auxiliary state vector.
	 * @param u Measured input vector.
	 * @param c Feed-forward or constant output vector.
	 * @return Concatenated state derivatives and controller outputs.
	 */
    virtual Eigen::VectorXd define_equations(const Eigen::VectorXd& x, const Eigen::VectorXd& u, const Eigen::VectorXd& c) {
        return Eigen::VectorXd::Zero(2);
    };

	/**
	 * @brief Controller dynamics for scalar state and signals.
	 * @param x Integrator or auxiliary state.
	 * @param u Measured input value.
	 * @param c Feed-forward or constant output value.
	 * @return Concatenated state derivative and controller output.
	 */
    virtual Eigen::VectorXd define_equations(const double x, const double u, const double c) {
        return Eigen::VectorXd::Zero(2);
    };

    /** @brief Print reference values to standard output. */
   virtual void printValues() override {
        for (const auto& ref : reference) {
            std::cout << ref << " ";
        }
        std::cout << std::endl;
    }

	/** @brief Replace all reference setpoints. @param ref New reference vector. */
    void setReference(const std::vector<double>& ref) {
        for (size_t i = 0; i < ref.size(); ++i) {
            reference[i] = ref[i]; // Set reference values
		}
	}

	/**
	 * @brief Set one reference channel.
	 * @param ref New reference value.
	 * @param i Channel index.
	 */
    void setReference(const double ref, int i) {
        reference[i] = ref; // Set reference values
    }

	/** @brief Return a copy of the current reference vector. */
    std::vector<double> getReference() const {
        return reference; // Get reference values
	}

	/** @brief Return the controller symbol/name. */
    std::string getControllerSymbol() const {
        return controller_symbol; // Get controller symbol
    }

	/** @brief Return the number of control channels. */
    int getNumberOfSignals() const {
        return static_cast<int>(reference.size()); // Get number of signals
	}

	/** @brief Return controller-specific parameters (implemented by subclasses). */
    virtual vector<double> getParameters() const { return reference; }; // Pure virtual function for parameters

	/** @brief Set controller-specific parameters (implemented by subclasses). @param params Parameter vector. */
    virtual void setParameters(const vector<double>& params) {}; // Pure virtual function to set parameters

	virtual ~Controller() = default; // Virtual destructor for proper cleanup

protected: 
	std::string controller_symbol; // Symbol for the controller
	bool active = false;           // Activation status
    std::vector<double> reference; // Reference values
};

#endif // CONTROLLER_H
