#ifndef _HELPER_FUNCTIONS_H_
#define _HELPER_FUNCTIONS_H_

/**
 * @file Helper_Functions.h
 * @brief Umbrella header aggregating all solver helper modules.
 *
 * Includes standard numeric utilities, symbolic algebra helpers,
 * differential-equation integrators, file I/O, and DQsym domain-conversion
 * functions. Plotting headers are not pulled in here; include
 * `ui/Visualization.h` from translation units that draw plots.
 */

#include "Standard_functions.h"
#include "Symbolic_functions.h"
#include "Differential_equations.h"
#include "Writer.h"
#include "DQsym_Conversion_Functions.h"

#endif
