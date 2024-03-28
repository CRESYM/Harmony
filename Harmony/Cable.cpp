#include "cable.h"
#include "constants.h"
#include "symbol.h"
#include "element.h"
#include <cmath> // Include cmath for mathematical functions like log, sqrt, etc.
#include <algorithm>
#include <stdexcept>

// Constructor
Cable::Cable():transformation(false) {} // Initialize transformation to false by default

// Destructor
Cable::~Cable() {}

std::string convertToString(const std::vector<std::pair<double, double>>& value) {
	std::ostringstream oss;
	for (const auto& pair : value) {
		oss << "(" << pair.first << "," << pair.second << ") ";
	}
	return oss.str();
}

// Function to set a property with the given key and value
void Cable::setProperty(const std::string& key, const std::string& value) {
// handle setting properties based on the key and value here
// For example, you might want to set different properties based on the key

	if (key == "length") {
		// Convert the value string to double and set the length
		length = std::stod(value);
	}
	else if (key == "configuration") {
		// Set the configuration
		configuration = value;
	}
	else if (key == "type") {
		// Set the type
		type = value;
	}
	else if (key == "transformation") {
		// Set the transformation
		transformation = value;
	}
	else {
		// Handle other properties as needed
		// For example:
		throw std::invalid_argument("Unknown property name.");
	}

}

// Function to handle properties
void handleProperty(Cable& c, const std::string& key, const std::vector<std::pair<double, double>>& value) {
	if (key == "positions") {
		// Handle positions property
		// No need to convert, directly use the provided value
		for (const auto& pos : value) {
			c.addPosition(pos.first, pos.second);
		}
	}
	else if (c.isConductor(key)) {
		// Handle conductor property
		// Assuming addConductor accepts a single Conductor object
		Conductor conductor; // Create a Conductor object
		// Populate the conductor object using the provided value
		// For example:
		// conductor = createConductorFromVector(value); // Convert vector to Conductor
		c.addConductor(key, conductor); // Add the conductor to the Cable object
	}
	else if (c.isInsulator(key)) {
		// Handle insulator property
		// Assuming addInsulator accepts a single Insulator object
		Insulator insulator; // Convert vector to Insulator
		c.addInsulator(key, insulator);
	}
	else if (key == "transformation") {
		// Handle transformation property
		std::string transformationStr = convertToString(value);
		c.setTransformation(transformationStr);
	}
	else {
		throw std::invalid_argument("Unknown cable property name.");
	}
}

/*Symbol symbols(const std::string& name) {
	return Symbol(name);
}*/
// Create a symbol table and add symbol-value pairs


void cable(Cable& c, const std::vector<std::vector<double>>& P, const std::vector<std::vector<double>>& Z, const std::unordered_map<std::string, std::vector<std::pair<double, double>>>& kwargs, bool transformation) {
	// Access Z[i][i] from the Cable object c
	int i = 3;
	double Z_ii = c.Z[i][i]; //impedance
	double P_ii = c.P[i][i]; // shunt admittance

	// Iterate through kwargs
	for (const auto& pair : kwargs) {
		const std::string& key = pair.first;
		const std::vector<std::pair<double, double>>& value = pair.second;

		// Check if property exists and handle it accordingly
		handleProperty(c, key, value);

		// conversion procedure #not FP
		//core outer radius    #not FP
		// Perform additional transformations based on specific keys 
		// checks if the equivalent area of the conductor C1 is not zero. If it's not zero, it performs a calculation involving the property ρ and updates its value accordingly. If the area is zero, it does nothing.
		if (key == "C1") {
			// Access and modify the conductor from the Cable object
			Conductor* conductorPtr = c.getConductor("C1");
			if (conductorPtr != nullptr) {
				double area = conductorPtr->getArea();
				if (area != 0) {
					conductorPtr->setResistivity(conductorPtr->getResistivity() * M_PI * conductorPtr->getOuterRadius() * conductorPtr->getOuterRadius() / area);

					// Update the conductor in the Cable object
					c.updateConductor("C1", *conductorPtr);
				}
			}
		}
		// add metalic screen conversions, equivalent sheat layer
		else if (key == "SC") {
			// Conversion procedure for metalic screen (SC) and equivalent sheath layer (C2)
			// add metalic screen conversions, equivalent sheat layer
			// Check if SC conductor exists
			Conductor* conductorSC = c.getConductor("SC");
			if (conductorSC != nullptr) {
				// Conductor found, perform actions
			}
			else {
				// Conductor not found, handle accordingly
			}
			// Assuming nullptr is returned when conductor is not found
			bool conductorExists = c.getConductor("SC") != nullptr;
			if (conductorExists) {
				// Check if C2 conductor is present
				if (c.getConductor("C2") == nullptr) { // Assuming nullptr is returned when conductor is not found
					throw std::invalid_argument("There must be present sheath together with screen layer.");
				}

				// If the equivalent area of the semiconductor is defined and different from zero
				Conductor* conductorSC = c.getConductor("SC");
				if (conductorSC->area != 0) {
					// Calculate internal radius of conductor C2
					c.getConductor("C2")->ri = sqrt(conductorSC->ro * conductorSC->ro - conductorSC->area / M_PI);
				}
				else {
					// If the screen equivalent area is not defined, set the internal radius of conductor C2 equal to the internal screen radius
					c.getConductor("C2")->ri = conductorSC->ri;
				}

			}

			// Calculate outer radius of conductor C2
			double c2OuterRadius = sqrt((c.getConductor("C2")->ro * c.getConductor("C2")->ro - conductorSC->ro * conductorSC->ro) *
				conductorSC->resistivity / c.getConductor("C2")->resistivity + conductorSC->ro * conductorSC->ro);
			c.getConductor("C2")->ro = c2OuterRadius;

			// Remove the SC conductor
			c.removeConductor("SC");

			// Change Insulator 1
			c.getInsulator("I1")->ro = c.getConductor("C2")->ri;

			// Change Insulator 2 if present
			if (c.getInsulator("I2") != nullptr) {
				double x = log(c.getInsulator("I2")->ro / c.getConductor("C2")->ro) / log(c.getInsulator("I2")->ro / c.getInsulator("I2")->ri);
				c.getInsulator("I2")->ri = c.getConductor("C2")->ro;
				c.getInsulator("I2")->permittivity *= x; // Using 'permittivity' for dielectric constant
				c.getInsulator("I2")->permeability /= x; // Assuming 'permeability' is the member representing permeability
			}
		}
		// Semiconductor configuration
		if (c.getInsulator("I1") && c.getInsulator("I1")->ro != 0) {
			double x = log(c.getInsulator("I1")->ro / c.getInsulator("I1")->ri) / log(c.getInsulator("I1")->b / c.getInsulator("I1")->a);
			c.getInsulator("I1")->permittivity *= x;
			double N = 1.4;
			c.getInsulator("I1")->permeability *= (1 + 2 * M_PI * M_PI * N * N * (c.getInsulator("I1")->ro * c.getInsulator("I1")->ro - c.getInsulator("I1")->ri * c.getInsulator("I1")->ri) / log(c.getInsulator("I1")->ro / c.getInsulator("I1")->ri));
		}
	}
	// Ground parameters
	double mu_0 = 4 * M_PI * 1e-7; // Vacuum permeability
	double epsilon_0 = 8.85e-12;    // Vacuum permittivity
	auto earthParams = c.getEarthParameters();
	double mu_g = std::get<0>(earthParams) * mu_0; // Ground permittivity
	double epsilon_g = std::get<1>(earthParams) * epsilon_0; // Ground permeability
	double sigma_g = 1 / std::get<2>(earthParams); // Ground conductivity
	double rho_g = 1 / std::get<2>(earthParams);
	double gamma = 0.5772156649;
	double g = 1e-11;
	//Symbol s = symbols("s"); // Definition of the variable S as a symbol
	SymbolTable symbolTable;
	symbolTable.addSymbol("s", 10.0); // Add symbol "s" with a numeric value of 10.0

	// Retrieve the numeric value associated with the symbol "s"
	double s_value = symbolTable.getNumericValue("s");

	// Define variables for frequency domain implementation
	auto& conductors = c.getCableConductors();
	auto& insulators = c.getCableInsulators();
	auto& positions = c.getPositions();

	double d_ij = 0; // Initialize d_ij
	//int n = 3; // ested for cycle with a short notation. assuming 3 conductors: n=3 -> (k,l)= (1,1) -> (1,2) -> (1,3) -> (2,1) -> (2,2) -> (2,3) -> (3,1) -> (3,2) -> (3,3)
	//std::vector<std::vector<double>> P(n, std::vector<double>(n, 0.0)); // Initialize P matrix with zeros



	// Define variables for frequency domain implementation
	size_t n_l = conductors.size(); // Number of cable layers
	size_t n = positions.size();    // Number of cables
	//std::vector<std::vector<double>> Z(n * n_l, std::vector<double>(n * n_l, 0.0)); // Impedance matrix		std::vector<std::vector<double>> P(n * n_l, std::vector<double>(n * n_l, 0.0)); // P matrix
	
	// make series impedance #pag24 simulator_tutorial
	// between rows 153 and 181 -> CONDUCTORS Case (no insulators) -> i need as external indicator. It is needed to compute the Z matrix position until key is inside the number of keys present in conductors
	//size_t i = 0; // External indicator
	
	for (const auto& pair : conductors) {
		const auto& conductor = pair.second;
		double r_i = conductor.ri;
		double r_o = conductor.ro;
		double mu = conductor.permeability * mu_0;
		double rho = conductor.resistivity;

		// Retrieve the numeric value associated with the symbol "s"
		double m = sqrt(s_value * mu / rho);


		double Δr = r_o - r_i;

		double Z_aa, Z_bb, Z_ab;
		if (r_i != 0) {
			Z_aa = rho * m / (2 * M_PI * r_i) * std::tanh(m * Δr) - rho / (2 * M_PI * r_i * (r_i + r_o));
			Z_bb = rho * m / (2 * M_PI * r_o) * std::tanh(m * Δr) + rho / (2 * M_PI * r_o * (r_i + r_o));
		}
		else {
			Z_bb = rho * m / (2 * M_PI * r_o) * std::tanh(0.733 * m * r_o) + 0.3179 * rho / (M_PI * r_o * r_o);
		}
		Z_ab = rho * m / (M_PI * (r_o + r_i)) * (1.0 / sinh(m * Δr));

		c.Z[i][i] += Z_bb;
		if (i > 0) {
			c.Z[i][i - 1] += -Z_ab;
			c.Z[i - 1][i] += -Z_ab;
			c.Z[i - 1][i - 1] += Z_aa;
		}
		if (i == n_l - 1) {

			double m = sqrt(s_value * mu_g / rho_g); // same as row 156, this time with ground permeability μᵍ and resistivity ρᵍ
			double H = 2 * positions[0].second;
			double d_ij = 0; // Calculate dᵢⱼ from both conductors and insulators

			// Iterate over conductors to find maximum radius
			for (const auto& conductor : conductors) {
				d_ij = std::max(d_ij, conductor.second.ro);
			}

			// Iterate over insulators to find maximum radius
			for (const auto& insulator : insulators) {
				d_ij = std::max(d_ij, insulator.second.ro);
			}
			
			double x = d_ij;
			double Z_g = s_value * mu_g / (2 * M_PI) * (-log(gamma * m * d_ij / 2) + 0.5 - 2 * m * H / 3);
			c.Z[i][i] += Z_g;
		}
		i++;
	}

	// make shunt admittance -> Insulator
	// Define variables for shunt admittance calculation
	//int i = 0; // Re-assign the value 1 to i
	for (const auto& insulatorPair : c.getCableInsulators()) {
		const Insulator& insulator = insulatorPair.second;
		double r_i = insulator.ri;
		double r_o = insulator.ro;
		double mu = insulator.permeability * mu_0;
		double epsilon = insulator.permittivity * epsilon_0;
		// Assuming P is declared as a 2D vector

		double Z_i = s_value * mu / (2 * M_PI) * log(r_o / r_i); // Insulator layer impedance
		double P_i = log(r_o / r_i) / (2 * M_PI * epsilon); // P expression

		c.Z[i][i] += Z_i; // For the impedance matrix diagonal values, add also the insulator impedance
		// Update the elements of P matrix for the current insulator
		for (int j = 0; j <= i; j++) {
			c.P[j][i] += P_i;
			c.P[i][j] += P_i;
		}

		// Update the elements of P matrix in the submatrix P[1:i,1:i]
		for (int j = 0; j < i; j++) {
			for (int k = 0; k < i; k++) {
				c.P[j][k] += P_i;
			}
		}
	}

	for (int i = 1; i <= n; ++i) { // n = number of cables
		for (int row = (i - 1) * n_l; row < i * n_l; ++row) {
			for (int col = (i - 1) * n_l; col < i * n_l; ++col) {
				c.Z[row][col] = Z[row % n_l][col % n_l]; // Copy and translate the same conductor impedance matrix
				c.P[row][col] = P[row % n_l][col % n_l]; // Same for the P matrix
			}
		}

		for (int j = i + 1; j <= n; ++j) { // Add earth return impedance and mutual impedance between cables
			double m = sqrt(s_value * mu_g / rho_g);
			double H = positions[i - 1].second + positions[j - 1].second; // H = sum of depth of ith and jth cables
			double d_ij = sqrt(pow(positions[i - 1].first - positions[j - 1].first, 2) + pow(positions[i - 1].second - positions[j - 1].second, 2));
			double x = abs(positions[i - 1].first - positions[j - 1].first);

			double Z_g = s_value * mu_g / (2 * M_PI) * (-log(gamma * m * d_ij / 2) + 0.5 - 2 * m * H / 3);
			c.Z[i * n_l][j * n_l] += Z_g; // Mutual impedance between cables
			c.Z[j * n_l][i * n_l] += Z_g; // Same as the row above, but in the mutual position
		}
	}

	// reduction for represention core, sheath and armor
	for (int k = 0; k < n; ++k) {
		for (int l = 0; l < n; ++l) {
			for (int i = n_l - 2; i >= 0; --i) {
				for (int j = 0; j <= i; ++j) {
					// Update Z matrix
					for (int p = 0; p < n_l; ++p) {
						c.Z[l * n_l + p][(k * n_l) + j] += Z[l * n_l + p][(k * n_l) + i + 1];
					}
				}
			}

			for (int i = n_l - 2; i >= 0; --i) {
				for (int j = 0; j <= i; ++j) {
					// Update Z matrix
					for (int p = 0; p < n_l; ++p) {
						c.Z[(k * n_l) + j][l * n_l + p] += Z[(k * n_l) + i + 1][l * n_l + p];
					}
				}
			}
		}
	}

	if (c.getType == "underground") {
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < n; ++j) {
				double H = positions[i - 1].second + positions[j - 1].second; // H = sum of depth of ith and jth cables
				double x = abs(positions[i - 1].first - positions[j - 1].first);
				double y = abs(positions[i - 1].second - positions[j - 1].second);
				double D1, D2;
				if (i == j) {
					double max_conductor_radius = 0;
					for (const auto& conductor : conductors) {
						max_conductor_radius = std::max(max_conductor_radius, conductor.second.ro);
					}
					double max_insulator_radius = 0;
					for (const auto& insulator : insulators) {
						max_insulator_radius = std::max(max_insulator_radius, insulator.second.ro);
					}
					D1 = std::max(max_conductor_radius, max_insulator_radius);
					D2 = H;
				}
				else {
					D1 = sqrt(x * x + y * y);
					D2 = sqrt(x * x + H * H);
				}
				double P_ij = log(D2 / D1) / (2 * M_PI * epsilon_0);
				for (int k = i * n_l; k < (i + 1) * n_l; ++k) {
					for (int l = j * n_l; l < (j + 1) * n_l; ++l) {
						c.P[k][l] += P_ij;
					}
				}
			}
		}
	}

	// Assuming c.P and c.Z are matrices in C++
	// Copy P and Z matrices to c.P and c.Z
	c.setP(P); // Assign P matrix to c.P
	c.setZ(Z); // Assign Z matrix to c.Z

	// Create an Element object with input_pins = n, output_pins = n,
	// element_value = c, and transformation = transformation
	// Assuming `n` and `transformation` are defined appropriately

// Create an unordered_map to hold the constructor arguments
	std::unordered_map<std::string, std::string> args;
	args["input_pins"] = std::to_string(n);
	args["output_pins"] = std::to_string(n);
	args["transformation"] = transformation ? "true" : "false";

	// Assuming `c` is an instance of the `Cable` class and `elem` is an instance of the `Element` class
	Element elem(args);
	//elem.element_value = c; // Assuming `element_value` is of type `Cable` or a suitable type

}
