#ifndef _LOAD_BASE_H_
#define _LOAD_BASE_H_

#include "../Element.h"

class Load_base : public Element {
public:
	Load_base(const string& symbol, int pins1, int pins2) : Element(symbol, pins1, pins2) {}

	~Load_base() {}
	// Power flow calculations (AC and DC)
	void computePowerFlowDC(std::map<std::string, double>& branchDCData,
		std::map<std::string, double>& globalParams) const override;
};

#endif
