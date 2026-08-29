/**
 * @file network.h
 * @brief Top-level electrical network container: buses, elements, connections, and hierarchical grids.
 */

#ifndef NETWORK_H
#define NETWORK_H

#include "Constants.h"

#include <memory>

class Bus;
class Element;
class SubNetwork;

/**
 * @brief Maps each bus to the elements connected at its terminals.
 *
 * Outer key is a bus pointer; value is the list of elements attached to that bus.
 */
using Net = std::unordered_map<Bus*, std::vector<Element*>>;

/**
 * @defgroup network Network topology
 * @brief Core classes for representing electrical networks, buses, and hierarchical sub-networks.
 * @{
 */

/**
 * @brief Root network model holding buses, elements, and AC/DC grid hierarchy.
 *
 * Owns buses and elements registered via addBus/addElement and deletes them in the destructor.
 * Sub-networks (AC/DC grids) are owned separately and released through empty_areas().
 */
class Network {
protected:
    std::unordered_map<std::string, Bus*> buses;
    std::unordered_map<std::string, Element*> elements;
    std::unordered_map<Bus*, std::vector<Element*>> connections;

    int pins = 0;

    std::vector<std::string> ac_grid_names;
    std::vector<std::string> dc_grid_names;

    std::unordered_map<std::string, SubNetwork*> ac_grids;
    std::unordered_map<std::string, SubNetwork*> dc_grids;
    std::unordered_map<std::string, Element*> converters;

    /** When true, buses/elements registered via addBus/addElement are owned. */
    bool ownsResources_ = true;

private:
    std::unordered_map<std::string, std::unique_ptr<Bus>> ownedBuses_;
    std::unordered_map<std::string, std::unique_ptr<Element>> ownedElements_;
    std::unordered_map<std::string, std::unique_ptr<SubNetwork>> ownedGrids_;

public:

    /**
     * @brief Construct an empty network with zero pins.
     */
    Network();

    /**
     * @brief Destroy the network and all owned buses, elements, and grid sub-networks.
     */
    virtual ~Network();

    /** @brief Register an owned bus (transfers ownership on root networks). */
    void addBus(const std::string& busName, std::unique_ptr<Bus> bus);

    /** @brief Register a bus using its own name as the map key. */
    void addBus(Bus* bus);

    /** @brief Register a bus under an explicit name (transfers ownership on root networks). */
    void addBus(const std::string& busName, Bus* bus);

    /** @brief Register a non-owning bus pointer (used by @ref SubNetwork views). */
    void addBusView(const std::string& busName, Bus* bus);

    /** @brief Register an owned element. */
    void addElement(const std::string& designator, std::unique_ptr<Element> elem);

    /** @brief Register an element using an auto-generated designator. */
    void addElement(Element* elem);

    /** @brief Register an element under a specific designator string. */
    void addElement(const std::string& designator, Element* elem);

    /** @brief Register a non-owning element pointer (used by @ref SubNetwork views). */
    void addElementView(const std::string& designator, Element* elem);

    /**
     * @brief Connect an element terminal to a bus and record the association in both directions.
     * @param elem Element whose terminal is being wired.
     * @param terminal Terminal index on the element (0-based).
     * @param bus Target bus.
     */
    void connectElementToBus(Element* elem, int terminal, Bus* bus);

    /**
     * @brief Remove and destroy an element by designator.
     * @param designator Designator of the element to delete.
     */
    void deleteElement(const std::string& designator);

    /**
     * @brief Remove and destroy a bus by name.
     * @param busName Name of the bus to delete.
     */
    void deleteBus(const std::string& busName);

    /**
     * @brief Print bus-to-element connection listing to standard output.
     */
    void printConnections();

    /**
     * @brief Print all registered buses to standard output.
     */
	void printBuses();

    /**
     * @brief Print all registered elements to standard output.
     */
    void printElements();

    /**
     * @brief Print a concise summary of network contents (counts and names).
     */
    void print_summary() const;

    /**
     * @brief Const access to the bus name-to-pointer map.
     * @return Reference to the internal bus map.
     */
    const std::unordered_map<std::string, Bus*>& getBuses() const { return buses; }

    /**
     * @brief Const access to the element designator-to-pointer map.
     * @return Reference to the internal element map.
     */
    const std::unordered_map<std::string, Element*>& getElements() const { return elements; }

    /**
     * @brief Mutable access to the element designator-to-pointer map.
     * @return Reference to the internal element map.
     */
    std::unordered_map<std::string, Element*>& getElements() { return elements; }

    /**
     * @brief Const access to the bus-to-connected-elements map.
     * @return Reference to the internal connections map.
     */
    const std::unordered_map<Bus*, std::vector<Element*>>& getConnections() const { return connections; }

    /**
     * @brief Mutable access to the bus-to-connected-elements map.
     * @return Reference to the internal connections map.
     */
    std::unordered_map<Bus*, std::vector<Element*>>& getConnections() { return connections; }

    /**
     * @brief Populate AC and DC grid sub-networks from the current flat network topology.
     */
    void add_areas();

    /**
     * @brief Destroy all grid sub-networks and clear grid/converter registries.
     *
     * Deletes owned SubNetwork shells in ac_grids and dc_grids; does not delete buses or
     * elements referenced by those views.
     */
    void empty_areas();

    /**
     * @brief Check whether both AC and DC grid maps are empty.
     * @return True if no AC or DC grid sub-networks are registered.
     */
    bool is_area_empty() {return ac_grids.empty() && dc_grids.empty(); }

    /**
     * @brief Return ordered names of registered AC grids.
     * @return Vector of AC grid name strings.
     */
    vector<string> get_ac_grid_names() { return ac_grid_names; };

    /**
     * @brief Return ordered names of registered DC grids.
     * @return Vector of DC grid name strings.
     */
    vector<string> get_dc_grid_names() { return dc_grid_names; };

    /**
     * @brief Mutable access to the AC grid sub-network map.
     * @return Reference to name-to-SubNetwork map for AC areas.
     */
    std::unordered_map<std::string, SubNetwork*>& get_ac_grids() { return ac_grids; }

    /**
     * @brief Mutable access to the DC grid sub-network map.
     * @return Reference to name-to-SubNetwork map for DC areas.
     */
    std::unordered_map<std::string, SubNetwork*>& get_dc_grids() { return dc_grids; }

    /**
     * @brief Mutable access to the converter element map.
     * @return Reference to designator-to-Element map for converters.
     */
    std::unordered_map<std::string, Element*>& get_converters() { return converters; }

};

/** @} */ // end of network group

#endif // NETWORK_H
