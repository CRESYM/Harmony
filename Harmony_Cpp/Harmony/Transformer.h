#ifndef TRANSFORMER_H
#define TRANSFORMER_H

#include "Element.h"
#include <vector>
#include <string>
#include <iostream>

using namespace SymEngine;

class Transformer : public Element {
public:
    // Constructor to initialize the Transformer with a given symbol, number of pins, and values
    Transformer(const std::string& symbol, int pins, const std::vector<double>& values);

    ~Transformer() override; 

    void compute_y_parameters(double frequency) override;

    // Print the Y-parameter matrix values
    void printElementValues() override {
        std::cout << "Y-parameter Matrix for Transformer: " << getElementSymbol() << std::endl;
        for (int i = 0; i < Y_matrix.nrows(); i++) {
            for (int j = 0; j < Y_matrix.ncols(); j++) {
                // Check the bounds before accessing
                if (i < Y_matrix.nrows() && j < Y_matrix.ncols()) {
                    std::cout << Y_matrix.get(i, j)->__str__() << " ";  // Print each matrix element
                }
                else {
                    std::cerr << "Index out of bounds: (" << i << ", " << j << ")" << std::endl;
                }
            }
            std::cout << std::endl;
        }
    }

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

    //DenseMatrix Y_matrix;  // Y-parameter matrix for a transformer (4 elements: Y11, Y12, Y21, Y22)
};

#endif // TRANSFORMER_H
