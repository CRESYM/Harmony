#ifndef _CONTROL_BLOCK_H_
#define _CONTROL_BLOCK_H_

#include "../Constants.h"

/**
 * @file Control_block.h
 * @brief Base class for dynamic control blocks used by converters.
 * @ingroup control
 */

/**
 * @brief Abstract base for filters, integrators, and controllers.
 *
 * Derived classes override @ref printValues to expose block-specific parameters.
 */
class ControlBlock {
public:
	ControlBlock() = default;

	/** @brief Print block-specific parameter values to standard output. */
	virtual void printValues() {};

	virtual ~ControlBlock() = default;
};

#endif // !_CONTROL_BLOCK_H_
