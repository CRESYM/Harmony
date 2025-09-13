#ifndef TRANSMISSIONLINE_H
#define TRANSMISSIONLINE_H

#include "../Element.h"

class Element; // Forward declaration of Element class

class TransmissionLine : public Element {
public:
    // Parameterized constructor that calls the base class constructor
	TransmissionLine(const std::string& symbol, const std::string& location, int pins, const std::vector<double>&);

    ~TransmissionLine() {}

	void computePowerFlowAC(std::map<std::string, double>& branchData,
		const std::map<std::string, double>& global) const;

	void computePowerFlowDC(std::map<std::string, double>& branchDCData,
		const std::map<std::string, double>& global) const;

   // void compute_y_parameters(double R, double L, double G, double C, double length);
private:
	double R_tl = 0.01;       // Resistance per unit length (ohms/m)
	double L_tl = 2.5e-7;     // Inductance per unit length (H/m)
	double G_tl = 1e-9;       // Conductance per unit length (S/m)
	double C_tl = 1e-11;      // Capacitance per unit length (F/m)
	double length = 1000;     // Length of the transmission line (m)

protected:
	int m_pins = 0;
};

#endif

