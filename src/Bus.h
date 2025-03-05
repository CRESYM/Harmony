#ifndef BUS_H
#define BUS_H

#include <vector>
#include <string>
#include <symengine/matrix.h>

using namespace SymEngine;

class Element;  // Forward declaration of the Element class

class Bus {
public:

    // Constructor to initialize the bus with a name and number of pins/phases
    Bus(const std::string& name, int number);

    // Destructor to clean up resources
    ~Bus();

    // Getters to access the bus name and the number of pins/phases
    std::string getBusName() { return busName; }
    int getPinNumber() { return numberPins; }

    //  Operator overload to compare the bus name with a string
    bool operator==(const char* name);

    // Function to attach an element to the bus
    void attachElement(Element* elem);

    // Function to print all the elements connected to this bus
    void printConnectedElements();

private:
    std::string busName; // The name of the bus
    int numberPins; // The number of pins (phases) of the bus
    std::vector<Element*> connectedElements;  // Elements connected to this bus

};

#endif // BUS_H

