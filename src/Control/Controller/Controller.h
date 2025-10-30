#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "../Control_block.h"

class Controller : ControlBlock {
public:
	// Constructor takes a symbol, type, values (Kp, Ki), and number of signals
    Controller(const std::string& symbol, const vector<double>& value, int number_signals, std::vector<double> refs = { 0 })
        : ControlBlock(), controller_symbol(symbol), reference(refs) {
        if (reference.size() != number_signals) {
            reference.resize(number_signals, 0.0); // Initialize reference values
        }
    }

    virtual Eigen::VectorXd define_equations(const Eigen::VectorXd& x, const Eigen::VectorXd& u, const Eigen::VectorXd& c) {
        return Eigen::VectorXd::Zero(2);
    };
    virtual Eigen::VectorXd define_equations(const double x, const double u, const double c) {
        return Eigen::VectorXd::Zero(2);
    };

    // Override method to print element-specific values
   virtual void printValues() override {
        for (const auto& ref : reference) {
            std::cout << ref << " ";
        }
        std::cout << std::endl;
    }

    // Method to update controller parameters (if dynamic change is needed)
    void setReference(const std::vector<double>& ref) {
        for (size_t i = 0; i < ref.size(); ++i) {
            reference[i] = ref[i]; // Set reference values
		}
	}

    void setReference(const double ref, int i) {
        reference[i] = ref; // Set reference values
    }

    std::vector<double> getReference() const {
        return reference; // Get reference values
	}

	// Getters for controller parameters
    std::string getControllerSymbol() const {
        return controller_symbol; // Get controller symbol
    }

    int getNumberOfSignals() const {
        return static_cast<int>(reference.size()); // Get number of signals
	}

    virtual vector<double> getParameters() const { return reference; }; // Pure virtual function for parameters
    virtual void setParameters(const vector<double>& params) {}; // Pure virtual function to set parameters

	virtual ~Controller() = default; // Virtual destructor for proper cleanup

protected: 
	std::string controller_symbol; // Symbol for the controller
	bool active = false;           // Activation status
    std::vector<double> reference; // Reference values
};

#endif // CONTROLLER_H

