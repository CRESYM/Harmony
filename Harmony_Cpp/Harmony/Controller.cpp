#include "Controller.h"


// updateParameters: Update controller parameters (if dynamic change is needed)
void Controller::updateParameters(double newKp, double newKi, double newZeta, double newBandwidth) {
    Kp = newKp;
    Ki = newKi;
    zeta = newZeta;
    bandwidth = newBandwidth;
    std::cout << "Controller parameters updated: \n"
        << "  New Proportional Gain (Kp): " << Kp << "\n"
        << "  New Integral Gain (Ki): " << Ki << "\n"
        << "  New Damping Ratio (ζ): " << zeta << "\n"
        << "  New Bandwidth: " << bandwidth << " Hz\n";
}

