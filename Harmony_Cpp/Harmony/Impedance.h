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
    //Impedance(const std::string& symbol, int inputPins, int outputPins, const RCP<const Basic>& impedanceValue);

    // Constructor for three-phase using a 3x3 matrix for multi-phase systems
    //Impedance(const std::string& symbol, int inputPins, int outputPins, const DenseMatrix& impedanceMatrix);

    // Constructor for vector-based values (single-phase or three-phase)
    //Impedance(const std::string& symbol, int inputPins, int outputPins, const std::vector<std::vector<RCP<Symbol>>>& impedanceValues);
    // Unified constructor for both single-phase and three-phase systems
    Impedance(const std::string& symbol, int inputPins, int outputPins,
        const DenseMatrix& impedanceMatrix = createZeroMatrix(1))
        : Element(symbol, inputPins, outputPins), Z_matrix(impedanceMatrix) {

    }

    // Destructor
    ~Impedance();

    // Method to compute Y-parameters
    void compute_y_parameters(double frequency) override;

private:
    DenseMatrix Z_matrix; // Matrix representation of impedance
    // Helper function to create a zero matrix
    static DenseMatrix createZeroMatrix(int size) {
        DenseMatrix zeroMatrix(size, size);
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                zeroMatrix.set(i, j, zero); // Use SymEngine's symbolic `zero`
            }
        }
        return zeroMatrix;
    }
};

#endif // _IMPEDANCE_H_


