#ifndef TRANSFORMER_Y_DELTA_H
#define TRANSFORMER_Y_DELTA_H

#include "Element.h"
#include <vector>
#include <stdexcept>
#include <string>

class TransformerYDelta : public Element {
public:
    // Constructor to initialize the Transformer with a given symbol, number of pins, values (including phase lag), mutual reactances, and coupling coefficients
    TransformerYDelta(const std::string& symbol, int pins, const std::vector<double>& values,
        const std::vector<double>& mutualInductances, const std::vector<double>& couplingCoefficients);

    ~TransformerYDelta() override;

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

    double getPhaseLag() const { return phaseLag; }  // Method to retrieve phase lag

    double getMutualDelta() const { return M_delta; } // Method to retrieve mutual reactance in Delta winding

    double getMutualStar() const { return M_star; }   // Method to retrieve mutual reactance in Star winding

private:
    std::vector<double> R;  // Resistances for primary (Y side) and secondary (Delta side) windings
    std::vector<double> X;  // Reactances for primary and secondary windings
    double a;               // Turns ratio
    double phaseLag;       // Phase lag to incorporate in Y parameters
    double M_delta;        // Mutual reactance in Delta winding
    double M_star;         // Mutual reactance in Star winding
    std::vector<double> mutualInd; // Mutual inductances
    std::vector<double> couplingCoeff; // Coupling coefficients
};

#endif // TRANSFORMER_Y_DELTA_H


