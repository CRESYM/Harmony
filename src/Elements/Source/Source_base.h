#ifndef _SOURCE_BASE_H
#define _SOURCE_BASE_H

#include "../Element.h"

// Base class for AC and DC sources, containing common properties and methods
class Source_base : public Element {
public:
	Source_base(const std::string& symbol, const std::string& location, int pins) : Element(symbol, location, pins, pins) {}
	~Source_base() {}

    // Power flow computations for AC and DC networks
    void computePowerFlow(std::map<std::string, double>& branchData,
        std::map<std::string, double>& globalParams) const override;

	double getZsrc() const { return Zsrc.empty() ? 0 : Zsrc[0]; }
	double getVg() const { return V.empty() ? 0 : V[0]; }
protected:
	vector<double> Zsrc = {};               // Internal source impedance [Ohms]
    vector<double> V = {};                  // Voltage amplitude or DC voltage [kV]
    vector<double> theta = {};              // Phase shift, used only for AC sources [radians]

    // Properties used for power flow
    double P;      // Active power output [MW]
    double Q;      // Reactive power output [MVAr]
    double P_min;  // Min active power output [MW]
    double P_max;  // Max active power output [MW]
    double Q_min;  // Min reactive power output [MVA]
    double Q_max;  // Max reactive power output [MVA]
};

#endif
