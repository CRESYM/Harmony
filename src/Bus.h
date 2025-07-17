#ifndef BUS_H
#define BUS_H

#include "Constants.h"

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


    // Default parameter values for bus creation
    static const std::vector<std::string> DefaultACBusParams;
    static const std::vector<std::string> DefaultDCBusParams;

    // Predefined bus configurations
    static const std::vector<std::vector<std::string>>& getDefaultACBusInfo();
    static const std::vector<std::vector<std::string>>& getDefaultDCBusInfo();

private:
    std::string busName; // The name of the bus
    int numberPins; // The number of pins (phases) of the bus
    std::vector<Element*> connectedElements;  // Elements connected to this bus

    // Static vectors containing default buses info
    static std::vector<std::vector<std::string>> defaultACBusInfo;
    static std::vector<std::vector<std::string>> defaultDCBusInfo;
};

#endif // BUS_H

