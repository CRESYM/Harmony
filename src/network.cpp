/**
 * @file network.cpp
 * @brief Implementation of Top-level electrical network container: buses, elements, connections, and hierarchical grids.
 */
#include "Bus.h"
#include "SubNetwork.h"
#include "Include_components.h"


Network::Network() : pins(0) {
}


Network::~Network() {
    // SubNetworks hold non-owning views; delete shells before owned buses/elements
    empty_areas();

    for (auto& [name, bus] : buses)
        delete bus;
    buses.clear();

    for (auto& [designator, elem] : elements)
        delete elem;
    elements.clear();

    connections.clear();
}

// Function to add a bus to the network
void Network::addBus(Bus* bus) {
    buses[bus->getBusName()] = bus;
    // Increment the total pin count if the bus is not ground
    if (!bus->isGround())
        pins += bus->getPinNumber();
}

// Function to add a bus to the network
void Network::addBus(const std::string& busName, Bus* bus) {
    buses[busName] = bus;
    if (!bus->isGround())
        pins += bus->getPinNumber();
}

// Function to add an element to the network
void Network::addElement(Element* elem) {
    elements[elem->getElementSymbol()] = elem;
}

// Function to add an element to the network
void Network::addElement(const std::string& designator, Element* elem) {
    elements[designator] = elem;
}

// Function to connect an element to a bus
void Network::connectElementToBus(Element* elem, int terminal, Bus* bus) {
    if (elements.find(elem->getElementSymbol()) == elements.end()) {
        elements[elem->getElementSymbol()] = elem;
    }
    // Store bus if not already present (use bus name as key only if unique)
    bool busFound = false;
    for (auto& [key, b] : buses) {
        if (b == bus) { busFound = true; break; }
    }
    if (!busFound) {
        buses[bus->getBusName()] = bus;
    }

    // Pin-count validation (skip for ground — ground accepts any pin count)
    if (bus->getBusName() != "gnd") {
        int elemPins = 0;
        // Determine how many pins this terminal represents
        if (dynamic_cast<Converter*>(elem)) {
            // Converter: terminal 1 = input_pins (AC), terminal 2 = output_pins (DC)
            elemPins = (terminal == 1) ? elem->getInputPins() : elem->getOutputPins();
        }
        else {
            elemPins = elem->getInputPins();
        }

        if (elemPins != bus->getPinNumber()) {
            std::cerr << "[WARNING] Pin mismatch: " << elem->getElementSymbol()
                << " terminal " << terminal << " has " << elemPins
                << " pins, but bus " << bus->getBusName()
                << " has " << bus->getPinNumber() << " pins.\n";
        }
    }

    elem->attachBus(bus, terminal);
    bus->attachElement(elem);
    connections[bus].push_back(elem);
}

// Function to delete an element from the network
void Network::deleteElement(const std::string& designator) {
    auto it = elements.find(designator);
    if (it == elements.end()) {
        throw std::invalid_argument("Element not found");
    }

    Element* elem = it->second;

    for (auto& [bus, elemList] : connections) {
        elemList.erase(std::remove(elemList.begin(), elemList.end(), elem), elemList.end());
    }

    for (auto& [busName, bus] : buses) {
        bus->detachElement(elem);
    }

    converters.erase(designator);
    elements.erase(it);
    delete elem;
}

// Function to delete a bus from the network
void Network::deleteBus(const std::string& busName) {
    auto it = buses.find(busName);
    if (it == buses.end()) {
        throw std::invalid_argument("Bus not found");
    }

    Bus* bus = it->second;

    auto conn_it = connections.find(bus);
    if (conn_it != connections.end() && !conn_it->second.empty()) {
        throw std::invalid_argument("Cannot delete bus with connected elements");
    }

    connections.erase(bus);
    if (!bus->isGround()) {
        pins -= bus->getPinNumber();
    }

    buses.erase(it);
    delete bus;
}

// Function to print the connections between elements and buses
void Network::printConnections() {
    for (const auto& connection : connections) {
        Bus* bus = connection.first;
        std::cout << "Bus " << bus->getBusName() << " has the following elements attached:\n";
        for (Element* elem : connection.second) {
            std::cout << "  - " << elem->getElementSymbol() << std::endl; // Use the getter method
        }
        //std::cout << "Total number of pins is " << pins << std::endl;
    }
}

void Network::printBuses() {
    for (const auto& bus : buses) {
        std::cout << "Bus: " << bus.first << ", Pin Number: " << bus.second->getPinNumber() << std::endl;
    }
}

void Network::printElements() {
    for (const auto& elem : elements) {
        std::cout << "Element: " << elem.first << ", Symbol: " << elem.second->getElementSymbol() << std::endl;
    }
}

/**
 * @brief Prints a summary of the detected AC/DC grids and converters.
 */
void Network::print_summary() const {
    std::cout << "\n================= NETWORK SUMMARY =================\n";

    // --- AC Grids ---
    std::cout << "\n--- AC Grids (" << ac_grids.size() << ") ---\n";
    if (ac_grids.empty()) {
        std::cout << "No AC subnetworks found.\n";
    }
    else {
        for (const auto& [name, sub] : ac_grids) {
            std::cout << "AC Grid: " << name << "\n";
            std::cout << "  Buses (" << sub->getBuses().size() << "): ";
            for (const auto& [busName, busPtr] : sub->getBuses())
                std::cout << busName << " ";
            std::cout << "\n  Elements (" << sub->getElements().size() << "): ";
            for (const auto& [elName, elPtr] : sub->getElements())
                std::cout << elName << " ";
            std::cout << "\n  Outputs (" << sub->getOutputs().size() << "): ";
            for (auto& [outName, outBus] : sub->getOutputs())
                std::cout << outName << " ";
            std::cout << "\n\n";
        }
    }

    // --- DC Grids ---
    std::cout << "\n--- DC Grids (" << dc_grids.size() << ") ---\n";
    if (dc_grids.empty()) {
        std::cout << "No DC subnetworks found.\n";
    }
    else {
        for (const auto& [name, sub] : dc_grids) {
            std::cout << "DC Grid: " << name << "\n";
            std::cout << "  Buses (" << sub->getBuses().size() << "): ";
            for (const auto& [busName, busPtr] : sub->getBuses())
                std::cout << busName << " ";
            std::cout << "\n  Elements (" << sub->getElements().size() << "): ";
            for (const auto& [elName, elPtr] : sub->getElements())
                std::cout << elName << " ";
            std::cout << "\n  Outputs (" << sub->getOutputs().size() << "): ";
            for (auto& [outName, outBus] : sub->getOutputs())
                std::cout << outName << " ";
            std::cout << "\n\n";
        }
    }

    // --- Converters ---
    std::cout << "\n--- Converters (" << converters.size() << ") ---\n";
    if (converters.empty()) {
        std::cout << "No converters detected.\n";
    }
    else {
        for (const auto& [name, elem] : converters) {
            std::cout << "Converter: " << name << "\n";
        }
    }

    std::cout << "\n===============================================================\n";
}


/**
 * @brief Populates AC and DC grid subnetworks from the main network.
 *
 * This function iterates through all buses in the provided network, categorizes them
 * into AC or DC areas based on their location string, and groups them into
 * `SubNetwork` objects. It also identifies and separates MMC converters.
 *
 * @param net Pointer to the main Network object.
 */
void Network::add_areas() {
    // This function can be implemented to categorize and add AC and DC grids to the system

    // Iterate over all buses in the network
    for (const auto& bus_pair : buses) {
        Bus* bus = bus_pair.second;
        if (!bus) continue;

        std::string area = bus->getBusLocation(); // Assume Bus has getBusLocation()

        // Skip if location string is invalid
        if (area.empty()) continue;

        bool is_ac = (area[0] == 'A' || area[0] == 'a') && (area[1] == 'C' || area[1] == 'c');
        bool is_dc = (area[0] == 'D' || area[0] == 'd') && (area[1] == 'C' || area[1] == 'c');

        // Pointer to selected map
        auto& grid_map = is_ac ? ac_grids : dc_grids;
        auto& grid_names = is_ac ? ac_grid_names : dc_grid_names;

        if (!is_ac && !is_dc)
            continue; // Ignore buses not belonging to AC/DC areas

        // Create subnetwork if not already existing
        if (grid_map.find(area) == grid_map.end()) {
            grid_map[area] = new SubNetwork(area);
            grid_names.push_back(area);
        }

        // Retrieve the SubNetwork
        SubNetwork* sub = grid_map[area];

        // Add bus to the SubNetwork
        sub->addBus(bus);

        // Add connected elements
        auto conn_it = connections.find(bus);
        if (conn_it != connections.end()) {
            for (auto& elem : conn_it->second) {
                if (!elem) continue;

                // --- Check if element is a Converter dynamically ---
                Converter* converter = dynamic_cast<Converter*>(elem);
                if (converter) {
                    std::string conv_name = elem->getElementSymbol();
                    if (converters.find(conv_name) == converters.end()) {
                        converters[conv_name] = elem;
                        //std::cout << "Detected MMC converter: " << conv_name << "\n";
                    }

                    std::string loc = converter->getElementLocation();
                    if (loc.empty()) continue;

                    // Normalize and parse format "ACi_DCj" (case-insensitive)
                    std::string loc_upper = loc;
                    std::transform(loc_upper.begin(), loc_upper.end(), loc_upper.begin(), ::toupper);

                    std::regex pattern(R"(AC(\d+)_DC(\d+))", std::regex::icase);
                    std::smatch match;
                    if (std::regex_search(loc_upper, match, pattern) && match.size() == 3) {
                        std::string ac_area = "AC" + match[1].str();
                        std::string dc_area = "DC" + match[2].str();

                        // Ensure both grids exist
                        if (ac_grids.find(ac_area) == ac_grids.end()) {
                            ac_grids[ac_area] = new SubNetwork(ac_area);
                        }
                        if (dc_grids.find(dc_area) == dc_grids.end()) {
                            dc_grids[dc_area] = new SubNetwork(dc_area);
                        }

                        // Identify converter terminal buses (AC and DC)
                        auto conns = converter->getConnections();
                        Bus* ac_bus = nullptr;
                        Bus* dc_bus = nullptr;

                        for (const auto& [bus, terminal] : conns) {
                            if (!bus) continue;
                            std::string bname = bus->getBusLocation();
                            std::string bname_lower = bname;
                            std::transform(bname_lower.begin(), bname_lower.end(), bname_lower.begin(), ::tolower);

                            if (bname_lower.rfind("ac", 0) == 0)
                                ac_bus = bus;
                            else if (bname_lower.rfind("dc", 0) == 0)
                                dc_bus = bus;
                        }

                        // Add buses as outputs for their corresponding grids
                        if (ac_bus) {
                            ac_grids[ac_area]->addOutput(ac_bus->getBusName(), ac_bus);
                        }
                        if (dc_bus) {
                            dc_grids[dc_area]->addOutput(dc_bus->getBusName(), dc_bus);
                        }
                    }
                    else {
                        std::cerr << "[WARN] Converter " << converter->getElementSymbol()
                            << " has invalid location format: " << loc << "\n";
                    }
                }
                else {
                    sub->addElement(elem);

                }

                elem->setTransformation(sub->getTransformation()); // Set transformation flag for non-MMC elements

            }
        }
    }

    // Optional summary output
    //std::cout << "\n=== Area Summary ===\n";
    //std::cout << "AC Grids Detected: " << ac_grids.size() << "\n";
    //for (const auto& [name, sub] : ac_grids) {
    //    std::cout << "  - " << name << " (" << sub->getBuses().size() << " buses, "
    //        << sub->getElements().size() << " elements, " << sub->getOutputs().size() << " outputs)\n";
    //}

    //std::cout << "DC Grids Detected: " << dc_grids.size() << "\n";
    //for (const auto& [name, sub] : dc_grids) {
    //    std::cout << "  - " << name << " (" << sub->getBuses().size() << " buses, "
    //        << sub->getElements().size() << " elements, " << sub->getOutputs().size() << " outputs)\n";
    //}

    //std::cout << "Converters Detected: " << converters.size() << "\n";
    //for (const auto& [name, elem] : converters) {
    //    std::cout << "  - " << name << " (" << elem->getElementSymbol() << ")\n";
    //}
}