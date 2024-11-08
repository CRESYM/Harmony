#include "cable.h"


//using PropertyValue = boost::variant<std::vector<std::pair<double, double>>, std::string>; // Define PropertyValue type


// Define a union type to hold different property values
union PropertyValue {
	std::vector<std::pair<double, double>> positions;
	Conductor conductor;
	Insulator insulator;
	std::string transformationStr;

	// Constructor and destructor to manage memory properly
	PropertyValue() {}
	~PropertyValue() {}
};

// Define an enum to indicate the type of property value
enum class PropertyType {
	Positions,
	Conductor,
	Insulator,
	Transformation
};

bool convertToBoolean(const std::string& value) {
	// Convert string to lowercase for case-insensitive comparison
	std::string lowercaseValue = value;
	std::transform(lowercaseValue.begin(), lowercaseValue.end(), lowercaseValue.begin(),
		[](unsigned char c) { return std::tolower(c); });

	// Check if the string represents true or false
	if (lowercaseValue == "true" || lowercaseValue == "1") {
		return true;
	}
	else if (lowercaseValue == "false" || lowercaseValue == "0") {
		return false;
	}
	else {
		// Handle invalid input
		throw std::invalid_argument("Invalid boolean value: " + value);
	}
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
		//transformation = value;
		// Set the transformation based on the string value
		if (value == "true") {
			transformation = true;
		}
		else if (value == "false") {
			transformation = false;
		}
		else {
			// Handle invalid input
			throw std::invalid_argument("Invalid value for transformation: " + value);
		}
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
		for (const auto& pos : value) {
			c.addPosition(pos.first, pos.second);
		}
	}
	else if (c.isConductor(key)) {
		Conductor conductor; // Create a Conductor object
		// Populate the conductor object using the provided value
		c.addConductor(key, conductor); // Add the conductor to the Cable object
	}
	else if (c.isInsulator(key)) {
		// Handle insulator property
		// Assuming addInsulator accepts a single Insulator object
		Insulator insulator;
		c.addInsulator(key, insulator);
	}
	else if (key == "transformation") {
		// Handle transformation property
		//bool transformationValue = convertToBoolean(value);
		//c.setTransformation(transformationValue);
		if (value.size() == 1) {
			bool transformationValue = value[0].first != 0.0 || value[0].second != 0.0;
			c.setTransformation(transformationValue);
		}
		else {
			throw std::invalid_argument("Invalid value for transformation property: not a boolean string.");
		}
	}
	else {
		throw std::invalid_argument("Unknown cable property name.");
	}
}


void cable(Cable& c, const std::vector<std::vector<double>>& P, const std::vector<std::vector<double>>& Z, const std::unordered_map<std::string, std::vector<std::pair<double, double>>>& kwargs) {


	 // Evaluate PI to a double
	double pi_value = rcp_static_cast<const RealDouble>(PI)->as_double();

	// Iterate through kwargs
	for (const auto& pair : kwargs) {
		const std::string& key = pair.first;
		//const std::vector<std::pair<double, double>>& value = pair.second;
		const auto& value = pair.second;

		handleProperty(c, key, value);


		if (key == "C1") {
			// Access and modify the conductor from the Cable object
			Conductor* conductorPtr = c.getConductor("C1");
			if (conductorPtr != nullptr) {
				double area = conductorPtr->getArea();
				if (area != 0) {
					conductorPtr->setResistivity(conductorPtr->getResistivity() * pi_value * conductorPtr->getOuterRadius() * conductorPtr->getOuterRadius() / area);

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
			auto conductorC1 = c.getConductor("C1");
			if (conductorC1 && conductorC1->getArea() != 0) {
				conductorC1->setResistivity(conductorC1->getResistivity() * pi_value * conductorC1->getOuterRadius() * conductorC1->getOuterRadius() / conductorC1->getArea());
				c.updateConductor("C1", *conductorC1);
			}
		}
		else if (key == "SC") {
			auto conductorSC = c.getConductor("SC");
			auto conductorC2 = c.getConductor("C2");

			if (conductorSC && conductorC2) {
				if (conductorSC->getArea() != 0) {
					c.getConductor("C2")->ri = std::sqrt(conductorSC->ro * conductorSC->ro - conductorSC->area / pi_value);
				}
				else {
					//conductorC2->setInnerRadius(conductorSC->getInnerRadius());
					c.getConductor("C2")->ri = conductorSC->ri;
				}
		
				double c2OuterRadius = std::sqrt((conductorC2->getOuterRadius() * conductorC2->getOuterRadius() - conductorSC->getOuterRadius() * conductorSC->getOuterRadius()) *
					conductorSC->getResistivity() / conductorC2->getResistivity() + conductorSC->getOuterRadius() * conductorSC->getOuterRadius());
				c.getConductor("C2")->ro = c2OuterRadius;

				// Remove the SC conductor
				c.removeConductor("SC");

				// Change Insulator 1
				c.getInsulator("I1")->ro = c.getConductor("C2")->ri;
				//c.getInsulator("I1")->setOuterRadius(conductorC2->getInnerRadius());
				auto insulatorI2 = c.getInsulator("I2");

				// Change Insulator 2 if present
				if (c.getInsulator("I2") != nullptr) {
					double x = log(c.getInsulator("I2")->ro / c.getConductor("C2")->ro) / log(c.getInsulator("I2")->ro / c.getInsulator("I2")->ri);
					c.getInsulator("I2")->ri = c.getConductor("C2")->ro;
					c.getInsulator("I2")->permittivity *= x; // Using 'permittivity' for dielectric constant
					c.getInsulator("I2")->permeability /= x; // Assuming 'permeability' is the member representing permeability
				}

			}
			else {
				throw std::invalid_argument("There must be present sheath together with screen layer.");

			}
		}

		// Semiconductor configuration

		if (c.getInsulator("I1") && c.getInsulator("I1")->ro != 0) {
			double x = log(c.getInsulator("I1")->ro / c.getInsulator("I1")->ri) / log(c.getInsulator("I1")->b / c.getInsulator("I1")->a);
			c.getInsulator("I1")->permittivity *= x;
			double N = 1.4;
			// Assuming 'permeability' represents relative permeability
			c.getInsulator("I1")->permeability *= (1 + 2 * pi_value * pi_value * N * N * (c.getInsulator("I1")->ro * c.getInsulator("I1")->ro - c.getInsulator("I1")->ri * c.getInsulator("I1")->ri) / log(c.getInsulator("I1")->ro / c.getInsulator("I1")->ri));
		}
	}
	// Ground parameters
	double mu_0 = 4 * pi_value * 1e-7; // Vacuum permeability
	double epsilon_0 = 8.85e-12;    // Vacuum permittivity
	auto earthParams = c.getEarthParameters(); //(μᵣ, ϵᵣ, ρ) in units([], [], [Ωm]) compact way of representing the type for a tuple of length N where all elements are of type Int or Float64.
	double mu_g = std::get<0>(earthParams) * mu_0; // Ground permittivity
	double epsilon_g = std::get<1>(earthParams) * epsilon_0; // Ground permeability
	double sigma_g = 1 / std::get<2>(earthParams); // Ground conductivity
	double rho_g = 1 / sigma_g; // Ground resistivity
	double gamma = 0.5772156649;
	double g = 1e-11;


	// Define variables for frequency domain implementation
	auto& conductors = c.getCableConductors();
	auto& insulators = c.getCableInsulators();
	auto& positions = c.getPositions();

	double d_ij = 0; // Initialize d_ij

	// Define variables for frequency domain implementation
	size_t n_l = conductors.size(); // Number of cable layers
	size_t n = positions.size();    // Number of cables

	// Create vec_basic objects for P and Z matrices
	vec_basic P_values, Z_values;

	size_t i = 0; // External indicator

	double s_value = 1.0; // Default value for the symbolic variable 's'

	for (const auto& pair : conductors) {
		const auto& conductor = pair.second;
		double r_i = conductor.ri;
		double r_o = conductor.ro;
		double mu = conductor.permeability * mu_0;
		double rho = conductor.resistivity;

		// Define symbols
		SymEngine::Symbol s("s");
		SymEngine::Symbol symbolic_mu("mu");
		SymEngine::Symbol symbolic_rho("rho");

		// Create symbolic expressions for each symbol
		auto sym_s = SymEngine::symbol(s.get_name());
		auto sym_mu = SymEngine::symbol(symbolic_mu.get_name());
		auto sym_rho = SymEngine::symbol(symbolic_rho.get_name());

		// Compute the product of s, symbolic_mu, and symbolic_rho
		auto product = SymEngine::mul({ sym_s, sym_mu, sym_rho });

		// Define a map to hold the values of symbolic variables
		std::map<std::string, double> symbols;



		// Set the values of symbolic variables in the map
		symbols[s.get_name()] = s_value; // Assuming s_value is the value of the symbol 's'
		symbols[symbolic_mu.get_name()] = mu; // Assuming mu is the permeability value
		symbols[symbolic_rho.get_name()] = rho; // Assuming rho is the resistivity value

		// Evaluate the product expression with the symbol values
		try {
			std::cout << "Expression to be evaluated: " << *product << std::endl;

			double product_value = SymEngine::eval_double(*product);
			double m = std::sqrt(s_value * mu / rho); // Calculate m
			double delta_r = r_o - r_i; // Calculate Δr
			double Z_aa, Z_bb, Z_ab;

			if (r_i != 0) {
				Z_aa = rho * m / (2 * pi_value * r_i) * std::tanh(m * delta_r) - rho / (2 * pi_value * r_i * (r_i + r_o));
				Z_bb = rho * m / (2 * pi_value * r_o) * std::tanh(m * delta_r) + rho / (2 * pi_value * r_o * (r_i + r_o));
			}
			else {
				Z_bb = rho * m / (2 * pi_value * r_o) * std::tanh(0.733 * m * r_o) + 0.3179 * rho / (pi_value * r_o * r_o);
			}
			Z_ab = rho * m / (pi_value * (r_o + r_i)) * (1.0 / sinh(m * delta_r));

			c.Z[i][i] += Z_bb;
			//i++;


			if (i > 0) {
				c.Z[i][i - 1] += -Z_ab; // Eq. 44 from simulator tutorial, not considering the insulator material Zi
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
				double Z_g = s_value * mu_g / (2 * pi_value) * (-log(gamma * m * d_ij / 2) + 0.5 - 2 * m * H / 3);
				c.Z[i][i] += Z_g;
			}
			i++;
		}
		catch (const SymEngine::SymEngineException &e) {
			// Catch any SymEngine exceptions and print the error message
			std::cerr << "SymEngine exception caught: " << e.what() << std::endl;
		}
		catch (const std::exception &e) {
			// Catch any other exceptions and print the error message
			std::cerr << "Exception caught: " << e.what() << std::endl;
		}
	}

	// make shunt admittance -> Insulator
	// Define variables for shunt admittance calculation
	int insulatorIndex = 0; // Re-assign the value 0 to i
	for (const auto& insulatorPair : c.getCableInsulators()) {
		const Insulator& insulator = insulatorPair.second;
		double r_i = insulator.ri;
		double r_o = insulator.ro;
		double mu = insulator.permeability * mu_0;
		double epsilon = insulator.permittivity * epsilon_0;
		// Assuming P is declared as a 2D vector

		double Z_i = s_value * mu / (2 * pi_value) * log(r_o / r_i); // Insulator layer impedance
		double P_i = log(r_o / r_i) / (2 * pi_value * epsilon); // P expression

		c.Z[insulatorIndex][insulatorIndex] += Z_i; // For the impedance matrix diagonal values, add also the insulator impedance
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

	for (int i = 1; i <= n; ++i) {
		for (int j = 1; j <= n; ++j) {
			// Copy and translate the same conductor impedance matrix
			for (int k = 1; k <= n_l; ++k) {
				for (int l = 1; l <= n_l; ++l) {
					c.Z[(i - 1) * n_l + k][(j - 1) * n_l + l] = Z[k][l];
					c.P[(i - 1) * n_l + k][(j - 1) * n_l + l] = P[k][l];
				}
			}

			// Adding earth return impedance and mutual impedance between cables
			if (j > i) { // Only calculate for distinct pairs (i, j) where j > i
				double m = sqrt(s_value * mu_g / rho_g);
				double H = c.getPositions()[i].second + c.getPositions()[j].second;
				double dᵢⱼ = sqrt(pow(c.getPositions()[i].first - c.getPositions()[j].first, 2) +
					pow(c.getPositions()[i].second - c.getPositions()[j].second, 2));
				double x = abs(c.getPositions()[i].first - c.getPositions()[j].first);
				double Z_g = s_value * mu_g / (2 * pi_value) * (-log(gamma * m * d_ij / 2) + 0.5 - 2 * m * H / 3);

				c.Z[i * n_l][j * n_l] += Z_g;
				c.Z[j * n_l][i * n_l] += Z_g;
			}
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
						//c.Z[static_cast<int>(l * n_l) + p][static_cast<int>(k * n_l) + j] += Z[static_cast<int>(l * n_l) + p][static_cast<int>(k * n_l) + i + 1];
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

	//if (c.getType == "underground") {
	std::string expectedType = "underground";
	std::string cableType = c.getType();

	//if (c.getType() == "underground") {
	if (cableType == expectedType) {
		std::cout << "The cable type matches the expected value.\n";

		for (int i = 1; i < n; ++i) {
			for (int j = 0; j < n; ++j) {
				double H = positions[i].second + positions[j].second; // H = sum of depth of ith and jth cables
				double x = abs(positions[i].first - positions[j].first);
				double y = abs(positions[i].second - positions[j].second);
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
				double P_ij = log(D2 / D1) / (2 * pi_value * epsilon_0);
				for (int k = i * n_l; k < (i + 1) * n_l; ++k) {
					for (int l = j * n_l; l < (j + 1) * n_l; ++l) {
						c.P[k][l] += P_ij;
					}
				}
			}
		}
	}
	else {
		std::cout << "The cable type does not match the expected value.\n";
	}

	// Assuming c.P and c.Z are matrices in C++
	// Copy P and Z matrices to c.P and c.Z
	c.setP(P); // Assign P matrix to c.P
	c.setZ(Z); // Assign Z matrix to c.Z

	std::unordered_map<std::string, std::string> args;
	args["symbol"] = "some_symbol";       // Set the symbol
	args["input_pins"] = "4";             // Set the number of input pins
	args["output_pins"] = "2";            // Set the number of output pins
	args["element_value"] = "42";         // Optional: Example value for element_value

	//args["transformation"] = "true";

	//Element elem(args);

	// Assuming `c` is an instance of the `Cable` class and `elem` is an instance of the `Element` class
	//Element elem(args);
		// Create an Element object using the map
	try {
		// Create an Element object using the map
		Element elem(args);

		// Access element_value if needed, safely cast to the expected type
		if (elem.element_value != nullptr) {
			int* val = static_cast<int*>(elem.element_value);
			std::cout << "Element Value: " << *val << std::endl;
		}

		elem.printElementInfo();  // Print the element's info to verify it's correct
	}
	catch (const std::exception& e) {
		std::cerr << "Error while creating Element: " << e.what() << std::endl;
	}

}
// Destructor definition
Cable::~Cable() {
	// Implement the destructor if needed
}


