#ifndef BUS_H
#define BUS_H

#include <vector>
#include <string>
#include <symengine/matrix.h>

using namespace SymEngine;

class Element;  // Forward declaration

class Bus {
public:

    // Constructor
    Bus(const std::string& name, int number);

    // Destructor
    ~Bus();

    // Getters
    std::string getBusName() { return busName; }
    int getPinNumber() { return numberPins; }

    // Compare
    bool operator==(const char* name);

    // Function to attach an element to the bus
    void attachElement(Element* elem);

    // Print elements connected to this bus
    void printConnectedElements();

private:
    std::string busName;
    int numberPins; // Definition of number of phases/pins/terminals
    std::vector<Element*> connectedElements;  // Elements connected to this bus

};

#endif // BUS_H

