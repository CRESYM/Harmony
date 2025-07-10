#include "Resistor.h"
#include <symengine/symbol.h>
#include <symengine/basic.h>
#include <symengine/add.h>
#include <symengine/mul.h>
#include <symengine/integer.h>

using namespace SymEngine;

// Helper symbolic operations
RCP<const Basic> inv(const RCP<const Basic>& val) {
    return div(one, val);
}

RCP<const Basic> addSym(const RCP<const Basic>& a, const RCP<const Basic>& b) {
    if (a.is_null()) return b;
    if (b.is_null()) return a;
    return add(a, b);
}

RCP<const Basic> subSym(const RCP<const Basic>& a, const RCP<const Basic>& b) {
    if (a.is_null()) return mul(integer(-1), b);
    return sub(a, b);
}

Resistor::Resistor(const std::string& symbol, int pins, const std::vector<double>& R)
    : Element(symbol, pins, pins)
{
    if (pins <= 0) {
        throw std::invalid_argument("Invalid number of pins. Must be greater than 0.");
    }

    if (R.empty()) {
        throw std::invalid_argument("Resistance vector is empty.");
    }
    
    if (R.size() != 1 && R.size() != static_cast<size_t>(pins)) {
        throw std::invalid_argument("Mismatch between number of pins and resistance values.");
    }

    if (R.size() == 1) {
        if (R[0] <= 0.0)
            throw std::invalid_argument("Resistance must be positive.");
        R_values = std::vector<double>(pins, R[0]);
    }
    else {
        // Validate all resistances
        for (double r : R) {
            if (r <= 0.0)
                throw std::invalid_argument("All resistance values must be positive.");
        }
        R_values = R;
    }

    initializeYMatrix(pins);
}

// Constructor for scalar input
Resistor::Resistor(const std::string& symbol, int pins, double R)
    : Resistor(symbol, pins, std::vector<double>{R})  
{
}

// Initialize admittance matrix based on R_values
void Resistor::initializeYMatrix(int pins)
{
    Y_matrix.assign(2 * pins, std::vector<double>(2 * pins, 0.0));

    // Diagonal entries for positive nodes
    for (int i = 0; i < pins; ++i) {
        Y_matrix[i][i] = 1.0 / R_values[i];
    }

    // Populate rest of Y matrix for series connection
    for (int i = 0; i < pins; ++i) {
        for (int j = 0; j < pins; ++j) {
            double y = Y_matrix[i][j];
            Y_matrix[pins + i][j] = -y;
            Y_matrix[i][pins + j] = -y;
            Y_matrix[pins + i][pins + j] = y;
        }
    }
}

void Resistor::writeMNAmatrix(DenseMatrix& A,
    int num_equations,
    int index,
    const RCP<const Basic>& value,
    const std::unordered_map<Bus*, int>& busIndex)
{
    std::vector<Bus*> buses = getBuses();
    Bus* node1 = buses.size() > 0 ? buses[0] : nullptr;
    Bus* node2 = buses.size() > 1 ? buses[1] : nullptr;

    if (node1 && busIndex.count(node1)) {
        int n1 = busIndex.at(node1);
        A.set(n1, n1, addSym(A.get(n1, n1), inv(value)));

        if (node2 && busIndex.count(node2)) {
            int n2 = busIndex.at(node2);
            A.set(n1, n2, subSym(A.get(n1, n2), inv(value)));
            A.set(n2, n2, addSym(A.get(n2, n2), inv(value)));
            A.set(n2, n1, subSym(A.get(n2, n1), inv(value)));
        }
    }
    else if (node2 && busIndex.count(node2)) {
        int n2 = busIndex.at(node2);
        A.set(n2, n2, addSym(A.get(n2, n2), inv(value)));
    }
}

void Resistor::writeMNAmatrixNumeric(Eigen::MatrixXd& A,
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

    double G = 1.0 / R_values[0];  // Conductance

    if (n1) {
        int r1 = busIndex.at(n1);
        A(r1, r1) += G;
    }
    if (n2) {
        int r2 = busIndex.at(n2);
        A(r2, r2) += G;
    }
    if (n1 && n2) {
        int r1 = busIndex.at(n1);
        int r2 = busIndex.at(n2);
        A(r1, r2) -= G;
        A(r2, r1) -= G;
    }
}


//void Resistor::writeMatrixSymbolic(DenseMatrix& A,
//    const std::unordered_map<Bus*, int>& busIndex)
//{
//    auto buses = getBuses();
//    Bus* node1 = buses.size() > 0 ? buses[0] : nullptr;
//    Bus* node2 = buses.size() > 1 ? buses[1] : nullptr;
//
//    std::cout << "[Resistor::writeMatrixSymbolic] Called for resistor '" << getElementSymbol() << "'\n";
//    std::cout << "  Connected buses: ";
//    if (node1) std::cout << node1->getBusName() << " ";
//    if (node2) std::cout << node2->getBusName();
//    std::cout << "\n";
//
//    if (R_values.empty() || R_values[0] <= 0) {
//        std::cerr << "[Resistor::writeMatrixSymbolic] ERROR: invalid resistance value\n";
//        return;
//    }
//
//    RCP<const Basic> Y_value = div(one, real_double(R_values[0]));
//    std::cout << "  Calculated admittance Y_value = " << *Y_value << "\n";
//
//    if (node1) {
//        if (busIndex.count(node1)) {
//            int n1 = busIndex.at(node1);
//            std::cout << "  Setting A(" << n1 << "," << n1 << ") += Y_value\n";
//            A.set(n1, n1, add(A.get(n1, n1), Y_value));
//
//            if (node2) {
//                if (busIndex.count(node2)) {
//                    int n2 = busIndex.at(node2);
//                    std::cout << "  Setting off-diagonal entries:\n";
//                    std::cout << "    A(" << n1 << "," << n2 << ") -= Y_value\n";
//                    std::cout << "    A(" << n2 << "," << n2 << ") += Y_value\n";
//                    std::cout << "    A(" << n2 << "," << n1 << ") -= Y_value\n";
//
//                    A.set(n1, n2, sub(A.get(n1, n2), Y_value));
//                    A.set(n2, n2, add(A.get(n2, n2), Y_value));
//                    A.set(n2, n1, sub(A.get(n2, n1), Y_value));
//                    std::cout << "  Done setting off-diagonal\n";
//                }
//                else {
//                    std::cerr << "Warning: node2 not found in busIndex\n";
//                }
//            }
//        }
//        else {
//            std::cerr << "Warning: node1 not found in busIndex\n";
//        }
//    }
//    else if (node2) {
//        if (busIndex.count(node2)) {
//            int n2 = busIndex.at(node2);
//            std::cout << "  Setting A(" << n2 << "," << n2 << ") += Y_value\n";
//            A.set(n2, n2, add(A.get(n2, n2), Y_value));
//        }
//        else {
//            std::cerr << " Warning: node2 not found in busIndex\n";
//        }
//    }
//
//    std::cout << "Finished for resistor '" << getElementSymbol() << "'\n";
//}

void Resistor::printElementValues() {
    std::cout << "Resistor values (Ohms): ";
    for (double r : R_values) {
        std::cout << r << " ";
    }
    std::cout << "\nAdmittance matrix (Y):\n";
    for (const auto& row : Y_matrix) {
        for (double val : row) {
            std::cout << val << "\t";
        }
        std::cout << "\n";
    }
}

Resistor::~Resistor() {}
