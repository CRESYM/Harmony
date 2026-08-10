#pragma once

#include <map>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include <symengine/basic.h>

using JSON = nlohmann::json;

/**
 * @file json_parameters.h
 * @brief Named numeric parameters for JSON simulation files (Type A indirection).
 */

/** @brief Lookup table for `"parameters": { "name": number }` at simulation or component scope. */
class JsonParameterTable {
public:
	void clear();

	/** @brief Add or override entries from a JSON object of numeric values. */
	void mergeFromObject(const JSON& obj, const char* context);

	bool contains(const std::string& name) const;

	/** @brief Resolve a JSON number or parameter-name string to double. */
	double resolveScalar(const JSON& value, const char* context) const;

	/** @brief Resolve each element of a JSON array (numbers or parameter names). */
	std::vector<double> resolveArray(const JSON& array, const char* context) const;

	/** @brief True when @p value is a non-empty string (parameter reference). */
	static bool isReference(const JSON& value);

	/** @brief Validate object shape: all values must be numbers. */
	static void validateObject(const JSON& obj, const char* context);

	/** @brief Validate array: each entry is a number or known parameter name. */
	void validateNumericOrReferenceArray(
		const JSON& array,
		const char* context,
		int expectedSize = -1) const;

	/** @brief SymEngine substitution map for numeric parameter names. */
	SymEngine::map_basic_basic toSubstitutionMap() const;

	/** @brief All defined parameter names. */
	std::vector<std::string> names() const;

private:
	std::map<std::string, double> values_;
};
