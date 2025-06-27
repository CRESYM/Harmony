#include "Capacitor.h"
#include "Bus.h"
#include <symengine/expression.h>

using namespace SymEngine;

Capacitor::Capacitor(const std::string& symbol, int inputPins, int outputPins, double capacitance, double frequency)
    : Element(symbol, inputPins, outputPins), C(capacitance)
{
    // Symbolic admittance: Y = s * C used in frequency-domain models.
    DenseMatrix Y = createZeroMatrix(2 * inputPins, 2 * outputPins);
    RCP<const Basic> sC = mul(real_double(C), s);  // s * C

    // Fill 2x2 admittance matrix for a simple capacitor between two nodes
    // |  sC  -sC |
    // | -sC   sC |
    if (inputPins == 1 && outputPins == 1) {
        Y.set(0, 0, sC);
        Y.set(0, 1, mul(integer(-1), sC));
        Y.set(1, 0, mul(integer(-1), sC));
        Y.set(1, 1, sC);
    }
    else {
        // Generalization to multiphase
        std::cerr << "Multiphase capacitor model.\n";
    }

    Y_matrix = Y; //store in element class
}

// Return stored initial voltage
//double Capacitor::get_initial_value() const {
//    return initial_value;
//}

Capacitor::~Capacitor()
{
}

// Print basic capacitor info
void Capacitor::printElementValues() {
    std::cout << "Capacitor symbol: " << getElementSymbol() << std::endl;
    std::cout << "Capacitance: " << C << " F" << std::endl;
    std::cout << "Initial voltage: " << initial_value << " V" << std::endl;
}

