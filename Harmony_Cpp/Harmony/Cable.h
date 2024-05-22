#ifndef _CABLE_H_
#define _CABLE_H_

#include "Transmissionline.h"
#include "Element.h"

#include <basic.h>
#include <symbol.h>
#include <complex.h>
#include <rational.h>
#include <dict.h>
#include <eval_double.h>
#include <unordered_map>
#include <vector>
#include <tuple>
#include <string>
#include <utility>
#include <memory>

using namespace SymEngine; 
using SymEngine::symbol;
using SymEngine::Basic;
using SymEngine::RCP;
using SymEngine::eval_double;

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
class Cable : public TransmissionLine {
public:
	// Constructor
	Cable() :
		length(0),
		configuration("coaxial"),
		type("underground"),
		transformation(false),
		eliminate(true),
		real_part(0), // Initialize real part to 0
		imag_part(0),
		//s(SymEngine::Complex(0, 0)) // Initialize s with default value
		s(nullptr)
	{}

	//const SymEngine::Complex& getSymbolS() const { return s; }

	// Public member functions
	std::vector<std::vector<double>> P;
	std::vector<std::vector<double>> Z;


	//void setLength(double length) { this->length = length; }
	void setLength(double newLength) { length = newLength; }
	void addConductor(const std::string& key, const Conductor& conductor) { conductors[key] = conductor; }
	void addInsulator(const std::string& key, const Insulator& insulator) { insulators[key] = insulator; }
	//void addPosition(double x, double y) { positions.push_back(std::make_pair(x, y)); }
	void addPosition(double x, double y) { positions.emplace_back(x, y); }
	//void setEarthParameters(int mu, int epsilon, int rho) { earth_parameters = std::make_tuple(mu, epsilon, rho); }
	//void setConfiguration(const std::string& configuration) { this->configuration = configuration; }
	//void setTransformation(const std::string& value) { transformation = value; }
	void setEarthParameters(double mu, double epsilon, double rho) { earth_parameters = std::make_tuple(mu, epsilon, rho); }
	void setConfiguration(const std::string& newConfig) { configuration = newConfig; }
	//void setTransformation(const std::string& value) { transformation = value; }
	void setTransformation(const bool value) { transformation = value; }
	void setEliminate(bool value) { eliminate = value; } //to change the value of eliminate


	// Setters for P and Z matrices
	void setP(const std::vector<std::vector<double>>& newP) { P = newP; }
	void setZ(const std::vector<std::vector<double>>& newZ) { Z = newZ; }


	// Setter for s
	void setSymbolS(const SymEngine::Complex* symbol) {
		s = symbol;
	}

	// Getter for s
	const SymEngine::Complex* getSymbolS() const {
		return s;
	}

	// Getter methods
	double getLength() const { return length; }
	//const std::unordered_map<std::string, Conductor>& getConductors() const { return conductors; }
	//const std::unordered_map<std::string, Insulator>& getInsulators() const { return insulators; }
	const std::tuple<double, double, double>& getEarthParameters() const { return earth_parameters; }
	const std::vector<std::pair<double, double>>& getPositions() const { return positions; }
	const std::string& getConfiguration() const { return configuration; }
	const std::string& getType() const { return type; }
	bool getEliminate() const { return eliminate; }

	// Getters for symbolic matrices
	//std::vector<std::vector<RCP<const Basic>>> getCZ() const;
	//std::vector<std::vector<RCP<const Basic>>> getCP() const;

	// Getters for symbolic matrices
	std::vector<std::vector<RCP<const Basic>>> getCZ() const { return CZ; }
	std::vector<std::vector<RCP<const Basic>>> getCP() const { return CP; }

	//const Complex& getSymbolS() const { return s; }

	Conductor* getConductor(const std::string& key) {
		auto it = conductors.find(key);
		if (it != conductors.end()) {
			return &(it->second); // Return a pointer to the conductor if found
		}
		else {
			return nullptr; // Return nullptr if conductor not found
		}
	}

	// Function to access an insulator from the insulators map
	Insulator* getInsulator(const std::string& key) {
		auto it = insulators.find(key);
		if (it != insulators.end()) {
			return &(it->second);
		}
		return nullptr; // Insulator not found
	}


	/*std::tuple<int, int, int> getEarthParameters() const {
		return earth_parameters;
	}

	std::string getType() const {return type;}
	void setType(const std::string& newType) {type = newType;}
	}*/

	// Function to modify a conductor in the conductors map
	void updateConductor(const std::string& key, const Conductor& conductor) {
		conductors[key] = conductor;
	}

	// Function to remove a conductor from the conductors map
	void removeConductor(const std::string& key) {
		conductors.erase(key);
	}

	// Define a member function in the Cable class to access the conductors map
	const std::unordered_map<std::string, Conductor>& getCableConductors() const {
		return conductors;
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

	//void setCZ(const std::vector<std::vector<RCP<const Basic>>>& cz);
	//void setCP(const std::vector<std::vector<RCP<const Basic>>>& cp);

	void setCZ(const std::vector<std::vector<RCP<const Basic>>>& cz) {
		CZ = cz;
	}

	void setCP(const std::vector<std::vector<RCP<const Basic>>>& cp) {
		CP = cp;
	}


	//friend void cable(Cable& c, const std::vector<std::vector<double>>& P, const std::vector<std::vector<double>>& Z, const std::unordered_map<std::string, std::vector<std::pair<double, double>>>& kwargs, bool transformation);
	friend void cable(Cable& c, const std::vector<std::vector<double>>& P, const std::vector<std::vector<double>>& Z, const std::unordered_map<std::string, std::vector<std::pair<double, double>>>& kwargs, bool transformation);

	// Destructor
	~Cable();

private:
	// Private member variables
	double length;   //line length [m]
	//dictionary with a particular order. Key: Symbol-> C1, C2, C3 and C4. Value: Conductor-> Mutable Struct Conductor, defined above
	std::unordered_map<std::string, Conductor> conductors = { {"Symbol", Conductor(/* Constructor arguments for Conductor*/)},
															  {"Conductor", Conductor(/* Constructor arguments for Conductor*/)} };
	//dictionary with a particular order. Key: Symbol-> I1, I2, I3 and I4. Value: Insulator-> Mutable Struct Insulator, defined above
	std::unordered_map<std::string, Insulator> insulators = { {"Symbol", Insulator(/* Constructor arguments for Conductor*/)},
															  {"Conductor", Insulator(/* Constructor arguments for Conductor*/)} };
	//indicates all variables are real number, vector composed by tuple of real numbers. e.g. positions=[(0,0),(1,1)]. Cables positions 1st:x=0, y=0. 2nd: x=1, y=1.
	std::vector<std::pair<double, double>> positions;
	//(μᵣ, ϵᵣ, ρ) in units ([], [], [Ωm]) compact way of representing the type for a tuple of length N where all elements are of type Int or Float64.
	std::tuple<double, double, double> earth_parameters;

	std::string configuration;//Configuration is a datatype symbol with value coaxial Symbol -> Type of data. Symbols can be entered using the quote operator ":"
	std::string type;
	bool eliminate = true;
	//std::string transformation;
	bool transformation;

	// Define variables for real and imaginary parts
	// Define variables for real and imaginary parts
	SymEngine::rational_class real_part;
	SymEngine::rational_class imag_part;

	//SymEngine::Complex s; // Assuming Complex is a member of Cable class
	const SymEngine::Complex* s; // Store pointer to SymEngine::Complex

	std::vector<std::vector<RCP<const Basic>>> CZ;
	std::vector<std::vector<RCP<const Basic>>> CP;
};

#endif

