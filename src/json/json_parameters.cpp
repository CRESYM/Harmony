/**
 * @file json_parameters.cpp
 * @brief Named numeric parameters for JSON simulation files.
 */
#include "json_parameters.h"

#include <stdexcept>

#include <symengine/basic.h>
#include <symengine/number.h>
#include <symengine/real_double.h>
#include <symengine/symbol.h>


void JsonParameterTable::clear() {
	values_.clear();
}


void JsonParameterTable::mergeFromObject(const JSON& obj, const char* context) {
	validateObject(obj, context);
	for (auto it = obj.begin(); it != obj.end(); ++it) {
		values_[it.key()] = it.value().get<double>();
	}
}


bool JsonParameterTable::contains(const std::string& name) const {
	return values_.find(name) != values_.end();
}


bool JsonParameterTable::isReference(const JSON& value) {
	return value.is_string() && !value.get<std::string>().empty();
}


void JsonParameterTable::validateObject(const JSON& obj, const char* context) {
	if (!obj.is_object()) {
		throw std::invalid_argument(
			std::string("ERROR: expected object for ") + context + ".\n");
	}
	for (auto it = obj.begin(); it != obj.end(); ++it) {
		if (it.key().empty()) {
			throw std::invalid_argument(
				std::string("ERROR: empty parameter name in ") + context + ".\n");
		}
		if (!it.value().is_number()) {
			throw std::invalid_argument(
				"ERROR: parameter '" + it.key() + "' in " + context + " must be numeric.\n");
		}
	}
}


double JsonParameterTable::resolveScalar(const JSON& value, const char* context) const {
	if (value.is_number()) {
		return value.get<double>();
	}
	if (isReference(value)) {
		const std::string name = value.get<std::string>();
		const auto it = values_.find(name);
		if (it == values_.end()) {
			throw std::invalid_argument(
				"ERROR: unknown parameter '" + name + "' in " + context + ".\n");
		}
		return it->second;
	}
	throw std::invalid_argument(
		std::string("ERROR: expected number or parameter name in ") + context + ".\n");
}


std::vector<double> JsonParameterTable::resolveArray(const JSON& array, const char* context) const {
	if (!array.is_array() || array.empty()) {
		throw std::invalid_argument(
			std::string("ERROR: expected non-empty array in ") + context + ".\n");
	}
	std::vector<double> out;
	out.reserve(array.size());
	for (size_t i = 0; i < array.size(); ++i) {
		out.push_back(resolveScalar(array.at(i), context));
	}
	return out;
}


void JsonParameterTable::validateNumericOrReferenceArray(
	const JSON& array,
	const char* context,
	const int expectedSize) const
{
	if (!array.is_array() || array.empty()) {
		throw std::invalid_argument(
			std::string("ERROR: missing, invalid or empty array in ") + context + ".\n");
	}
	for (size_t i = 0; i < array.size(); ++i) {
		const JSON& entry = array.at(i);
		if (entry.is_number()) {
			continue;
		}
		if (isReference(entry)) {
			const std::string name = entry.get<std::string>();
			if (!contains(name)) {
				throw std::invalid_argument(
					"ERROR: unknown parameter '" + name + "' at index " + std::to_string(i)
					+ " in " + context + ".\n");
			}
			continue;
		}
		throw std::invalid_argument(
			"ERROR: array entry at index " + std::to_string(i)
			+ " in " + context + " must be numeric or a parameter name.\n");
	}
	if (expectedSize != -1 && static_cast<int>(array.size()) != expectedSize) {
		throw std::runtime_error(
			std::string("ERROR: ") + context + " must have exactly "
			+ std::to_string(expectedSize) + " elements.\n");
	}
}


SymEngine::map_basic_basic JsonParameterTable::toSubstitutionMap() const {
	SymEngine::map_basic_basic subs;
	for (const auto& [name, value] : values_) {
		subs[SymEngine::symbol(name)] = SymEngine::real_double(value);
	}
	return subs;
}


std::vector<std::string> JsonParameterTable::names() const {
	std::vector<std::string> out;
	out.reserve(values_.size());
	for (const auto& [name, _] : values_) {
		out.push_back(name);
	}
	return out;
}
