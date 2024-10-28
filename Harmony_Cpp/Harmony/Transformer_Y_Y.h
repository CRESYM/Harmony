#ifndef TRANSFORMER_Y_Y_H
#define TRANSFORMER_Y_Y_H

#include "Element.h"
#include <vector>
#include <stdexcept>
#include <iostream>

class TransformerYY : public Element {
public:
    // Constructor to initialize the Transformer with a given symbol, number of pins, values,
    // mutual inductances, and coupling coefficients.
    TransformerYY(const std::string& symbol, int pins, const std::vector<double>& values,
        const std::vector<double>& couplingCoefficients);

    ~TransformerYY() override;

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
    //double getMutualInductance(int winding1, int winding2) const {
    //    if (winding1 >= 0 && winding1 < mutualInd.size() && winding2 >= 0 && winding2 < mutualInd.size()) {
    //        return mutualInd[winding1 * mutualInd.size() + winding2];  // Assuming a 1D representation
    //    }
    //    throw std::out_of_range("Invalid winding indices");
    //}

    double getCouplingCoefficient(int winding1, int winding2) const {
        if (winding1 >= 0 && winding1 < couplingCoeff.size() && winding2 >= 0 && winding2 < couplingCoeff.size()) {
            return couplingCoeff[winding1 * couplingCoeff.size() + winding2];  // Assuming a 1D representation
        }
        throw std::out_of_range("Invalid winding indices");
    }

private:
    std::vector<double> R;                 // Resistances for primary and secondary windings
    std::vector<double> X;                 // Reactances for primary and secondary windings
    double a;                               // Turns ratio
    double phaseLag;                        // Phase lag in radians
    std::vector<double> couplingCoeff;     // Coupling coefficients between windings
    double M_Y;                             // Mutual reactance in Y winding
  };


#endif // TRANSFORMER_Y_Y_H
