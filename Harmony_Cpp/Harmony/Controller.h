#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "Element.h"

class Controller : public Element {
public:
    // Constructor
    Controller(const std::string& symbol, int pins, double Kp, double Ki, double zeta, double bandwidth, const std::vector<double>& ref)
        : Element(symbol, pins, pins), // Controllers typically have the same number of input and output pins
        Kp(Kp), Ki(Ki), zeta(zeta), bandwidth(bandwidth), reference(ref) {}


    // Override method to print element-specific values
    virtual void printElementValues() override {
        Element::printElementInfo();
        std::cout << "Controller Parameters:\n"
            << "  Proportional Gain (Kp): " << Kp << "\n"
            << "  Integral Gain (Ki): " << Ki << "\n"
            << "  Damping Ratio (ζ): " << zeta << "\n"
            << "  Bandwidth: " << bandwidth << " Hz\n"
            << "  Reference Values: ";
        for (const auto& ref : reference) {
            std::cout << ref << " ";
        }
        std::cout << std::endl;
    }

    // Method to update controller parameters (if dynamic change is needed)
    void updateParameters(double newKp, double newKi, double newZeta, double newBandwidth);


private: 
    double Kp;                     // Proportional gain
    double Ki;                     // Integral gain
    double zeta;                   // Damping ratio
    double bandwidth;              // Bandwidth
    std::vector<double> reference; // Reference values
};

#endif // CONTROLLER_H

