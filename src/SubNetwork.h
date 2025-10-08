#ifndef SUBNETWORK_H
#define SUBNETWORK_H

#include "Network.h"

// The SubNetwork class inherits all behavior from Network,
// but is distinct in identity — useful for modular, hierarchical systems
class SubNetwork : public Network {
private:
    std::unordered_map<std::string, Bus*> outputBuses;  // Output buses specific to this subnetwork
    std::string subnetworkName;

public:
    // Default constructor
    SubNetwork();

    // Constructor with a name
    explicit SubNetwork(const std::string& name);

    // Destructor
    ~SubNetwork();

    // Optional: identify this object as a SubNetwork
    std::string getSubNetworkName() const { return subnetworkName; }
    void setSubNetworkName(const std::string& name) { subnetworkName = name; }

    // Optional: print info specific to SubNetwork
    void printInfo() const;
};

#endif // SUBNETWORK_H