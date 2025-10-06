#ifndef _IMPEDANCE_BASE_H_
#define _IMPEDANCE_BASE_H_

#include "../Element.h"

class Impedance_base : public Element {
public:
    Impedance_base(const string& symbol, const std::string& location, int pins1, int pins2) : Element(symbol, location, pins1, pins2) {}
    ~Impedance_base() {}

    // Power flow calculations (AC and DC)
    void computePowerFlow(std::map<std::string, double>& branchData,
		std::map<std::string, double>& globalParams) const override;


private:
};


#endif
