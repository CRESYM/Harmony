/**
 * @file example_transformer.cpp
 * @brief Runnable example: Transformer topologies and Y-parameters.
 */
#include "Examples.h"

#include "../network.h"
#include "../Bus.h"
#include "../Include_components.h"

void example_transformer() {
	std::vector<double> transformer_values = { 4.3218, 0.0, 0.7938, 0.084225, 2.0, 0.0 };
	TransformerDeltaY_real* transformerDY = new TransformerDeltaY_real("T3", "AC1", 3, transformer_values);

	transformerDY->writeFile(10, 10000, 1000);
}