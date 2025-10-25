#ifndef PROPORTIONALINTEGRAL_CONTROLLER_H
#define PROPORTIONALINTEGRAL_CONTROLLER_H

#include "Controller.h"

class ProportionalIntegralController : public Controller {
public:
	// Constructor takes a symbol, type, values (Kp, Ki), and number of signals
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

	void updateParameters(double newKp, double newKi, double newZeta, double newBandwidth);


	virtual Eigen::VectorXd define_equations(const Eigen::VectorXd& x, const Eigen::VectorXd& u, const Eigen::VectorXd& c) override;
	virtual Eigen::VectorXd define_equations(const double x, const double u, const double c) override;

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
	
	double getKp() const {
		return Kp; // Get proportional gain
	}
	double getKi() const {
		return Ki; // Get integral gain
	}
	double getZeta() const {
		return zeta; // Get damping ratio
	}
	double getBandwidth() const {
		return bandwidth; // Get bandwidth
	}

	virtual vector<double> getParameters() const override {
		return { Kp, Ki, zeta, bandwidth }; // Return controller parameters
	}

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