#ifndef _CABLE_H_

#define _CABLE_H_

#include "Transmission_line.h"

#include <unordered_map>
#include <vector>
#include <tuple>
#include <string>

// Define the Conductor class presents conducting layer
class Conductor {
public:
	// Constructor
	Conductor(double ri = 0, double ro = 0, double resistivity = 0, double permeability = 1, double area = 0)
		: ri(ri), ro(ro), resistivity(resistivity), permeability(permeability), area(area) {}

	// Member variables
	double ri; //conductor inner radius
	double ro; //conductor outer radius
	double resistivity; //conductor resistivity ρ [Ωm]
	double permeability;  //relative permeability μᵣ
	double area; //nominal area

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

// Define the Insulator class presents insulating layer
class Insulator {
public:
	// Constructor
	Insulator(double ri = 0, double ro = 0, double permittivity = 1, double permeability = 1, double innerSemiConductorOuterRadius = 0, double outerSemiConductorInnerRadius = 0)
		: ri(ri), ro(ro), permittivity(permittivity), permeability(permeability), innerSemiConductorOuterRadius(innerSemiConductorOuterRadius), outerSemiConductorInnerRadius(outerSemiConductorInnerRadius) {}

	// Member variables
	double ri;  //insulator inner radius
	double ro;  //insulator outer radius
	double permittivity; //relative permittivity ϵᵣ
	double permeability; //relative permeability μᵣ
	//If a semiconductor is present, in an insulator, we have: rᵢ < semiconductor < a + a < insulator < b + b < semiconductor < rₒ

	double innerSemiConductorOuterRadius;
	double outerSemiConductorInnerRadius;
	double a; //inner semiconductor outer radius -> Inner semiconductor rᵢ < r < a
	double b; //outer semiconductor inner radius -> Outer semiconductor b < r < rₒ
};

// Define the Cable class as a subtype (<:) of abstract type Transmission_line
class Cable {
public:
	std::vector<std::vector<double>> P; //initialization (still no value inside) of the array P with datatype Basic
	std::vector<std::vector<double>> Z; //same as ow above
	int n = 3;

	// Public member functions
	void setLength(double length) { this->length = length; } 
	void addConductor(const std::string& key, const Conductor& conductor) { conductors[key] = conductor; } 
	void addInsulator(const std::string& key, const Insulator& insulator) { insulators[key] = insulator; } 
	void addPosition(double x, double y) { positions.push_back(std::make_pair(x, y)); }
	void setEarthParameters(int mu, int epsilon, int rho) { earth_parameters = std::make_tuple(mu, epsilon, rho); } 
	void setConfiguration(const std::string& configuration) { this->configuration = configuration; } 
	void setType(const std::string& type) { this->type = type; } // 
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
	double length = 0;   //line length [m]
	//dictionary with a particular order. Key: Symbol-> C1, C2, C3 and C4. Value: Conductor-> Mutable Struct Conductor, defined above
	std::unordered_map<std::string, Conductor> conductors = { {"Symbol", Conductor(/* Constructor arguments for Conductor */)},
															  {"Conductor", Conductor(/* Constructor arguments for Conductor */)} };
	//dictionary with a particular order. Key: Symbol-> I1, I2, I3 and I4. Value: Insulator-> Mutable Struct Insulator, defined above
	std::unordered_map<std::string, Insulator> insulators = { {"Symbol", Insulator(/* Constructor arguments for Conductor */)},
															  {"Conductor", Insulator(/* Constructor arguments for Conductor */)} };
	//indicates all variables are real number, vector composed by tuple of real numbers. e.g. positions=[(0,0),(1,1)]. Cables positions 1st:x=0, y=0. 2nd: x=1, y=1.
	std::vector<std::pair<double, double>> positions;
	//(μᵣ, ϵᵣ, ρ) in units ([], [], [Ωm]) compact way of representing the type for a tuple of length N where all elements are of type Int or Float64.
	std::tuple<int, int, int> earth_parameters;

	std::string configuration = "coaxial";//Configuration is a datatype symbol with value coaxial Symbol -> Type of data. Symbols can be entered using the quote operator ":"
	std::string type = "underground";
	bool eliminate = true;
	std::string transformation;
};

#endif

