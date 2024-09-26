#ifndef _IMPEDANCE_H_
#define _IMPEDANCE_H_

#include "Element.h"
#include <symengine/symengine_rcp.h>
#include <symengine/basic.h>
#include <vector> // For using vectors

using namespace SymEngine;

class Impedance : public Element {
public:
    // Constructor for single-phase using a symbolic impedance value
    Impedance(const std::string& symbol, int inputPins, int outputPins, const RCP<const Basic>& impedanceValue);

    // Constructor for three-phase using a 3x3 matrix for multi-phase systems
    Impedance(const std::string& symbol, int inputPins, int outputPins, const DenseMatrix& impedanceMatrix);

    // Constructor for vector-based values (single-phase or three-phase)
    Impedance(const std::string& symbol, int inputPins, int outputPins, const std::vector<std::vector<RCP<Symbol>>>& impedanceValues);

    // Destructor
    ~Impedance();

    // Method to compute Y-parameters
    void compute_y_parameters(double frequency) override;

private:
    DenseMatrix Z_matrix; // Matrix representation of impedance
};

#endif // _IMPEDANCE_H_


