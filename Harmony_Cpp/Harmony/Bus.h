#ifndef BUS_H
#define BUS_H

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
    Bus(const std::string& name);

    // Destructor
    ~Bus();

    // Function to attach an element to the bus
    void attachElement(Element* elem);

    // Print elements connected to this bus
    void printConnectedElements();

private:
    //int numBuses;
   // int numBranches;

};

#endif // BUS_H

