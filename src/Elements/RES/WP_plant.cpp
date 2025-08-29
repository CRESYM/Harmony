#include "WP_plant.h"
#include "WT_type_3.h"
#include "WT_type_4.h"

WPplant::WPplant(const string& symbol, int turbine_type, int number_WT, const vector<double>& parameters) : RES_base(symbol) {
	// Default parameters for a WP plant can be set here if needed
	if (turbine_type == 3) {
		// Initialize WT type 3 parameters
		WTtype3* wt = new WTtype3(symbol, parameters);
		Y_matrix = wt->get_y_parameters(); // Example of using WTtype3's Y_matrix
		wind_turbine = wt;
	}
	else if (turbine_type == 4) {
		// Initialize WT type 4 parameters
		WTtype4* wt = new WTtype4(symbol, parameters);
		Y_matrix = wt->get_y_parameters(); // Example of using WTtype4's Y_matrix
		wind_turbine = wt;
	}
	else {
		throw std::invalid_argument("Unsupported turbine type. Only types 3 and 4 are supported.");
	}
	// Scale Y_matrix by the number of wind turbines
	mul_dense_scalar(Y_matrix, integer(number_WT), Y_matrix);
}