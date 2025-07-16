#ifndef FILTER_H
#define FILTER_H

#include "Control_block.h"

class Filter : public ControlBlock {
public:
    // Constructor
    Filter(const std::string& symbol, const std::string& type, int order, vector<double> values, int number_of_connections)
		: ControlBlock(), filter_symbol(symbol), filter_size(number_of_connections), filter_type(type), filter_order(order) {
        if (values.size() != 3) {
            throw std::invalid_argument("Filter values must contain time constant, damping ratio, and bandwidth.");
        }
        timeConstant = values[0]; // Time constant (T)
        zeta = values[1];         // Damping ratio (ζ)
		bandwidth = values[2];    // Bandwidth
    }

    // Override method to print element-specific values
    virtual void printValues() override {
        std::cout << "Filter Parameters:"
            << "  Time Constant (T): " << timeConstant << "\n"
            << "  Damping Ratio (ζ): " << zeta << "\n"
            << "  Bandwidth: " << bandwidth << " Hz\n";
    }

    // Method to update filter parameters (if dynamic change is needed)
    void updateParameters(double newTimeConstant, double newZeta, double newBandwidth);

	// Method to get differential equations
    VectorXd define_differential_equations(const VectorXd& x, const VectorXd& u);
    VectorXd define_differential_equations(const VectorXd& x, const double u);
    VectorXd define_differential_equations(const double x, const double u);



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
