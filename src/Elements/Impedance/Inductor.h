#ifndef INDUCTOR_H
#define INDUCTOR_H

#include "../Element.h"

/**
 * @class Inductor
 * @brief Represents an inductor element for circuit/network equations.
 *
 * This class models an inductor component used in the network formulation.
 * It supports frequency-domain representation (symbolic or numeric) where the
 * admittance is Y(s) = 1 / (s * L) for each modeled branch. The class
 * provides an interface to write the element contributions into the MNA
 * (Modified Nodal Analysis) symbolic matrix and to print element values.
 *
 * The constructor expects a vector of inductance values (one per terminal or
 * configuration) and the element participates in assembling the global MNA
 * matrix via `writeMNAmatrix`.
 */
class Inductor : public Element {
public:
    // Frequency-domain constructor (symbolic admittance Y = 1 / (sL))
    Inductor(const std::string& symbol, const std::string& location, int pins, const std::vector<double>& inductance);

    // MNA matrix writer
    void writeMNAmatrix(SymEngine::DenseMatrix& matrix, std::unordered_map<Bus*, int>& bus_indices, int location, std::map<Element*, std::vector<RCP<const Basic>>>&) override;

    void printElementValues() override;

    double getInitialCurrent() const;

private:
    std::vector<double> L;   // inductance (H)
    double initial_value = 0; // Initial current value for the inductor, not used for now
};

#endif // INDUCTOR_H
