#ifndef _INCLUDE_CONTROL_BLOCKS_H
#define _INCLUDE_CONTROL_BLOCKS_H

/**
 * @file Include_control_blocks.h
 * @brief Umbrella header for control blocks used by converters.
 * @ingroup control
 *
 * Includes @ref ControlBlock, @ref Controller and its P/PI subclasses,
 * @ref Filter, and @ref Integrator.
 */

#include "./Control/Control_block.h"

#include "./Control/Controller/Controller.h"
#include "./Control/Controller/Proportional_controller.h"
#include "./Control/Controller/ProportionalIntegral_controller.h"

#include "./Control/Filter.h"
#include "./Control/Integrator.h"

#endif