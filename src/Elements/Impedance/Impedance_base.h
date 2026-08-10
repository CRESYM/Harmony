#ifndef _IMPEDANCE_BASE_H_
#define _IMPEDANCE_BASE_H_

/**
 * @file Impedance_base.h
 * @brief Base class for passive impedance-type network elements.
 */

#include "../Element.h"

/**
 * @class Impedance_base
 * @brief Common base for resistive, reactive, and generic admittance elements.
 * @ingroup impedance
 */
class Impedance_base : public Element {
public:
    /**
     * @brief Construct an impedance element with symmetric pin topology.
     * @param symbol Element identifier.
     * @param location Network area or location string.
     * @param pins1 Number of input pins.
     * @param pins2 Number of output pins.
     */
    Impedance_base(const string& symbol, const std::string& location, int pins1, int pins2) : Element(symbol, location, pins1, pins2) {}
    ~Impedance_base() {}

    // Power flow calculations (AC and DC)
    void computePowerFlow(std::map<std::string, double>& branchData,
		std::map<std::string, double>& globalParams) const override;


private:
};


#endif
