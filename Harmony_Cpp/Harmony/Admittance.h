#ifndef _ADMITTANCE_H_
#define _ADMITTANCE_H_

#include "Element.h"
#include <vector> // Include vector header
#include <SymEngine/Matrix.h> // Ensure you include the necessary SymEngine headers

using namespace SymEngine; // Use the SymEngine namespace

class Admittance : public Element {
public:
    Admittance(const std::string& symbol, int pins, const RCP<const Basic>& admittanceValue);
    Admittance(const std::string& symbol, int pins, const DenseMatrix& admittanceMatrix);
    Admittance(const std::string& symbol, int pins, const std::vector<RCP<Symbol>>& admittanceValues);
    ~Admittance();

    void compute_y_parameters(double frequency) override;

private:
    DenseMatrix Y_matrix; // Matrix representation of admittance
};

#endif // _ADMITTANCE_H_

