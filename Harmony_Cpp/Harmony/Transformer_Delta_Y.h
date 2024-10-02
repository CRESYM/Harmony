#ifndef TRANSFORMER_DELTA_Y_H
#define TRANSFORMER_DELTA_Y_H

#include "Element.h"

class TransformerDeltaY : public Element {
public:
    // Constructor to initialize the Transformer with a given symbol, number of pins, and values (with phase lag)
    TransformerDeltaY(const std::string& symbol, int pins, const std::vector<double>& values);

    ~TransformerDeltaY() override;

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

private:
    std::vector<double> R;  // Resistances for primary (Delta side) and secondary (Star side) windings
    std::vector<double> X;  // Reactances for primary and secondary windings
    double a;  // Turns ratio
    double phaseLag;  // Phase lag to incorporate in the Y parameters
};

#endif // TRANSFORMER_DELTA_Y_H
#pragma once
