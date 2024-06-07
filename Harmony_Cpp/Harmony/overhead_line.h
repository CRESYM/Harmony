#ifndef OVERHEAD_LINE_H
#define OVERHEAD_LINE_H

#include "Element.h"
#include "Transmissionline.h"
//#include "symbol.h" // Include Symbol class declaration
//#include "basic.h" //Include basic.h instead of redefining Basic struct 

#include <vector>
#include <tuple>
#include <variant>
#include <string>
#include <symbol.h>
#include <basic.h>

class Conductors {
public:
	int nb = 1; //number of bundles (phases) nᵇ
	int nsb = 1; //number of subconductors per bundle nˢᵇ
	double ybc = 0; //height above the ground of the lowest bundle  [m]  yᵇᶜ
	double deltaYbc = 0; //vertical offset between the bundles   [m] Δyᵇᶜ
	double deltaXbc = 0; //horizontal offset between the lowest bundles  [m] Δxᵇᶜ
	double deltaTildeXbc = 0; //horizontal offset in group of bundles    [m] Δ̃xᵇᶜ
	double dsag = 0; //sag offset    [m] dˢᵃᵍ
	double dsb = 0; //subconductor spacing (symmetric)  [m] dˢᵇ
	double rc = 0; //conductor radius  [m] rᶜ
	double RdC = 0; //DC resistance for the entire conductor [Ω/m] Rᵈᶜ
	double gc = 1e-11; // shunt conductance gᶜ 
	double murc = 1; //elative conductor permeability μᵣᶜ
	std::tuple<std::vector<double>, std::vector<double>> positions = { {},{} }; //add absolute positions manually
	SymEngine::Symbol organization; // Symbol for organization
	Conductors() : organization("") {} // Default constructor for organization
};

class Groundwires {
public:
	int ng = 0; //number of groundwires (typically 0 or 2) nᵍ
	double deltaXg = 0; //horizontal offset between groundwires [m] Δxᵍ
	double deltaYg = 0; //vertical offset between the lowest conductor and groundwires  [m] Δyᵍ
	double rg = 0; //ground wire radius  [m] rᵍ
	double dgSag = 0; //sag offset [m] dᵍˢᵃᵍ
	double RdG = 0; //groundwire DC resistance [Ω/m] Rᵍᵈᶜ
	double mug = 1; //relative groundwire permeability μᵣᵍ
	std::tuple<std::vector<double>, std::vector<double>> positions = { {},{} }; //add absolute positions manually
};

class overhead_Line : public TransmissionLine {
public:
	double length = 0;  // line length [km]
	Conductors conductors;
	Groundwires groundwires;
	std::tuple<int, int, int> earthParameters = std::make_tuple(1, 1, 1); // (μᵣ_earth, ϵᵣ_earth, ρ_earth) in units ([], [], [Ωm])
	//std::vector<std::vector<SymEngine::Basic>> P; // Example initialization
	//std::vector<std::vector<SymEngine::Basic>> Z; // Example initialization
	std::vector<std::vector<std::shared_ptr<SymEngine::Basic>>> P;
	std::vector<std::vector<std::shared_ptr<SymEngine::Basic>>> Z;

	bool eliminate = true;

	// Variables to store organization x and y values
	std::vector<double> organization_x_values;
	std::vector<double> organization_y_values;

	// Static member functions for estimating conductor configurations
	static std::tuple<std::vector<double>, std::vector<double>> estimate_flat(const Conductors& c);
	static std::tuple<std::vector<double>, std::vector<double>> estimate_vertical(const Conductors& c);
	static std::tuple<std::vector<double>, std::vector<double>> estimate_delta(const Conductors& c);
	static std::tuple<std::vector<double>, std::vector<double>> estimate_concentric(const Conductors& c);
	static std::tuple<std::vector<double>, std::vector<double>> estimate_offset(const Conductors& c);
	static std::tuple<std::vector<double>, std::vector<double>> bundle_position(int n, double d);

	//	void overhead_line(overhead_Line& tl, std::unordered_map<std::string, std::variant<int, double, std::tuple<std::vector<double>, std::vector<double>>, Symbol>>& kwargs);

/*		// Function to set P matrix
	void setP(const std::vector<std::vector<SymEngine::Basic>>& newP) {
		P = newP;
	}

	// Function to set Z matrix
	void setZ(const std::vector<std::vector<SymEngine::Basic>>& newZ) {
		Z = newZ;
	}*/

	// Function to set P matrix
	void setP(const std::vector<std::vector<std::shared_ptr<SymEngine::Basic>>>& newP) {
		P.clear(); // Clear the current vector

		// Copy each element from newP to P
		for (const auto& row : newP) {
			std::vector<std::shared_ptr<SymEngine::Basic>> newRow;
			for (const auto& elem : row) {
				// Create a new shared pointer to SymEngine::Basic and add it to newRow
				newRow.push_back(elem);
			}
			// Add the newRow to P
			P.push_back(newRow);
		}
	}

	// Function to set Z matrix
	void setZ(const std::vector<std::vector<std::shared_ptr<SymEngine::Basic>>>& newZ) {
		Z.clear(); // Clear the current vector

		// Copy each element from newZ to Z
		for (const auto& row : newZ) {
			std::vector<std::shared_ptr<SymEngine::Basic>> newRow;
			for (const auto& elem : row) {
				// Create a new shared pointer to SymEngine::Basic and add it to newRow
				newRow.push_back(elem);
			}
			// Add the newRow to Z
			Z.push_back(newRow);
		}
	}
};

#endif // OVERHEAD_LINE_H

