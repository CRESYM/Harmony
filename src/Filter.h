#ifndef FILTER_H
#define FILTER_H

#include "Element.h"

class Filter : public Element {
public:
    // Constructor
    Filter(const std::string& symbol, int pins, double timeConstant, double zeta, double bandwidth)
        : Element(symbol, pins, pins), // Filters typically have the same number of input and output pins
        timeConstant(timeConstant), zeta(zeta), bandwidth(bandwidth) {}

    // Override method to print element-specific values
    virtual void printElementValues() override {
        Element::printElementInfo();
        std::cout << "Filter Parameters:"
            << "  Time Constant (T): " << timeConstant << "\n"
            << "  Damping Ratio (ζ): " << zeta << "\n"
            << "  Bandwidth: " << bandwidth << " Hz\n";
    }

    // Method to update filter parameters (if dynamic change is needed)
    void updateParameters(double newTimeConstant, double newZeta, double newBandwidth);


private:
    double timeConstant; // Time constant
    double zeta;         // Damping ratio
    double bandwidth;    // Bandwidth
};

#endif // FILTER_H
