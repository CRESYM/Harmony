#ifndef FILTER_H
#define FILTER_H

#include "Control_block.h"

class Filter : public ControlBlock {
public:
    // Constructor
    Filter(const std::string& symbol, const std::string& type, vector<double> values, int number_of_connections)
		: ControlBlock(), filter_symbol(symbol), filter_size(number_of_connections), filter_type(type) {
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


private:
	string filter_symbol; // Symbol for the filter
	string filter_type;   // Type of filter (e.g., LP, HP, BP)
	int filter_size;          // Number of pins for the filter
    double timeConstant; // Time constant
    double zeta;         // Damping ratio
    double bandwidth;    // Bandwidth
};

#endif // FILTER_H
