#ifndef BUS_H
#define BUS_H

//#include "network.h"  // Include the parent class
#include <vector>
#include <string>
#include <symengine/matrix.h>

using namespace SymEngine;

class Element;  // Forward declaration

class Bus {
public:
    std::string busName;
    std::vector<Element*> connectedElements;  // Elements connected to this bus

    // Constructor
    //Bus(int numBuses, int numBranches, const std::vector<std::pair<int, int>>& branches);
    Bus(const std::string& name);


    // Function to attach an element to the bus
    void attachElement(Element* elem);

    // Print elements connected to this bus
    void printConnectedElements();
    
    // Generates the incidence matrix
   // DenseMatrix generateIncidenceMatrix();

    // Function to print the matrix
    //void printIncidenceMatrix();

private:
    //int numBuses;
   // int numBranches;
   // std::vector<std::pair<int, int>> branches;  // List of branches (fromBus, toBus)
   // DenseMatrix A;  // Incidence matrix
};

#endif // BUS_H

