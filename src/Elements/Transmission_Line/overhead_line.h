#ifndef OVERHEAD_LINE_H
#define OVERHEAD_LINE_H

/**
 * @file overhead_line.h
 * @brief Overhead transmission line with conductor bundle and ground-wire geometry.
 */

#include "Transmissionline.h"
#include "../Element.h"

class Element; // Forward declaration of Element class

/**
 * @class Overhead_Line
 * @brief Overhead line model with bundle geometry and frequency-dependent Y parameters.
 * @ingroup transmission
 */
class Overhead_Line : public Element {
private:
	/**
	 * @class Conductors
	 * @brief Phase conductor bundle geometry and electrical properties.
	 */
	class Conductors {
	public:
		int number_bundles = 1; //number of bundles (phases) nᵇ
		int number_conductors_bundle = 1; //number of subconductors per bundle nˢᵇ
		double ybc = 0; //height above the ground of the lowest bundle  [m]  yᵇᶜ
		double deltaYbc = 0; //vertical offset between the bundles   [m] Δyᵇᶜ
		double deltaXbc = 0; //horizontal offset between the lowest bundles  [m] Δxᵇᶜ
		double deltaTildeXbc = 0; //horizontal offset in group of bundles    [m] Δ̃xᵇᶜ
		// used for concentric and offset organization only
		double dsag = 0; //sag offset    [m] 
		double dsb = 0; // subconductor spacing (symmetric)  [m] 
		double rc = 0; //conductor radius  [m] 
		double Rdc = 0; //DC resistance for the entire conductor [Ω/m] 
		double gc = 1e-11; // shunt conductance  
		double mu_rc = 1; // relative conductor permeability μᵣᶜ
		std::tuple<std::vector<double>, std::vector<double>> positions = { {},{} }; //add absolute positions manually, 
		// then organization must be set to "absolute"
		std::string organization; // Symbol for organization

		// Constructors
		Conductors() {};
		/**
		 * @brief Construct a conductor bundle with organization and geometry parameters.
		 * @param organization Bundle arrangement symbol (flat, vertical, delta, etc.).
		 * @param nb Vector of bundle counts per phase group.
		 * @param geo Geometry parameter vector (spacing, sag, radius, etc.).
		 * @param ybc Height of the lowest bundle above ground (m).
		 * @param dybc Vertical offset between bundles (m).
		 * @param dxbc Horizontal offset between lowest bundles (m).
		 * @param dtxbc Horizontal offset in bundle group (m).
		 * @param positions Optional absolute (x, y) positions per subconductor.
		 */
		Conductors(std::string, std::vector<int>&, std::vector<double>&, double, double, double, double, std::tuple<std::vector<double>, std::vector<double>>); // Default constructor for organization

		// Functions for bundle positions
		void estimate_flat();
		void estimate_vertical();
		void estimate_delta();
		void estimate_concentric();
		void estimate_offset();

		std::tuple<std::vector<double>, std::vector<double>> bundle_position();
	};

	/**
	 * @class Groundwires
	 * @brief Overhead ground wire geometry and electrical properties.
	 */
	class Groundwires {
	public:
		int ng = 0; // number of groundwires (typically 0 or 2) 
		double deltaXg = 0; // horizontal offset between groundwires [m] Δxg
		double deltaYg = 0; // vertical offset between the lowest conductor and groundwires  [m] Δyg
		double rg = 0; // ground wire radius  [m] rg
		double dgsag = 0; // sag offset [m] dgsag
		double Rgdc = 0; // groundwire DC resistance [Ω/m] Rgdc
		double mu_g = 1; // relative groundwire permeability μᵣ
		std::tuple<std::vector<double>, std::vector<double>> positions; // add absolute positions manually

		/**
		 * @brief Construct ground wire geometry from parameter tuples.
		 * @param ng Number of ground wires (typically 0 or 2).
		 * @param geo Geometry parameter vector (offsets, radius, sag, resistance).
		 * @param dxg Horizontal offset between ground wires (m).
		 * @param dyg Vertical offset from lowest conductor (m).
		 * @param positions Optional absolute (x, y) positions per ground wire.
		 */
		Groundwires(int, std::vector<double>&, double, double, std::tuple<std::vector<double>, std::vector<double>>);
	};

	double length = 0;  // line length [km]
	Conductors* conductors = nullptr;
	Groundwires* groundwires = nullptr;

	// Earth parameters are defined as (mu_r, epsilon_r, resistivity)
	std::tuple<double, double, double> earthParameters = std::make_tuple(1, 1, 1); // (μᵣ_earth, ϵᵣ_earth, ρ_earth) in units ([], [], [Ωm])
	MatrixXd P;
	DenseMatrix Y;
	DenseMatrix Z;

	bool eliminate = true;

	// Variables to store organization x and y values
	std::vector<double> organization_x_values;
	std::vector<double> organization_y_values;
public:
	/**
	 * @brief Construct an overhead line with earth, conductor, and ground-wire data.
	 * @param symbol Element identifier.
	 * @param location Network area or location string.
	 * @param length Line length (km).
	 * @param earth Earth parameters tuple (mu_r, epsilon_r, resistivity).
	 * @param conductor Conductor bundle tuple (organization, counts, geometry, offsets).
	 * @param groundwire Ground wire tuple (count, geometry, horizontal/vertical offsets).
	 */
	Overhead_Line(const std::string& symbol, const std::string& location, double length, std::tuple<double, double, double> earth,
		std::tuple<std::string, std::vector<int>, std::vector<double>, double, double, double, double> conductor,
		std::tuple<int, std::vector<double>, double> groundwire);

	~Overhead_Line() override;

	// Function to compute Y parameters
	virtual vector<vector<complex<double>>> compute_y_parameters(double omega_num) override;
};

#endif // OVERHEAD_LINE_H

