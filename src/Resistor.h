#ifndef _RESISTOR_H_
#define _RESISTOR_H_

#include "Element.h"

/**
 * A pure resistor element with real-valued resistance.
 * Computes admittance matrix Y = 1/R for series connection.
 */


class Resistor : public Element {
public:
    Resistor(const std::string& symbol, int pins, double R);

    Resistor(const std::string& symbol, int pins, const std::vector<double>& R_values);

    ~Resistor();

    const std::vector<std::vector<double>>& getYMatrix() const { return Y_matrix; }

    // Override to print resistor-specific data
    void printElementValues() override;

private:
    std::vector<double> R_values;
    std::vector<std::vector<double>> Y_matrix;

};

#endif // _RESISTOR_H_
