#ifndef _DC_SOURCE_H_
#define _DC_SOURCE_H_

#include "Source_base.h"

class DC_source : public Source_base {
public:
	DC_source(const std::string& symbol, const std::string& location, int pins, double V, double R);
	DC_source(const std::string& symbol, const std::string& location, int pins, const vector<double>& V, double R);
	DC_source(const std::string& symbol, const std::string& location, int pins, double V, const vector<double>& Z);
	DC_source(const std::string& symbol, const std::string& location, int pins, const vector<double>& V, const vector<double>& Z);

	~DC_source() {}

};

#endif