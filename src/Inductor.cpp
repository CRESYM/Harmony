#include "Inductor.h"
#include <symengine/symbol.h>
#include <symengine/mul.h>
#include <symengine/integer.h>
#include <iostream>

using namespace SymEngine;

// Frequency-domain constructor (symbolic Y = 1 / (sL))
Inductor::Inductor(const std::string& symbol, int inputPins, int outputPins, double inductance, double frequency)
    : Element(symbol, inputPins, outputPins), L(inductance), initial_value(0.0)
{
    DenseMatrix Y = createZeroMatrix(2 * inputPins, 2 * outputPins);
    RCP<const Basic> sL = mul(s, real_double(L));
    RCP<const Basic> Yval = div(one, sL);  // Y = 1 / (sL)

    if (inputPins == 1 && outputPins == 1) {
        Y.set(0, 0, Yval);
        Y.set(0, 1, mul(integer(-1), Yval));
        Y.set(1, 0, mul(integer(-1), Yval));
        Y.set(1, 1, Yval);
    }
    else {
        for (int i = 0; i < inputPins; ++i) {
            int a = 2 * i;
            int b = 2 * i + 1;
            Y.set(a, a, Yval);
            Y.set(a, b, mul(integer(-1), Yval));
            Y.set(b, a, mul(integer(-1), Yval));
            Y.set(b, b, Yval);
        }
    }

    Y_matrix = Y;  
}

// Time-domain constructor for MNA
Inductor::Inductor(const std::string& symbol,
    const std::vector<Bus*>& node1s,
    const std::vector<Bus*>& node2s,
    double inductance,
    const std::vector<double>& initialCurrents)
    : Element(symbol, static_cast<int>(node1s.size()), static_cast<int>(node2s.size())), L(inductance)
{
    int nph = static_cast<int>(node1s.size());
    SYMENGINE_ASSERT(node2s.size() == nph);
    SYMENGINE_ASSERT(initialCurrents.size() == 1 || initialCurrents.size() == nph);

    if (initialCurrents.size() == 1 && nph > 1) {
        initial_value = initialCurrents[0];
    }
    else if (initialCurrents.size() == nph) {
        initial_value = initialCurrents[0]; 
    }

    for (int i = 0; i < nph; ++i) {
        attachBus(node1s[i], i);
        attachBus(node2s[i], i + nph);
    }
}

// MNA matrix writer
void Inductor::writeMNAmatrix(DenseMatrix& A,
    int num_equations,
    int firstBranchIndex,
    const RCP<const Basic>& value,
    const std::unordered_map<Bus*, int>& busIndex)
{
    int nph = getInputPins();   
    RCP<const Basic> Lsym = value; 

    for (int p = 0; p < nph; ++p) {
        int row = num_equations + firstBranchIndex + p;

        Bus* n1 = nullptr;
        Bus* n2 = nullptr;
        for (auto& kv : connections) {
            if (kv.second == p)         n1 = kv.first;
            if (kv.second == p + nph)   n2 = kv.first;
        }

        if (!n1 || !n2) throw std::runtime_error("Missing bus connections in Inductor");

        int idx1 = busIndex.at(n1);
        int idx2 = busIndex.at(n2);

        A.set(row, idx1, one);
        A.set(row, idx2, minus_one);
        A.set(idx1, row, one);
        A.set(idx2, row, minus_one);
        A.set(row, row, mul(minus_one, Lsym));
    }

}

// Access initial value
double Inductor::getInitialCurrent() const {
    return initial_value;
}

// Print values
void Inductor::printElementValues() {
    std::cout << "Inductor symbol: " << getElementSymbol() << std::endl;
    std::cout << "Inductance: " << L << " H" << std::endl;
    std::cout << "Initial current: " << initial_value << " A" << std::endl;
    std::cout << "Y matrix (symbolic):\n" << Y_matrix.__str__() << std::endl;
}
