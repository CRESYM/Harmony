#ifndef _LOAD_BASE_H_
#define _LOAD_BASE_H_

/**
 * @file Load_base.h
 * @brief Base class for load elements connected to network buses.
 */

#include "../Element.h"

/**
 * @class Load_base
 * @brief Common base for load models with power-flow support.
 * @ingroup source
 */
class Load_base : public Element {
public:
	/**
	 * @brief Construct a load element with symmetric pin topology.
	 * @param symbol Element identifier.
	 * @param location Network area or location string.
	 * @param pins1 Number of input pins.
	 * @param pins2 Number of output pins.
	 */
	Load_base(const string& symbol, const std::string& location, int pins1, int pins2) : Element(symbol, location, pins1, pins2) {}

	~Load_base() {}
	// Power flow calculations (AC and DC)
	void computePowerFlow(std::map<std::string, double>& branchData,
		std::map<std::string, double>& globalParams) const override;
};

#endif
