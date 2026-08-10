#pragma once

#include <string>
#include <vector>

#include <symengine/basic.h>
#include <symengine/matrix.h>

#include "json_parameters.h"

/**
 * @file json_expression.h
 * @brief SymEngine expression parsing for JSON component definitions (Type B).
 */

class JsonExpression {
public:
	/** @brief Parse an expression, substitute numeric parameters, leave @c w / @c s symbolic. */
	static SymEngine::RCP<const SymEngine::Basic> parse(
		const std::string& exprText,
		const JsonParameterTable& params,
		const char* context);

	/** @brief Parse a complex impedance expression @c Z(s). */
	static SymEngine::RCP<const SymEngine::Basic> parseImpedance(
		const std::string& zExpr,
		const JsonParameterTable& params,
		const char* context);

	/** @brief Parse an admittance expression @c Y(s). */
	static SymEngine::RCP<const SymEngine::Basic> parseAdmittance(
		const std::string& yExpr,
		const JsonParameterTable& params,
		const char* context);

	/** @brief Parse a JSON array of expression strings. */
	static std::vector<SymEngine::RCP<const SymEngine::Basic>> parseArray(
		const JSON& array,
		const JsonParameterTable& params,
		const char* context);

	static void validateExprField(const JSON& value, const char* context);

	static void validateExprArray(
		const JSON& array,
		const JsonParameterTable& params,
		const char* context,
		int expectedSize = -1);

	/** @brief Pack parsed expressions into a 1×N SymEngine matrix. */
	static SymEngine::DenseMatrix toRowMatrix(
		const std::vector<SymEngine::RCP<const SymEngine::Basic>>& exprs);
};
