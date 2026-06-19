#ifndef WP_PLANT_H_
#define WP_PLANT_H_

/**
 * @file WP_plant.h
 * @brief Wind power plant aggregating one or more wind turbine models.
 */

#include "RES_base.h"

/**
 * @class WPplant
 * @brief Wind power plant composed of multiple wind turbine instances.
 * @ingroup res
 */
class WPplant : public RES_base {
public:
	/**
	 * @brief Construct a wind power plant with the given turbine type and count.
	 * @param symbol Element identifier.
	 * @param location Network area or location string.
	 * @param turbine_type Wind turbine type identifier (3 or 4).
	 * @param number_WT Number of wind turbines in the plant.
	 * @param parameters Packed plant and turbine parameters.
	 */
	WPplant(const string& symbol, const std::string& location, int turbine_type, int number_WT, const vector<double>& parameters);
	~WPplant() override;
private:
	RES_base* wind_turbine = nullptr;
};

#endif // !WP_PLANT_H_
