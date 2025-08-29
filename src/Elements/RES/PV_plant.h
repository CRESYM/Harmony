#ifndef PV_PLANT_H_
#define PV_PLANT_H_

#include "RES_base.h"

class PVplant : public RES_base {
public:
	PVplant(const string& symbol, const vector<double>& parameters);
	~PVplant() {}
};

#endif  // PV_PLANT_H_