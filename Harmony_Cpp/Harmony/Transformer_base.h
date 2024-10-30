#ifndef TRANSFORMER_BASE_H
#define TRANSFORMER_BASE_H

#include "Element.h"


class Transformer_base : public Element {
public:
    // Constructor to initialize the Transformer with a given symbol, number of pins, and values
    Transformer_base(const std::string& symbol, int pins, const std::vector<double>& values);

    ~Transformer_base();

    double getResistance(int winding) const {
        if (winding >= 0 && winding < R.size()) {
            return R[winding];
        }
        throw std::out_of_range("Invalid winding index");
    }

    double getInductance(int winding) const {
        if (winding >= 0 && winding < L.size()) {
            return L[winding];
        }
        throw std::out_of_range("Invalid winding index");
    }

protected:
    std::vector<double> R;  // Resistances for primary and secondary windings, and for magnetization resistance if given
    std::vector<double> L;  // Inductances for primary and secondary windings, and for the magnetization inductance if given
};

#endif
