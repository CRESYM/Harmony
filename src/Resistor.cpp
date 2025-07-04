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
