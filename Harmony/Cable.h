#ifndef _CABLE_H_

#define _CABLE_H_

#include "Transmission_line.h"

#include <unordered_map>
#include <vector>
#include <tuple>
#include <string>

// Define the Conductor class
class Conductor {
public:
	// Constructor
	Conductor(double ri = 0, double ro = 0, double resistivity = 0, double permeability = 1, double area = 0)
		: ri(ri), ro(ro), resistivity(resistivity), permeability(permeability), area(area) {}

	// Member variables
	double ri;
	double ro;
	double resistivity;
	double permeability;
	double area;

	// Getter functions
	double getInnerRadius() const { return ri; }
	double getOuterRadius() const { return ro; }
	double getResistivity() const { return resistivity; }
	double getPermeability() const { return permeability; }
	double getArea() const { return area; }

	// Setter function for resistivity
	void setResistivity(double newResistivity) {
		resistivity = newResistivity;
	}
};

// Define the Insulator class
class Insulator {
public:
	// Constructor
	Insulator(double ri = 0, double ro = 0, double permittivity = 1, double permeability = 1, double innerSemiConductorOuterRadius = 0, double outerSemiConductorInnerRadius = 0)
		: ri(ri), ro(ro), permittivity(permittivity), permeability(permeability), innerSemiConductorOuterRadius(innerSemiConductorOuterRadius), outerSemiConductorInnerRadius(outerSemiConductorInnerRadius) {}

	// Member variables
	double ri;
	double ro;
	double permittivity;
	double permeability;
	double innerSemiConductorOuterRadius;
	double outerSemiConductorInnerRadius;
	double a; // Add 'a' as a member variable
	double b; // Add 'b' as a member variable
};

// Define the Cable class
class Cable {
public:
	std::vector<std::vector<double>> P;
	std::vector<std::vector<double>> Z;
	int n = 3;

	// Public member functions
	void setLength(double length) { this->length = length; }
	void addConductor(const std::string& key, const Conductor& conductor) { conductors[key] = conductor; }
	void addInsulator(const std::string& key, const Insulator& insulator) { insulators[key] = insulator; }
	void addPosition(double x, double y) { positions.push_back(std::make_pair(x, y)); }
	void setEarthParameters(int mu, int epsilon, int rho) { earth_parameters = std::make_tuple(mu, epsilon, rho); }
	void setConfiguration(const std::string& configuration) { this->configuration = configuration; }
	void setType(const std::string& type) { this->type = type; }
	void setEliminate(bool eliminate) { this->eliminate = eliminate; }
	void setTransformation(const std::string& value) { transformation = value; }

	// Function to set P matrix
	void setP(const std::vector<std::vector<double>>& newP) {
		P = newP;
	}

	// Function to set Z matrix
	void setZ(const std::vector<std::vector<double>>& newZ) {
		Z = newZ;
	}

	/*// Function to update the elements of the P matrix
	void updatePMatrix(int i, double P_i) {
		for (int j = 0; j <= i; j++) {
			P[j][i] += P_i;
			P[i][j] += P_i;
		}
	}*/

	// Function to access a conductor from the conductors map
	Conductor* getConductor(const std::string& key) {
		auto it = conductors.find(key);
		if (it != conductors.end()) {
			return &(it->second); // Return a pointer to the conductor if found
		}
		else {
			return nullptr; // Return nullptr if conductor not found
		}
	}

	// Function to modify a conductor in the conductors map
	void updateConductor(const std::string& key, const Conductor& conductor) {
		conductors[key] = conductor;
	}

	// Function to remove a conductor from the conductors map
	void removeConductor(const std::string& key) {
		conductors.erase(key);
	}

/*	const std::unordered_map<std::string, Conductor>& getConductors() const {
		return conductors;
	}*/

	// Define a member function in the Cable class to access the conductors map
	const std::unordered_map<std::string, Conductor>& getCableConductors() const {
		return conductors;
	}

	// Function to access an insulator from the insulators map
	Insulator* getInsulator(const std::string& key) {
		auto it = insulators.find(key);
		if (it != insulators.end()) {
			return &(it->second);
		}
		return nullptr; // Insulator not found
	}

	// Define a member function in the Cable class to access the insulators map
	const std::unordered_map<std::string, Insulator>& getCableInsulators() const {
		return insulators;
	}

	// Functions for handling properties
	bool hasProperty(const std::string& key) { return (conductors.find(key) != conductors.end() || insulators.find(key) != insulators.end()); }
	void setProperty(const std::string& key, const std::string& value);
	bool isConductor(const std::string& key) { return (conductors.find(key) != conductors.end()); }
	bool isInsulator(const std::string& key) { return (insulators.find(key) != insulators.end()); }

	std::tuple<int, int, int> getEarthParameters() const {
		return earth_parameters;
	}

	const std::vector<std::pair<double, double>>& getPositions() const {
		return positions;
	}

	std::string getType() const {
		return type;
	}

	friend void cable(Cable& c, const std::vector<std::vector<double>>& P, const std::vector<std::vector<double>>& Z, const std::unordered_map<std::string, std::vector<std::pair<double, double>>>& kwargs, bool transformation);
	// Constructor
	Cable();

	// Destructor
	~Cable();

private:
	// Private member variables
	double length = 0;
	std::unordered_map<std::string, Conductor> conductors = { {"Symbol", Conductor(/* Constructor arguments for Conductor */)},
															  {"Conductor", Conductor(/* Constructor arguments for Conductor */)} };
	std::unordered_map<std::string, Insulator> insulators = { {"Symbol", Insulator(/* Constructor arguments for Conductor */)},
															  {"Conductor", Insulator(/* Constructor arguments for Conductor */)} };
	std::vector<std::pair<double, double>> positions;
	std::tuple<int, int, int> earth_parameters;
	std::string configuration = "coaxial";
	std::string type = "underground";
	//std::vector<std::vector<double>> P;
	//std::vector<std::vector<double>> Z;
	bool eliminate = true;
	std::string transformation;

	//std::pair<std::vector<std::vector<std::complex<double>>>, std::vector<std::vector<std::complex<double>>>> eval_parameters(const std::complex<double>& s) const;
};

#endif

