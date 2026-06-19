#pragma once
#include <iostream>

#include <nlohmann/json.hpp>
using JSON = nlohmann::json;

#include "../src/network.h"
#include "../src/Include_components.h"

/**
 * @file component_builder.h
 * @brief Factory for constructing @ref Element objects from JSON component entries.
 * @ingroup input
 */

/**
 * @brief Parses JSON component definitions and returns heap-allocated elements.
 *
 * Callers (typically @ref SimulationBuilder or @ref Network) own the returned pointers.
 */
class ComponentBuilder {

public:

	/**
	 * @brief Parse a JSON component object and return a heap-allocated Element.
	 * @param comp JSON object describing one component (`type`, `id`, `values`, …).
	 * @param index Zero-based index in the components array (used in error messages).
	 * @return Pointer to a newly allocated element; caller must manage lifetime.
	 */
	static Element* buildFromJSON(const JSON& comp, unsigned int index);

	/**
	 * @brief Require a numeric JSON field.
	 * @param key Field name.
	 * @param j JSON object to inspect.
	 * @throws std::invalid_argument if the field is missing or not numeric.
	 */
	static void findNumber(const std::string& key, const JSON& j);

	/**
	 * @brief Require a non-empty string JSON field.
	 * @param key Field name.
	 * @param j JSON object to inspect.
	 * @throws std::invalid_argument if the field is missing or empty.
	 */
	static void findNonEmptyString(const std::string& key, const JSON& j);

	/**
	 * @brief Require a non-empty numeric JSON array.
	 * @param key Field name.
	 * @param j JSON object to inspect.
	 * @param arraysize Expected array length, or `-1` to skip length check.
	 * @throws std::invalid_argument if the field is missing, empty, or wrong size.
	 */
	static void findNonEmptyNumericArray(const std::string& key, const JSON& j, int arraysize = -1);

private:

	// Impedance
	static Capacitor* buildCapacitor(const JSON& comp);
	static Inductor* buildInductor(const JSON& comp);
	static Resistor* buildResistor(const JSON& comp);
	// Loads
	static Load* buildLoad(const JSON& comp);
	static LoadPQ* buildLoadPQ(const JSON& comp);
	// Source
	static AC_source* buildACSource(const JSON& comp);
	static Generator* buildGenerator(const JSON& comp);
	// Transformer
	static Transformer_real* buildTransformerReal(const JSON& comp);
	static Transformer_classic* buildTransformerClassic(const JSON& comp);
	static TransformerDeltaDelta* buildTransformerDeltaDelta(const JSON& comp);
	static TransformerDeltaY_real* buildTransformerDeltaYReal(const JSON& comp);
	static TransformerDeltaY* buildTransformerDeltaY(const JSON& comp);
	static TransformerYDelta* buildTransformerYDelta(const JSON& comp);
	static TransformerYY_real* buildTransformerYYReal(const JSON& comp);
	static TransformerYY* buildTransformerYY(const JSON& comp);

	static std::vector<double> readTransformerClassicValues(const JSON& comp);
	static std::vector<double> readTransformerRealValues(const JSON& comp);
};
