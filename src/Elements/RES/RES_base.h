#ifndef HARMONY_ELEMENTS_RES_RES_BASE_H
#define HARMONY_ELEMENTS_RES_RES_BASE_H

/**
 * @file RES_base.h
 * @brief Base class for renewable energy source (RES) plant models.
 */

#include "Elements/Element.h"

/**
 * @class RES_base
 * @brief Common base for wind and PV plant elements.
 * @ingroup res
 */
class RES_base : public Element {
public:
	/**
	 * @brief Construct a three-phase RES element.
	 * @param symbol Element identifier.
	 * @param location Network area or location string.
	 */
	RES_base(const string& symbol, const std::string& location) : Element(symbol, location, 3, 3) {}

	~RES_base() override = default;
	void computePowerFlow(std::map<std::string, double>& branchData,
		std::map<std::string, double>& globalParams) const override;
};


#endif // HARMONY_ELEMENTS_RES_RES_BASE_H
