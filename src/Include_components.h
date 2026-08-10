#ifndef INCLUDE_COMPONENTS_H
#define INCLUDE_COMPONENTS_H

/**
 * @file Include_components.h
 * @brief Umbrella header for network element models and the state-space model.
 *
 * Includes impedance elements, loads, switches, transformers, sources,
 * transmission lines, converters, renewable plants, and @ref State_Space_Model.
 */

// ELEMENTS
#include "./Elements/Element.h"

// Impedances
#include "./Elements/Impedance/Impedance_base.h"
#include "./Elements/Impedance/Admittance.h"
#include "./Elements/Impedance/Impedance.h"
#include "./Elements/Impedance/Resistor.h"
#include "./Elements/Impedance/Capacitor.h"
#include "./Elements/Impedance/Inductor.h"

// Loads
#include "./Elements/Load/Load_base.h"
#include "./Elements/Load/Load.h"
#include "./Elements/Load/Load_PQ.h"

// Switches and Breakers
#include "./Elements/Switch/Switch.h"

// Transformers
#include "./Elements/Transformer/Transformer_base.h"
#include "./Elements/Transformer/Transformer_real.h"
#include "./Elements/Transformer/Transformer_classic.h"
#include "./Elements/Transformer/Transformer_Delta_Delta.h"
#include "./Elements/Transformer/Transformer_Y_Delta.h"
#include "./Elements/Transformer/Transformer_Delta_Y.h"
#include "./Elements/Transformer/Transformer_Delta_Y_real.h"
#include "./Elements/Transformer/Transformer_Y_Y.h"
#include "./Elements/Transformer/Transformer_Y_Y_real.h"

// Sources
#include "./Elements/Source/AC_source.h"
#include "./Elements/Source/Generator.h"
#include "./Elements/Source/DC_source.h"

// Lines
#include "./Elements/Transmission_Line/Transmissionline.h"
#include "./Elements/Transmission_Line/overhead_line.h"
#include "./Elements/Transmission_Line/Cable.h"

// Power Electronics
#include "./Elements/Converter/Converter.h"
#include "./Elements/Converter/MMC.h"
//#include "./Elements/Converter/Simple_MMC.h"

// RES
#include "./Elements/RES/RES_base.h"
#include "./Elements/RES/WT_type_3.h"
#include "./Elements/RES/WT_type_4.h"
#include "./Elements/RES/WP_plant.h"
#include "./Elements/RES/PV_plant.h"

#include "./Solver/State_Space_Model/State_Space_Model.h"


#endif // !INCLUDE_COMPONENTS_H





