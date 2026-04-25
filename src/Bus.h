#ifndef BUS_H
#define BUS_H

#include "Constants.h"

class Element;  // Forward declaration of the Element class

class Bus {
public:

    // Constructor to initialize the bus with a name and number of pins/phases
    Bus(const std::string& name, const std::string& location, int number);

    // Destructor to clean up resources
    ~Bus();

    // Getters to access the bus name and the number of pins/phases
    std::string getBusName() { return busName; }
    int getPinNumber() { return numberPins; }
	std::string getBusLocation() { return busLocation; }
	vector<Element*> getConnectedElements() { return connectedElements; }

    //  Operator overload to compare the bus name with a string
    bool operator==(const char* name);

    void setBusName(const std::string& name) {
		busName = name;
	}

    /// A bus is ground if its name is "gnd" (case-insensitive)
    bool isGround() const {
        std::string lower = busName;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        return lower == "gnd";
    }

    // Function to attach an element to the bus
    void attachElement(Element* elem);

    // Function to print all the elements connected to this bus
    void printConnectedElements();

	// Functions for setting and getting OPF information
    void computePowerFlowAC(std::map<std::string, double>& busAC,
        std::map<std::string, double>& globalParams) const;
    void computePowerFlowDC(std::map<std::string, double>& busDC,
        std::map<std::string, double>& globalParams) const;
    void setOPFInfo(const std::map<std::string, double>& info) {
        busOPFInfo = info;
	}
    std::map<std::string, double> getOPFInfo() const {
        return busOPFInfo;
	}

private:
    std::string busName; // The name of the bus
	std::string busLocation; // The location of the bus (e.g., "AC" or "DC")
    int numberPins; // The number of pins (phases) of the bus
    std::vector<Element*> connectedElements;  // Elements connected to this bus

	std::map<std::string, double> busOPFInfo; // Additional information about the bus OPF model
	// This includes area and voltage limits.
};

#endif // BUS_H

