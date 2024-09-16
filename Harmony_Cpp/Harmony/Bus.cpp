#include "bus.h"
#include <iostream>
#include <sstream>  // Include for string streams
#include <symengine/printers.h>  // Correct header for printing

// Constructor to initialize the Bus class
Bus::Bus(int numBuses, int numBranches, const std::vector<std::pair<int, int>>& branches)
    : numBuses(numBuses), numBranches(numBranches), branches(branches), A(numBuses, numBranches) {
    A = generateIncidenceMatrix();  // Generate the incidence matrix during initialization
}

// Function to generate the incidence matrix
DenseMatrix Bus::generateIncidenceMatrix() {
    DenseMatrix A(numBuses, numBranches);

    // Populate the matrix based on the branch connections
    for (size_t branch = 0; branch < branches.size(); ++branch) {
        int fromBus = branches[branch].first - 1;  // Convert to zero-indexed
        int toBus = branches[branch].second - 1;   // Convert to zero-indexed

        // Set the matrix values: 1 for outgoing, -1 for incoming
        A.set(fromBus, branch, integer(1));  // Outgoing from 'fromBus'
        A.set(toBus, branch, integer(-1));   // Incoming to 'toBus'
    }

    return A;
}

// Function to convert SymEngine Basic object to string
std::string symengine_to_string(const RCP<const Basic>& expr) {
    if (expr.get() == nullptr) {  // Null check using get()
        return "0";  // Return a placeholder for null pointers
    }
    std::ostringstream oss;
    oss << *expr;  // Use SymEngine's operator<< to stream into ostringstream
    return oss.str();  // Convert stream to string
}

// Function to print the incidence matrix
void Bus::printIncidenceMatrix() {
    std::cout << "Incidence Matrix (" << numBuses << "x" << numBranches << "):" << std::endl;
    for (size_t i = 0; i < A.nrows(); ++i) {
        for (size_t j = 0; j < A.ncols(); ++j) {
            // Convert each matrix element to a string and print it
            std::cout << symengine_to_string(A.get(i, j)) << " ";
        }
        std::cout << std::endl;
    }
}
