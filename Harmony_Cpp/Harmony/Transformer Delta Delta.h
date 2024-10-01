#ifndef TRANSFORMER_DELTA_DELTA_H
#define TRANSFORMER_DELTA_DELTA_H

#include "Element.h"
#include <vector>
#include <string>
#include <stdexcept>

class TransformerDeltaDelta : public Element {
public:
    // Constructor to initialize the Transformer with a given symbol, number of pins, and values
    TransformerDeltaDelta(const std::string& symbol, int pins, const std::vector<double>& values);

    // Destructor
    ~TransformerDeltaDelta() override;

    // Get the resistance of the specified winding (0 for primary, 1 for secondary)
    double getResistance(int winding) const {
        if (winding >= 0 && winding < R.size()) {
            return R[winding];
        }
        throw std::out_of_range("Invalid winding index");
    }

    // Get the reactance of the specified winding (0 for primary, 1 for secondary)
    double getReactance(int winding) const {
        if (winding >= 0 && winding < X.size()) {
            return X[winding];
        }
        throw std::out_of_range("Invalid winding index");
    }

    // Get the turns ratio (primary to secondary)
    double getTurnsRatio() const { return a; }

private:
    std::vector<double> R;  // Resistances for primary and secondary windings (size 2: R_primary, R_secondary)
    std::vector<double> X;  // Reactances for primary and secondary windings (size 2: X_primary, X_secondary)
    double a;  // Turns ratio between primary and secondary
};

#endif // TRANSFORMER_DELTA_DELTA_H
