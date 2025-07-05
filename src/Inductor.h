#ifndef INDUCTOR_H
#define INDUCTOR_H

#include "Element.h"
#include "Bus.h"
#include <symengine/expression.h>

using SymEngine::RCP;
using SymEngine::Basic;
using SymEngine::DenseMatrix;

class Inductor : public Element {
public:
    // Frequency-domain constructor (symbolic admittance Y = 1 / (sL))
    Inductor(const std::string& symbol, int inputPins, int outputPins, double inductance, double frequency);

    // Time-domain constructor (MNA-based) for both single, multi-phase
    Inductor(const std::string& symbol,
        const std::vector<Bus*>& node1s,
        const std::vector<Bus*>& node2s,
        double inductance,
        const std::vector<double>& initialCurrents = {});

    // MNA matrix writer
    void writeMNAmatrix(SymEngine::DenseMatrix& A,
        int num_equations,
        int firstBranchIndex,
        const RCP<const Basic>& value,
        const std::unordered_map<Bus*, int>& busIndex) override;

    void printElementValues() override;

    double getInitialCurrent() const;

private:
    double L;   // inductance (H)
    double initial_value;
};

#endif // INDUCTOR_H
