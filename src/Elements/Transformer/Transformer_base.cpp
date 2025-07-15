#include "Transformer_base.h"

Transformer_base::Transformer_base(const std::string& symbol, int pins, const std::vector<double>& values)
	: Element(symbol, pins, pins) {
}

// Destructor
Transformer_base::~Transformer_base() {
    std::cout << "Transformer object for " << getElementSymbol() << " destroyed." << std::endl;
}

//// Function to compute the power flow through the transformer
//std::complex<double> Transformer_base::compute_power_flow() {
//    // Compute the impedance of the transformer
//    std::complex<double> impedance = get_impedance();
//
//    // Current through the transformer primary winding (V = IZ -> I = V/Z)
//    std::complex<double> current_primary = voltage_primary / impedance;
//
//    // Power delivered to the transformer
//    std::complex<double> power_primary = voltage_primary * std::conj(current_primary);  // S = V * I*
//
//    // Power delivered to the secondary side (assuming ideal transformer, power is conserved)
//    std::complex<double> current_secondary = voltage_secondary / impedance;
//    std::complex<double> power_secondary = voltage_secondary * std::conj(current_secondary);
//
//    // Return the primary side power (could also return secondary side or both)
//    return power_primary;
//}
//
//// Function to calculate transformer impedance (using resistance and inductance)
//std::complex<double> Transformer_base::get_impedance() const {
//    double resistance = getResistance(0);  // Using the first winding as an example
//    double inductance = getInductance(0);  // Using the first winding as an example
//    double omega = 2 * M_PI * 50;  // Example for 50 Hz frequency, change as needed
//
//    // Impedance is R + jωL
//    std::complex<double> impedance(resistance, omega * inductance);
//    return impedance;
//}