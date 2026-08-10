/**
 * @file SubNetwork.h
 * @brief Hierarchical sub-network view for modular AC/DC grids and converter interfaces.
 */

#ifndef SUBNETWORK_H
#define SUBNETWORK_H

#include "network.h"

/**
 * @brief Named sub-network that inherits Network topology APIs but acts as a non-owning view.
 *
 * Used to represent AC grids, DC grids, and modular partitions. Buses and elements registered
 * here typically live in a parent Network; SubNetwork maps hold pointers without ownership.
 * Output buses mark interface nodes at sub-network boundaries.
 *
 * @ingroup network
 */
class SubNetwork : public Network {
private:
    std::unordered_map<std::string, Bus*> outputBuses;  // Output buses specific to this subnetwork
    std::string subnetworkName;
    bool transformation = true;

public:
    /**
     * @brief Construct an unnamed sub-network with default name "Unnamed_SubNetwork".
     */
    SubNetwork();

    /**
     * @brief Construct a named sub-network.
     * @param name Human-readable sub-network identifier.
     */
    explicit SubNetwork(const std::string& name);

    /**
     * @brief Release sub-network maps without deleting buses or elements.
     *
     * Clears internal maps (buses, elements, connections, output buses, grids, converters)
     * only. Pointers refer to objects owned by the parent Network; they are not deleted here.
     */
    ~SubNetwork();

    /**
     * @brief Get the sub-network name.
     * @return Current sub-network identifier string.
     */
    std::string getSubNetworkName() const { return subnetworkName; }

    /**
     * @brief Set the sub-network name.
     * @param name New identifier string.
     */
    void setSubNetworkName(const std::string& name) { subnetworkName = name; }

    /**
     * @brief Register an output (interface) bus for this sub-network.
     * @param busName Key for the output bus in the local map.
     * @param bus Non-owning pointer to the interface bus; ignored if null.
     */
    void addOutput(const std::string& busName, Bus* bus) {
        if (bus) {
            outputBuses[busName] = bus;
        }
	}

    /**
     * @brief Enable or disable coordinate transformation (e.g. abc to dq) for this sub-network.
     * @param flag True to apply transformation; false otherwise.
     */
	void setTransformation(bool flag) { transformation = flag; }

    /**
     * @brief Query whether coordinate transformation is enabled.
     * @return Current transformation flag.
     */
	bool getTransformation() const { return transformation; }

    /**
     * @brief Const access to the output-bus map.
     * @return Map of output bus names to non-owning bus pointers.
     */
	const std::unordered_map<std::string, Bus*>& getOutputs() const { return outputBuses; }

    /**
     * @brief Get the number of registered output buses.
     * @return Count of entries in the output-bus map.
     */
	const int getNumberOfOutputs() const { return outputBuses.size(); }

    /**
     * @brief Get the sub-network name (alias for getSubNetworkName).
     * @return Sub-network identifier string.
     */
	const string getName() const { return subnetworkName; }

    /**
     * @brief Print sub-network summary (name, bus/element counts, output bus count).
     */
    void printInfo() const;
};

#endif // SUBNETWORK_H
