#ifndef _CABLE_H_
#define _CABLE_H_

/**
 * @file Cable.h
 * @brief Underground or aerial multi-conductor cable with layered geometry.
 */

#include "../Element.h"

class Element; // Forward declaration of Element class

/**
 * @class Cable
 * @brief Multi-layer cable model with conductor and insulator sections.
 * @ingroup transmission
 */
class Cable : public Element {
public:

	/**
	 * @class Conductor
	 * @brief Conducting layer with inner/outer radius and material properties.
	 */
	class Conductor {
	public:
		/**
		 * @brief Construct a cable conductor layer.
		 * @param ri Inner radius (m).
		 * @param ro Outer radius (m).
		 * @param resistivity Resistivity rho (Ω·m).
		 * @param permeability Relative permeability mu_r.
		 * @param area Nominal cross-sectional area (m²).
		 */
		Conductor(double ri = 0, double ro = 0, double resistivity = 0, double permeability = 1, double area = 0)
			: ri(ri), ro(ro), resistivity(resistivity), permeability(permeability), area(area) {}

		// Member variables
		double ri; //conductor inner radius
		double ro; //conductor outer radius
		double resistivity; //conductor resistivity ρ [Ωm]
		double permeability;  //relative permeability μᵣ
		double area; //nominal area
	};

	/**
	 * @class Insulator
	 * @brief Insulating layer with optional inner/outer semiconductor radii.
	 */
	class Insulator {
	public:
		/**
		 * @brief Construct a cable insulator layer.
		 * @param ri Inner radius (m).
		 * @param ro Outer radius (m).
		 * @param permittivity Relative permittivity epsilon_r.
		 * @param permeability Relative permeability mu_r.
		 * @param innerSemiConductorOuterRadius Inner semiconductor outer radius a (m).
		 * @param outerSemiConductorInnerRadius Outer semiconductor inner radius b (m).
		 */
		Insulator(double ri = 0, double ro = 0, double permittivity = 1, double permeability = 1, double innerSemiConductorOuterRadius = 0, double outerSemiConductorInnerRadius = 0)
			: ri(ri), ro(ro), permittivity(permittivity), permeability(permeability), a(innerSemiConductorOuterRadius), b(outerSemiConductorInnerRadius) {}

		// Member variables
		double ri;  //insulator inner radius
		double ro;  //insulator outer radius
		double permittivity; //relative permittivity epsilon_r
		double permeability; //relative permeability mu_r

		//If a semiconductor is present in an insulator, we have: r_i < semiconductor < a + a < insulator < b + b < semiconductor < r_o
		double a; //inner semiconductor outer radius -> Inner semiconductor r_i < r < a
		double b; //outer semiconductor inner radius -> Outer semiconductor b < r < r_o
	};

	// Constructor simple
	/**
	 * @brief Default cable with placeholder symbol, location, and pin counts.
	 */
	Cable() : Element("cable", "DC1", 1, 1), length(0), type("underground"), eliminate(true) {};
	
	/**
	 * @brief Construct a cable from geometry, earth, conductor, and insulator maps.
	 * @param symbol Element identifier.
	 * @param location Network area or location string.
	 * @param pins Number of pins (phases).
	 * @param type_constructor Cable type ("underground" or "aerial").
	 * @param length_constructor Line length (m).
	 * @param earth Earth parameters tuple (mu_r, epsilon_r, resistivity).
	 * @param conductors_constructor Map of conductor symbols to Conductor layers.
	 * @param insulators_constructor Map of insulator symbols to Insulator layers.
	 * @param positions_constructor (x, y) positions of each cable in the layout.
	 */
	Cable(const string& symbol, const std::string& location, int pins, const string& type_constructor,
		double length_constructor, std::tuple<double, double, double> earth,
		std::map<string, Conductor*> conductors_constructor, std::map<string, Insulator*> insulators_constructor,
		std::vector<std::pair<double, double>> positions_constructor);

	void setLength(double newLength) { length = newLength; }
	void addConductor(const std::string& key, Conductor* conductor) { conductors[key] = conductor; }
	void addInsulator(const std::string& key, Insulator* insulator) { insulators[key] = insulator; }
	void addPosition(double x, double y) { positions.emplace_back(x, y); }
	void setEarthParameters(double mu, double epsilon, double rho) { earth_parameters = std::make_tuple(mu, epsilon, rho); }
	void setConfiguration(const std::string& newConfig) { configuration = newConfig; }
	void setEliminate(bool value) { eliminate = value; } //to change the value of eliminate


	// Getter methods
	double getLength() const { return length; }
	const std::tuple<double, double, double>& getEarthParameters() const { return earth_parameters; }
	const std::vector<std::pair<double, double>>& getPositions() const { return positions; }
	const std::string& getConfiguration() const { return configuration; }
	const std::string& getType() const { return type; }
	bool getEliminate() const { return eliminate; }

	Conductor* getConductor(const std::string& key) {
		auto it = conductors.find(key);
		if (it != conductors.end()) {
			return (it->second); // Return a pointer to the conductor if found
		}
		else {
			return nullptr; // Return nullptr if conductor not found
		}
	}

	// Function to access an insulator from the insulators map
	Insulator* getInsulator(const std::string& key) {
		auto it = insulators.find(key);
		if (it != insulators.end()) {
			return (it->second);
		}
		return nullptr; // Insulator not found
	}

	void updateInsulator(const std::string& key, Insulator* insulator) {
		insulators[key] = insulator;
	}

	void updateLayers(); // Function to update layers

	// Function to modify a conductor in the conductors map
	void updateConductor(const std::string& key, Conductor* conductor) {
		conductors[key] = conductor;
	}

	// Function to remove a conductor from the conductors map
	void removeConductor(const std::string& key) {
		auto it = conductors.find(key);
		if (it != conductors.end()) {
			delete it->second;
			conductors.erase(it);
		}
	}

	// Define a member function in the Cable class to access the conductors map
	const std::map<std::string, Conductor*>& getCableConductors() const {
		return conductors;
	}

	// Define a member function in the Cable class to access the insulators map
	const std::map<std::string, Insulator*>& getCableInsulators() const {
		return insulators;
	}

	// Functions for handling properties
	bool hasProperty(const std::string& key) { return (conductors.find(key) != conductors.end() || insulators.find(key) != insulators.end()); }
	bool isConductor(const std::string& key) { return (conductors.find(key) != conductors.end()); }
	bool isInsulator(const std::string& key) { return (insulators.find(key) != insulators.end()); }


	// Function to compute Y parameters
	virtual std::vector<std::vector<complex<double>>> compute_y_parameters(double frequency) override;

	virtual void printElementValues() override;

	// Destructor
	~Cable();

private:
	// Private member variables
	double length;   //line length [m]
	//dictionary with a particular order. Key: Symbol-> C1, C2, C3 and C4. Value: Conductor-> Mutable Struct Conductor, defined above
	// entries are "Symbol" of the conductor and pointer to the conductor	
	std::map<std::string, Conductor*> conductors;
	//dictionary with a particular order. Key: Symbol-> I1, I2, I3 and I4. Value: Insulator-> Mutable Struct Insulator, defined above
	// entries are "Symbol" of the insulator and pointer to the insulator
	std::map<std::string, Insulator*> insulators;
	//indicates all variables are real number, vector composed by tuple of real numbers. e.g. positions=[(0,0),(1,1)]. Cables positions 1st:x=0, y=0. 2nd: x=1, y=1.
	std::vector<std::pair<double, double>> positions;
	//(μᵣ, ϵᵣ, ρ) in units ([], [], [Ωm]) compact way of representing the type for a tuple of length N where all elements are of type Int or Float64.
	std::tuple<double, double, double> earth_parameters;

	std::string configuration = "coaxial"; // Configuration is a datatype symbol with value "coaxial". 
	std::string type; // Type is a datatype symbol with value "underground" or "aerial". 
	bool eliminate = true;

	DenseMatrix Z;
	DenseMatrix Y;
	MatrixXd P; //DenseMatrix P;
};

#endif