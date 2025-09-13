#ifndef _LOAD_BASE_H_
#define _LOAD_BASE_H_

#include "../Element.h"

class Load_base : public Element {
public:
	Load_base(const string& symbol, const std::string& location, int pins1, int pins2) : Element(symbol, location, pins1, pins2) {}

	~Load_base() {}
	// Power flow calculations (AC and DC)
	void computePowerFlowDC(std::map<std::string, double>& branchDCData,
		std::map<std::string, double>& globalParams) const override;

	void computePowerFlowAC(std::map<std::string, double>& branchData,
		std::map<std::string, double>& globalParams) const override;
};

#endif
