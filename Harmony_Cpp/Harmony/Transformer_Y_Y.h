#ifndef TRANSFORMER_Y_Y_H
#define TRANSFORMER_Y_Y_H

#include "Transformer_classic.h"

class TransformerYY : public Transformer_classic {
public:
    // Constructor to initialize the Transformer with a given symbol, number of pins, values,
    // mutual inductances, and coupling coefficients.
    TransformerYY(const std::string& symbol, int pins, const std::vector<double>& values);

    ~TransformerYY() override;

    double getPhaseLag() const { return phaseLag; }  // Method to get phase lag

    double getCouplingCoefficient(int winding1, int winding2) const {
        if (winding1 >= 0 && winding1 < couplingCoeff.size() && winding2 >= 0 && winding2 < couplingCoeff.size()) {
            return couplingCoeff[winding1 * couplingCoeff.size() + winding2];  // Assuming a 1D representation
        }
        throw std::out_of_range("Invalid winding indices");
    }

private:
    double phaseLag;                        // Phase lag in radians
    std::vector<double> couplingCoeff;     // Coupling coefficients between windings
    double M_Y;                             // Mutual reactance in Y winding
  };


#endif // TRANSFORMER_Y_Y_H
