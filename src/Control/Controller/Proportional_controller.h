#ifndef PROPORTIONAL_CONTROLLER_H
#define PROPORTIONAL_CONTROLLER_H

#include "Controller.h"

class ProportionalController : public Controller {
public:
	// Constructor takes a symbol, type, values (Kp), and number of signals
	ProportionalController(const std::string& symbol, const vector<double>& value, int number_signals, std::vector<double> refs = { 0 })
		: Controller(symbol, value, number_signals, refs) {
		if (value.size() != 1) {
			throw std::invalid_argument("Proportional controller values must contain Kp.");
		}
		K = value[0]; // Proportional gain
	}

	virtual Eigen::VectorXd define_equations(const Eigen::VectorXd& x, const Eigen::VectorXd& u, const Eigen::VectorXd& c) override;
	virtual Eigen::VectorXd define_equations(const double x, const double u, const double c) override;

	virtual void printValues() override {
		std::cout << "  Controller Type: P\n"
			<< "  Proportional Gain (K): " << K << "\n"
			<< "  Reference Values: ";
		for (const auto& ref : reference) {
			std::cout << ref << " ";
		}
		std::cout << std::endl;
	}

	double getK() const {
		return K; // Get proportional gain
	}

	void setK(double newK) {
		K = newK; // Set new proportional gain
	}

	virtual vector<double> getParameters() const override {
		return { K }; // Return controller parameters
	}

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
