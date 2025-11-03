#ifndef _RES_BASE_H_
#define _RES_BASE_H_

#include "../Element.h"

class RES_base : public Element {
public:
	RES_base(const string& symbol, const std::string& location) : Element(symbol, location, 3, 3) {}

	~RES_base() = default;
	void computePowerFlow(std::map<std::string, double>& branchData,
		std::map<std::string, double>& globalParams) const override;
};


#endif
