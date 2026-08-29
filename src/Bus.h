/**
 * @file Bus.h
 * @brief Electrical bus (node) representation with pin count, connections, and OPF metadata.
 */

#ifndef BUS_H
#define BUS_H

#include "Constants.h"

class Element;  // Forward declaration of the Element class

/**
 * @brief A network node connecting one or more element terminals.
 *
 * Tracks connected elements, location (AC/DC), optional OPF parameters, and supports
 * ground detection by name. Does not own connected elements.
 *
 * @ingroup network
 */
class Bus {
public:

    /**
     * @brief Construct a bus with name, grid location, and pin/phase count.
     * @param name Bus identifier (e.g. node name).
     * @param location Grid context string (e.g. "AC" or "DC").
     * @param number Number of pins/phases; must be greater than zero.
     * @throws std::invalid_argument if @p number is not positive.
     */
    Bus(const std::string& name, const std::string& location, int number);

    /**
     * @brief Clear connected-element references; does not delete elements.
     */
    ~Bus();

    /**
     * @brief Get the bus name.
     * @return Bus identifier string.
     */
    std::string getBusName() { return busName; }

    /**
     * @brief Get the number of pins/phases on this bus.
     * @return Pin count.
     */
    int getPinNumber() { return numberPins; }

    /**
     * @brief Get the bus location label (e.g. AC or DC grid).
     * @return Location string.
     */
	std::string getBusLocation() { return busLocation; }

    /**
     * @brief Get a copy of the list of elements connected to this bus.
     * @return Vector of non-owning element pointers.
     */
	vector<Element*> getConnectedElements() { return connectedElements; }

    /**
     * @brief Compare this bus name to a C-string.
     * @param name Name to compare against.
     * @return True if names are equal.
     */
    bool operator==(const char* name);

    /**
     * @brief Replace the bus name.
     * @param name New bus identifier.
     */
    void setBusName(const std::string& name) {
		busName = name;
	}

    /**
     * @brief Check whether this bus represents ground.
     *
     * Ground is detected when the bus name equals "gnd" (case-insensitive).
     *
     * @return True if this bus is ground.
     */
    bool isGround() const {
        std::string lower = busName;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        return lower == "gnd";
    }

    /**
     * @brief Append an element to the connected-elements list.
     * @param elem Non-owning pointer to the element; must not be null.
     */
    void attachElement(Element* elem);

    /**
     * @brief Remove an element from the connected-elements list without deleting it.
     * @param elem Element pointer to detach.
     */
    void detachElement(Element* elem);

    /**
     * @brief Print connected element information to standard output.
     */
    void printConnectedElements();

    /**
     * @brief Compute AC bus quantities for optimal power flow.
     * @param busAC Output map of AC bus OPF results keyed by quantity name.
     * @param globalParams Shared global OPF parameters.
     */
    void computePowerFlowAC(std::map<std::string, double>& busAC,
        std::map<std::string, double>& globalParams) const;

    /**
     * @brief Compute DC bus quantities for optimal power flow.
     * @param busDC Output map of DC bus OPF results keyed by quantity name.
     * @param globalParams Shared global OPF parameters.
     */
    void computePowerFlowDC(std::map<std::string, double>& busDC,
        std::map<std::string, double>& globalParams) const;

    /**
     * @brief Store OPF-related metadata for this bus (area, voltage limits, etc.).
     * @param info Map of OPF parameter names to values.
     */
    void setOPFInfo(const std::map<std::string, double>& info) {
        busOPFInfo = info;
	}

    /**
     * @brief Retrieve stored OPF metadata.
     * @return Copy of the bus OPF info map.
     */
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
