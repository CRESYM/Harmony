#ifndef TRANSMISSIONLINE_H
#define TRANSMISSIONLINE_H

/**
 * @file Transmissionline.h
 * @brief Lumped-parameter transmission line model with distributed R, L, G, C.
 */

#include "../Element.h"

class Element; // Forward declaration of Element class

/**
 * @class TransmissionLine
 * @brief Lumped transmission line with per-unit-length R, L, G, C parameters.
 * @ingroup transmission
 */
class TransmissionLine : public Element {
public:
    /**
     * @brief Construct a transmission line from a parameter vector.
     * @param symbol Element identifier.
     * @param location Network area or location string.
     * @param pins Number of pins (phases).
     * @param values Per-unit-length R, L, G, C and line length values.
     */
	TransmissionLine(const std::string& symbol, const std::string& location, int pins, const std::vector<double>&);

    ~TransmissionLine() {}

	void computePowerFlow(std::map<std::string, double>& branchData,
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

