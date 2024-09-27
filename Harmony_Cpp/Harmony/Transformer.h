#ifndef TRANSFORMER_H
#define TRANSFORMER_H

#include "Element.h"
#include <vector>
#include <string>
#include <iostream>

using namespace SymEngine;

class Transformer : public Element {
public:
    Transformer(const std::string& symbol, int pins, std::vector<double> values);  // Constructor with vector input for parameters

    ~Transformer() {}

    void compute_y_parameters(double frequency) override;  // Declaration for Y parameter computation
    void printElementValues() override;  // Declaration for printing values

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

    DenseMatrix Y_matrix;  // Y-parameter matrix for a transformer (4 elements: Y11, Y12, Y21, Y22)
};

#endif // TRANSFORMER_H
