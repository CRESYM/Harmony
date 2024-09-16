#ifndef BUS_H
#define BUS_H

#include "network.h"  // Include the parent class
#include <vector>
#include <symengine/matrix.h>

using namespace SymEngine;

class Bus : public Network {
public:
    // Constructor
    Bus(int numBuses, int numBranches, const std::vector<std::pair<int, int>>& branches);

    // Generates the incidence matrix
    DenseMatrix generateIncidenceMatrix();

    // Function to print the matrix
    void printIncidenceMatrix();

private:
    int numBuses;
    int numBranches;
    std::vector<std::pair<int, int>> branches;  // List of branches (fromBus, toBus)
    DenseMatrix A;  // Incidence matrix
};

#endif // BUS_H

