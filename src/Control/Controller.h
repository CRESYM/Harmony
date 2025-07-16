#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "Control_block.h"

class Controller : ControlBlock {
public:
	// Constructor takes a symbol, type, values (Kp, Ki), and number of signals
    Controller(const std::string& symbol, const std::string& type, const std::vector<double>& value, int number_signals, std::vector<double> refs = { 0 })
        : ControlBlock(), controller_symbol(symbol), controller_type(type), reference(refs) {
        if (value.size() != 2) {
            throw std::invalid_argument("Controller values must contain at least Kp and Ki.");
		}
        Kp = value[0]; // Proportional gain
        Ki = value[1]; // Integral gain
        zeta = 0.0;    // Default damping ratio
        bandwidth = 0.0; // Default bandwidth
        if (reference.size() != number_signals) {
            reference.resize(number_signals, 0.0); // Initialize reference values
        }
    }

    Eigen::VectorXd define_differential_equations(const Eigen::VectorXd& x, const Eigen::VectorXd& u, const Eigen::VectorXd& c);
	Eigen::VectorXd define_differential_equations(const double x, const double u, const double c);


    // Override method to print element-specific values
   virtual void printValues() override {
        std::cout << "Controller Parameters:\n"
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

    // Method to update controller parameters (if dynamic change is needed)
    void updateParameters(double newKp, double newKi, double newZeta, double newBandwidth);

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

private: 
	std::string controller_symbol; // Symbol for the controller
	std::string controller_type;   // Type of controller (e.g., PI, PID)
    double Kp;                     // Proportional gain
    double Ki;                     // Integral gain
    double zeta;                   // Damping ratio
    double bandwidth;              // Bandwidth
	bool active = false;           // Activation status
    std::vector<double> reference; // Reference values
};

#endif // CONTROLLER_H

