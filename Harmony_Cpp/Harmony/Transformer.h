#ifndef TRANSFORMER_H
#define TRANSFORMER_H

#include "Element.h"


class Transformer : public Element {
public:
    // Constructor to initialize the Transformer with a given symbol, number of pins, and values
    Transformer(const std::string& symbol, int pins, const std::vector<double>& values);

    ~Transformer() override; 

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

private:
    std::vector<double> R;  // Resistances for primary and secondary windings
    std::vector<double> X;  // Reactances for primary and secondary windings
    double a;  // Turns ratio
};

#endif // TRANSFORMER_H
