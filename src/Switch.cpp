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

//void Switch::writeMNAmatrix(DenseMatrix& A,
//    int num_equations,
//    int index,
//    const RCP<const Basic>& value,
//    const std::unordered_map<Bus*, int>& busIndex)
//{
//    int nph = static_cast<int>(phaseState.size());
//
//    for (int p = 0; p < nph; ++p) {
//        int row = num_equations + index + p - 1;   // one aux row per phase
//
//        Bus* n1 = nullptr;
//        Bus* n2 = nullptr;
//        for (auto& kv : connections) {
//            if (kv.second == p)           
//                n1 = kv.first;           // input
//            if (kv.second == p + nph)     
//                n2 = kv.first;           // output
//        }
//
//        if (phaseState[p]) {  // closed
//            if (n1) 
//            { 
//                int r = busIndex.at(n1);  
//                A.set(row, r, one);  
//                A.set(r, row, one); 
//            }
//            if (n2) 
//            { 
//                int r = busIndex.at(n2);  
//                A.set(row, r, mul(integer(-1), one));  
//                A.set(r, row, mul(integer(-1), one));
//            }
//        }
//        else {              // open
//            A.set(row, row, one);                     // impose Vbranch = 0
//            if (n1) 
//            { 
//                int r = busIndex.at(n1);  
//                A.set(r, row, one); 
//            }
//            if (n2) 
//            { 
//                int r = busIndex.at(n2);  
//                A.set(r, row, mul(integer(-1), one));
//            }
//        }
//    }
//}

void Switch::writeMNAmatrix(DenseMatrix& A,
    int num_equations,
    int index,
    const RCP<const Basic>& value,
    const std::unordered_map<Bus*, int>& busIndex)
{
    int nph = static_cast<int>(phaseState.size());

    for (int p = 0; p < nph; ++p) {
        int row = num_equations + index + p;  // CORRECTED: No -1

        Bus* n1 = nullptr;
        Bus* n2 = nullptr;

        //for (auto& kv : connections) {
        //    if (kv.second == p)
        //        n1 = kv.first;
        //    if (kv.second == p + nph)
        //        n2 = kv.first;
        //}
        for (auto& kv : connections) {
            int idx = kv.second;
            if (idx == p)       
                n1 = kv.first;
            else if (idx == p + nph) 
                n2 = kv.first;
        }

        if (phaseState[p]) {  // CLOSED switch
            if (n1) {
                int r = busIndex.at(n1);
                A.set(row, r, one);
                A.set(r, row, one);
            }
            if (n2) {
                int r = busIndex.at(n2);
                A.set(row, r, mul(integer(-1), one));
                A.set(r, row, mul(integer(-1), one));
            }
        }
        else {  // OPEN switch
            A.set(row, row, one);  // I = 0

            if (n1) {
                int r = busIndex.at(n1);
                A.set(r, row, one);
            }
            if (n2) {
                int r = busIndex.at(n2);
                A.set(r, row, mul(integer(-1), one));
            }
        }
    }
}

void Switch::printElementValues() {
    std::cout << "Switch " << element_symbol << " state: ";
    for (size_t i = 0; i < phaseState.size(); ++i) {
        std::cout << "[" << i << "]=" << (phaseState[i] ? "closed" : "open") << " ";
    }
    std::cout << std::endl;
}
