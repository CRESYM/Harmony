#ifndef TRANSFORMER_DELTA_DELTA_H
#define TRANSFORMER_DELTA_DELTA_H

#include "Element.h"

class TransformerDeltaDelta : public Element {
public:
    // Constructor to initialize the Transformer with a given symbol, number of pins, values (with phase lag),
    // mutual inductances, coupling coefficients, and mutual reactance for delta winding.
    TransformerDeltaDelta(const std::string& symbol, int pins, const std::vector<double>& values,
        const std::vector<double>& mutualInductances, const std::vector<double>& couplingCoefficients,
        double M_delta);

    ~TransformerDeltaDelta() override;

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

    double getMutualInductance(int index) const {
        if (index >= 0 && index < mutualInd.size()) {
            return mutualInd[index];
        }
        throw std::out_of_range("Invalid mutual inductance index");
    }

    double getCouplingCoefficient(int index) const {
        if (index >= 0 && index < couplingCoeff.size()) {
            return couplingCoeff[index];
        }
        throw std::out_of_range("Invalid coupling coefficient index");
    }

    double getMutualReactanceDelta() const { return M_delta; }  // Method to get mutual reactance for delta winding

private:
    std::vector<double> R;  // Resistances for primary and secondary windings
    std::vector<double> X;  // Reactances for primary and secondary windings
    double a;  // Turns ratio
    double phaseLag;  // Phase lag to incorporate in the Y parameters

    // New members for mutual inductances and coupling coefficients
    std::vector<double> mutualInd;  // Mutual inductances for transformer windings
    std::vector<double> couplingCoeff;  // Coupling coefficients for transformer windings
    double M_delta;  // Mutual reactance for the delta winding
};

#endif // TRANSFORMER_DELTA_DELTA_H
