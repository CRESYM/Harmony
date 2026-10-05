/**
 * @file example_transformer.cpp
 * @brief Runnable example: real transformer topologies and Y-parameters.
 */
#include "Examples.h"

#include "network/network.h"
#include "network/Bus.h"
#include "core/Include_components.h"

void example_transformer() {
	std::vector<double> transformer_values = { 4.3218, 0.0, 0.7938, 0.084225, 2.0, 0.0 };

	TransformerYY_real transformerYY("T_YY", "AC1", 3, transformer_values);
	TransformerYDelta_real transformerYD("T_YD", "AC1", 3, transformer_values);
	TransformerDeltaY_real transformerDY("T3", "AC1", 3, transformer_values);
	TransformerDeltaDelta_real transformerDD("T_DD", "AC1", 3, transformer_values);

	transformerYY.writeFile(10, 10000, 1000);
	transformerYD.writeFile(10, 10000, 1000);
	transformerDY.writeFile(10, 10000, 1000);
	transformerDD.writeFile(10, 10000, 1000);
}
