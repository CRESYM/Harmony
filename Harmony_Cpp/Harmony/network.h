#ifndef NETWORK_H
#define NETWORK_H

#include <string>
#include <unordered_map>
#include <vector>

class Network {
public:
	std::unordered_map<std::string, std::vector<std::string>> connections;

	// Constructor
	Network() {}

	// Function to add a node to the network
	void addNode(const std::string& nodeName) {
		connections[nodeName] = std::vector<std::string>();
	}

	// Function to connect elements in the network
	void connect(const std::string& source, const std::string& destination) {
		connections[source].push_back(destination);
	}

	// Overloaded function to connect multiple elements
	void connect(const std::vector<std::string>& sources, const std::string& destination) {
		for (const auto& source : sources) {
			connect(source, destination);
		}
	}
};

#endif // NETWORK_H
