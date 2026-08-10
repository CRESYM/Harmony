#ifndef INDUCTOR_H
#define INDUCTOR_H

/**
 * @file Inductor.h
 * @brief Inductor element with frequency-domain admittance Y = 1/(sL).
 */

#include "../Element.h"

/**
 * @class Inductor
 * @brief Represents an inductor element for circuit/network equations.
 * @ingroup impedance
 *
 * Supports frequency-domain representation where admittance is Y(s) = 1 / (s * L)
 * for each modeled branch and stamps into the symbolic MNA matrix.
 */
class Inductor : public Element {
public:
    /**
     * @brief Construct an inductor with per-phase or uniform inductance values.
     * @param symbol Element identifier.
     * @param location Network area or location string.
     * @param pins Number of pins (phases).
     * @param inductance Inductance values in henries (one value or one per pin).
     */
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
