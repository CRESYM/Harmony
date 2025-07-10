#include "Switch.h"
using namespace SymEngine;

Switch::Switch(const std::string& symbol,
    const std::vector<Bus*>& node1s,
    const std::vector<Bus*>& node2s,
    const std::vector<bool>& state)
    : Element(symbol, static_cast<int>(node1s.size()),
        static_cast<int>(node2s.size())),
    phaseState(state)
{
    int nph = static_cast<int>(node1s.size()); //number of phases

    SYMENGINE_ASSERT(node2s.size() == nph);
    SYMENGINE_ASSERT(state.size() == 1 || state.size() == nph);

    if (state.size() == 1 && nph > 1) { // single-phase
        phaseState = std::vector<bool>(nph, state[0]);
    }
    else {
        phaseState = state; //multi-phase
    }

    for (int i = 0; i < nph; ++i) {
        attachBus(node1s[i], i);         // input pins/phases
        attachBus(node2s[i], i + nph);   // output pins/phases
    }
}

void Switch::writeMNAmatrix(DenseMatrix& A,
    int num_equations,
    int index,
    const RCP<const Basic>& value,
    const std::unordered_map<Bus*, int>& busIndex)
{
    int nph = static_cast<int>(phaseState.size());

    for (int p = 0; p < nph; ++p) {
        int row = num_equations + index + p;

        // Find buses for this phase
        Bus* n1 = nullptr;
        Bus* n2 = nullptr;

        for (auto& kv : connections) {
            if (kv.second == p)
                n1 = kv.first;   // input
            else if (kv.second == p + nph)
                n2 = kv.first;   // output
        }

        // Skip stamping if open
        if (!phaseState[p]) continue;

        if (n1) {
            int i = busIndex.at(n1);
            A.set(row, i, one);
            A.set(i, row, one);
        }
        if (n2) {
            int j = busIndex.at(n2);
            A.set(row, j, minus_one);
            A.set(j, row, minus_one);
        }
        // Ideal switch: diagonal = 0
        A.set(row, row, zero);
    }
}


//void Switch::writeMatrixSymbolic(SymEngine::DenseMatrix& Y,
//    const std::unordered_map<Bus*, int>& busIndex) {
//
//    int nph = static_cast<int>(phaseState.size());
//
//    for (int i = 0; i < nph; ++i) {
//        if (!phaseState[i])
//            continue;  // skip open phases
//
//        Bus* n1 = nullptr;
//        Bus* n2 = nullptr;
//
//        for (auto& kv : connections) {
//            if (kv.second == i)
//                n1 = kv.first;
//            else if (kv.second == i + nph)
//                n2 = kv.first;
//        }
//
//        if (!n1 || !n2)
//            throw std::runtime_error("Switch missing bus connections in symbolic matrix.");
//
//        int idx1 = busIndex.at(n1);
//        int idx2 = busIndex.at(n2);
//
//        RCP<const Basic> Y_switch = symbol("Y_switch"); // define this elsewhere or pass as parameter
//
//        // Stamp matrix like a resistor connecting n1 and n2 with admittance Y_switch:
//        // Y(n1,n1) += Y_switch
//        Y.set(idx1, idx1, add(Y.get(idx1, idx1), Y_switch));
//        // Y(n2,n2) += Y_switch
//        Y.set(idx2, idx2, add(Y.get(idx2, idx2), Y_switch));
//        // Y(n1,n2) -= Y_switch
//        Y.set(idx1, idx2, sub(Y.get(idx1, idx2), Y_switch));
//        // Y(n2,n1) -= Y_switch
//        Y.set(idx2, idx1, sub(Y.get(idx2, idx1), Y_switch));
//    }
//}

void Switch::printElementValues() {
    std::cout << "Switch " << element_symbol << " state: ";
    for (size_t i = 0; i < phaseState.size(); ++i) {
        std::cout << "[" << i << "]=" << (phaseState[i] ? "closed" : "open") << " ";
    }
    std::cout << std::endl;
}

