#ifndef _SOURCE_BASE_H
#define _SOURCE_BASE_H

#include "../Element.h"

class Source_base : public Element {
public:
	Source_base(const std::string& symbol, const std::string& location, int pins, double V) : Element(symbol, location, pins, pins), Vg(V) {}
	~Source_base() {}

    // Power flow computations for AC and DC networks
    void computePowerFlow(std::map<std::string, double>& branchData,
        std::map<std::string, double>& globalParams) const override;

	double getZsrc() const { return Zsrc; }
	double getVg() const { return Vg; }
protected:
	double Zsrc = 0;    // Internal source impedance [Ohms]
    double Vg;          // Voltage amplitude [kV]
    double theta;       // Phase shift [radians]

    // Properties used for power flow
    double P;      // Active power output [MW]
    double Q;      // Reactive power output [MVAr]
    double P_min;  // Min active power output [MW]
    double P_max;  // Max active power output [MW]
    double Q_min;  // Min reactive power output [MVA]
    double Q_max;  // Max reactive power output [MVA]
};

#endif
