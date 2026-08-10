#pragma once

#include <nlohmann/json.hpp>
#include <set>
#include <string>

#include "json_parameters.h"

using JSON = nlohmann::json;

/**
 * @file json_validator.h
 * @brief Strict per-component JSON schema validation at load time.
 */

class JsonValidator {
public:
	static void validateRoot(const JSON& root);
	static void validateSimulation(const JSON& sim);
	static void validateBus(const JSON& bus, unsigned index);
	static void validateComponent(const JSON& comp, unsigned index, const JsonParameterTable& rootParams);
	static void validateComputation(const JSON& calc, unsigned index);

private:
	static void requireObject(const JSON& j, const char* context);
	static void rejectUnknownKeys(const JSON& j, const std::set<std::string>& allowed, const char* context);
	static void validateByType(const JSON& comp, const std::string& type, const JsonParameterTable& params);
};
