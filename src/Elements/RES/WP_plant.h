#ifndef WP_PLANT_H_
#define WP_PLANT_H_

#include "RES_base.h"

class WPplant : public RES_base {
public:
	WPplant(const string& symbol, const std::string& location, int turbine_type, int number_WT, const vector<double>& parameters);
	~WPplant() override;
private:
	RES_base* wind_turbine = nullptr;
};

#endif // !WP_PLANT_H_
