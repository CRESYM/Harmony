#ifndef TRANSFORMER_DELTA_Y_H
#define TRANSFORMER_DELTA_Y_H

#include "Element.h"
#include <vector>
#include <stdexcept>
#include <iostream>

class TransformerDeltaY : public Element {
public:
    // Constructor to initialize the Transformer with a given symbol, number of pins, values,
    // mutual inductances, and coupling coefficients.
    TransformerDeltaY(const std::string& symbol, int pins, const std::vector<double>& values,
        const std::vector<double>& mutualInductances, const std::vector<double>& couplingCoefficients);

    ~TransformerDeltaY() override;

    // Getters for resistance and reactance of specific windings
    double getResistance(int winding) const {
        if (winding >= 0 && winding < R.size()) {
            return R[winding];
        }
        throw std::out_of_range("Invalid winding index");
    }

    double getReactance(int winding) const {
        if (winding >= 0 && winding < X.size()) {
            return X[winding];
        }
        throw std::out_of_range("Invalid winding index");
    }

    double getTurnsRatio() const { return a; }
    double getPhaseLag() const { return phaseLag; }  // Method to get phase lag

    // Methods to get mutual inductance and coupling coefficients
    double getMutualInductance(int winding1, int winding2) const {
        if (winding1 >= 0 && winding1 < mutualInd.size() && winding2 >= 0 && winding2 < mutualInd.size()) {
            return mutualInd[winding1 * mutualInd.size() + winding2];  // Assuming a 1D representation
        }
        throw std::out_of_range("Invalid winding indices");
    }

    double getCouplingCoefficient(int winding1, int winding2) const {
        if (winding1 >= 0 && winding1 < couplingCoeff.size() && winding2 >= 0 && winding2 < couplingCoeff.size()) {
            return couplingCoeff[winding1 * couplingCoeff.size() + winding2];  // Assuming a 1D representation
        }
        throw std::out_of_range("Invalid winding indices");
    }

private:
    std::vector<double> R;                 // Resistances for primary (Delta side) and secondary (Star side) windings
    std::vector<double> X;                 // Reactances for primary and secondary windings
    double a;                               // Turns ratio
    double phaseLag;                        // Phase lag in radians
    std::vector<double> mutualInd;         // Mutual inductances between windings
    std::vector<double> couplingCoeff;     // Coupling coefficients between windings
    double M_delta;                         // Mutual reactance in Delta winding
    double M_star;                          // Mutual reactance in Star winding
};

#endif // TRANSFORMER_DELTA_Y_H
