#include "Capacitor.h"
#include "Bus.h"
#include <symengine/expression.h>

using namespace SymEngine;

// Frequency-domain constructor Supports both single-phase and multi-phase
Capacitor::Capacitor(const std::string& symbol, int inputPins, int outputPins, double capacitance, double frequency)
    : Element(symbol, inputPins, outputPins), C(capacitance), initial_value(0.0)
{
    DenseMatrix Y = createZeroMatrix(2 * inputPins, 2 * outputPins);
    RCP<const Basic> sC = mul(real_double(C), s);  // s * C

    // Single-phase case: 2x2 admittance matrix
    if (inputPins == 1 && outputPins == 1) {
        Y.set(0, 0, sC);
        Y.set(0, 1, mul(integer(-1), sC));
        Y.set(1, 0, mul(integer(-1), sC));
        Y.set(1, 1, sC);
    }
    else {
        // Multi-phase case: block diagonal admittance matrix
        for (int i = 0; i < inputPins; ++i) {
            int a = 2 * i;
            int b = 2 * i + 1;
            Y.set(a, a, sC);                       // Y(i,i)
            Y.set(a, b, mul(integer(-1), sC));     // Y(i,j)
            Y.set(b, a, mul(integer(-1), sC));     // Y(j,i)
            Y.set(b, b, sC);                       // Y(j,j)
        }
    }

    Y_matrix = Y; // Store symbolic admittance matrix in Element base class
}

// Time-domain unified constructor for MNA Supports both single-phase and multi-phase 
Capacitor::Capacitor(const std::string& symbol,
    const std::vector<Bus*>& node1s,
    const std::vector<Bus*>& node2s,
    double capacitance,
    const std::vector<double>& initialVoltage)
    : Element(symbol, static_cast<int>(node1s.size()), static_cast<int>(node2s.size())), C(capacitance)
{
    int nph = static_cast<int>(node1s.size());

    SYMENGINE_ASSERT(node2s.size() == nph);
    SYMENGINE_ASSERT(initialVoltage.size() == 1 || initialVoltage.size() == nph);

    // Support scalar initial voltage (broadcast to all phases)
    if (initialVoltage.size() == 1 && nph > 1) {
        initial_value = initialVoltage[0]; 
    }
    else if (initialVoltage.size() == nph) {
        initial_value = initialVoltage[0];  
    }

    // Attach terminals for each phase
    for (int i = 0; i < nph; ++i) {
        attachBus(node1s[i], i);         // Phase i input
        attachBus(node2s[i], i + nph);   // Phase i output
    }
}

Capacitor::~Capacitor()
{
}

void Capacitor::writeMNAmatrix(DenseMatrix& A,
    int num_equations,
    int firstBranchIndex,
    const RCP<const Basic>& value,
    const std::unordered_map<Bus*, int>& busIndex)
{
    int nph = getInputPins();          // #phases (inputPins == outputPins)
    RCP<const Basic> Csym = value;     

    for (int p = 0; p < nph; ++p) {
        int row = num_equations + firstBranchIndex + p;   // branch current row

        
        Bus* n1 = nullptr;
        Bus* n2 = nullptr;
        for (auto& kv : connections) {
            if (kv.second == p)         n1 = kv.first;       // +terminal
            if (kv.second == p + nph)   n2 = kv.first;       // ‑terminal
        }

        
        RCP<const Basic> ic_sym = symbol("ic_" + getElementSymbol() + std::to_string(p));

        A.set(row, num_equations, ic_sym);  
        // stamp ±C between branch row and nodes
        if (n1) { int r = busIndex.at(n1);  A.set(row, r, Csym);  A.set(r, row, one); }
        if (n2) { int r = busIndex.at(n2);  A.set(row, r, mul(integer(-1), Csym));  A.set(r, row, mul(integer(-1), one)); }
    }
}

//void Capacitor::writeMatrixSymbolic(
//    SymEngine::DenseMatrix& Y,
//    const std::unordered_map<Bus*, int>& busIndex)
//{
//    std::cout << "[Capacitor::writeMatrixSymbolic] Called for '" << getElementSymbol() << "'\n";
//
//    // Laplace‑domain admittance Y = s*C
//    auto s = SymEngine::symbol("s");
//    auto Ys = SymEngine::mul(s, SymEngine::real_double(C));
//
//    if (connections.size() != 2) {
//        std::cerr << "[Capacitor] ERROR: needs exactly two terminals\n";
//        return;
//    }
//
//    // grab the two buses out of the base‐class connections map
//    auto it = connections.begin();
//    Bus* p = it->first;   int pi = it->second;
//    ++it;
//    Bus* n = it->first;   int ni = it->second;
//
//    auto ip = busIndex.find(p);
//    auto in = busIndex.find(n);
//    if (ip == busIndex.end() || in == busIndex.end()) {
//        std::cerr << "[Capacitor] Bus not in map\n";
//        return;
//    }
//    int rp = ip->second, rn = in->second;
//
//    std::cout << "  stamping +sC at (" << rp << "," << rp << ") and (" << rn << "," << rn << ");"
//        << "  −sC at (" << rp << "," << rn << ") and (" << rn << "," << rp << ")\n";
//
//    // stamp onto the MNA (same pattern as resistor/AC_source)
//    Y.set(rp, rp, SymEngine::add(Y.get(rp, rp), Ys));
//    Y.set(rn, rn, SymEngine::add(Y.get(rn, rn), Ys));
//    Y.set(rp, rn, SymEngine::sub(Y.get(rp, rn), Ys));
//    Y.set(rn, rp, SymEngine::sub(Y.get(rn, rp), Ys));
//
//    std::cout << "[Capacitor::writeMatrixSymbolic] Done\n";
//}

void Capacitor::writeMNAmatrixNumeric(Eigen::MatrixXd& A,
    int num_equations,
    int index,
    const std::unordered_map<Bus*, int>& busIndex)
{
    Bus* n1 = nullptr;
    Bus* n2 = nullptr;

    for (const auto& [bus, terminal] : connections) {
        if (terminal == 0) n1 = bus;
        else if (terminal == 1) n2 = bus;
    }

    int row = index; 

    int n1_idx = (n1 && busIndex.count(n1)) ? busIndex.at(n1) : -1;
    int n2_idx = (n2 && busIndex.count(n2)) ? busIndex.at(n2) : -1;

    // Stamp KCL equations at the capacitor
    if (n1_idx != -1) {
        A(n1_idx, row) += 1.0;  // current flowing out of node n1
        A(row, n1_idx) += 1.0;  // voltage at node n1
    }
    if (n2_idx != -1) {
        A(n2_idx, row) += -1.0; // current into node n2
        A(row, n2_idx) += -1.0; // voltage at node n2
    }

    // Stamp the capacitor dynamic equation: I = C * dV/dt (or I - C * dV/dt = 0)
    A(row, row) -= C; 
}


// Print basic capacitor info
void Capacitor::printElementValues() {
    std::cout << "Capacitor symbol: " << getElementSymbol() << std::endl;
    std::cout << "Capacitance: " << C << " F" << std::endl;
    std::cout << "Initial voltage: " << initial_value << " V" << std::endl;
    std::cout << "Y matrix (symbolic):\n" << Y_matrix.__str__() << std::endl;
}

