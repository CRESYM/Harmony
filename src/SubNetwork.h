#ifndef SUBNETWORK_H
#define SUBNETWORK_H

#include "network.h"

// The SubNetwork class inherits all behavior from Network,
// but is distinct in identity � useful for modular, hierarchical systems
class SubNetwork : public Network {
private:
    std::unordered_map<std::string, Bus*> outputBuses;  // Output buses specific to this subnetwork
    std::string subnetworkName;
    bool transformation = true;

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

    void addOutput(const std::string& busName, Bus* bus) {
        if (bus) {
            outputBuses[busName] = bus;
        }
	}

	void setTransformation(bool flag) { transformation = flag; }
	bool getTransformation() const { return transformation; }

	// Get output buses
	const std::unordered_map<std::string, Bus*>& getOutputs() const { return outputBuses; }
	const int getNumberOfOutputs() const { return outputBuses.size(); }
	const string getName() const { return subnetworkName; }

    // Optional: print info specific to SubNetwork
    void printInfo() const;
};

#endif // SUBNETWORK_H