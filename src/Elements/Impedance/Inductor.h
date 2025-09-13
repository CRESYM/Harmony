#ifndef INDUCTOR_H
#define INDUCTOR_H

#include "../Element.h"

using SymEngine::RCP;
using SymEngine::Basic;
using SymEngine::DenseMatrix;

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
