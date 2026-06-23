/**
 * @file json_expression.cpp
 * @brief SymEngine expression parsing for JSON component definitions.
 */
#include "json_expression.h"

#include "Solver/Helper_Functions/Symbolic_functions.h"

#include <set>
#include <stdexcept>

#include <symengine/parser.h>
#include <symengine/symbol.h>
#include <symengine/visitor.h>

using SymEngine::Basic;
using SymEngine::Symbol;
using SymEngine::down_cast;
using SymEngine::is_a;
using SymEngine::DenseMatrix;
using SymEngine::RCP;
using SymEngine::set_basic;


namespace {

std::map<const std::string, const RCP<const Basic>> parserConstants() {
	return {
		{ "w", omega },
		{ "omega", omega },
		{ "s", s },
		{ "j", j },
		{ "pi", PI },
		{ "I", j },
	};
}

bool isAllowedFrequencySymbol(const RCP<const Basic>& sym) {
	if (!is_a<Symbol>(*sym)) {
		return false;
	}
	const std::string name = down_cast<const Symbol&>(*sym).get_name();
	return name == "w" || name == "omega" || name == "s";
}

void ensureOnlyFrequencySymbolsRemain(const RCP<const Basic>& expr, const char* context) {
	const set_basic symbols = free_symbols(*expr);
	for (const auto& sym : symbols) {
		if (!isAllowedFrequencySymbol(sym)) {
			const std::string name = down_cast<const Symbol&>(*sym).get_name();
			throw std::invalid_argument(
				std::string("ERROR: unresolved symbol '") + name + "' in " + context
				+ " (define it in 'parameters' or use w/s).\n");
		}
	}
}

RCP<const Basic> parseAndSubstitute(
	const std::string& exprText,
	const JsonParameterTable& params,
	const char* context)
{
	if (exprText.empty()) {
		throw std::invalid_argument(
			std::string("ERROR: empty expression in ") + context + ".\n");
	}

	RCP<const Basic> expr;
	try {
		expr = SymEngine::parse(exprText, true, parserConstants());
	}
	catch (const std::exception& ex) {
		throw std::invalid_argument(
			std::string("ERROR: failed to parse expression in ") + context + ": "
			+ ex.what() + "\n");
	}

	if (!params.names().empty()) {
		expr = expr->subs(params.toSubstitutionMap());
	}

	ensureOnlyFrequencySymbolsRemain(expr, context);
	return expr;
}

} // namespace


RCP<const Basic> JsonExpression::parse(
	const std::string& exprText,
	const JsonParameterTable& params,
	const char* context)
{
	return parseAndSubstitute(exprText, params, context);
}


RCP<const Basic> JsonExpression::parseImpedance(
	const std::string& zExpr,
	const JsonParameterTable& params,
	const char* context)
{
	return parseAndSubstitute(zExpr, params, context);
}


RCP<const Basic> JsonExpression::parseAdmittance(
	const std::string& yExpr,
	const JsonParameterTable& params,
	const char* context)
{
	return parseAndSubstitute(yExpr, params, context);
}


std::vector<RCP<const Basic>> JsonExpression::parseArray(
	const JSON& array,
	const JsonParameterTable& params,
	const char* context)
{
	if (!array.is_array() || array.empty()) {
		throw std::invalid_argument(
			std::string("ERROR: expected non-empty expression array in ") + context + ".\n");
	}
	std::vector<RCP<const Basic>> out;
	out.reserve(array.size());
	for (size_t i = 0; i < array.size(); ++i) {
		validateExprField(array.at(i), context);
		out.push_back(parseAndSubstitute(array.at(i).get<std::string>(), params, context));
	}
	return out;
}


void JsonExpression::validateExprField(const JSON& value, const char* context) {
	if (!value.is_string() || value.get<std::string>().empty()) {
		throw std::invalid_argument(
			std::string("ERROR: expected non-empty expression string in ") + context + ".\n");
	}
}


void JsonExpression::validateExprArray(
	const JSON& array,
	const JsonParameterTable& params,
	const char* context,
	const int expectedSize)
{
	if (!array.is_array() || array.empty()) {
		throw std::invalid_argument(
			std::string("ERROR: expected non-empty expression array in ") + context + ".\n");
	}
	for (size_t i = 0; i < array.size(); ++i) {
		validateExprField(array.at(i), context);
	}
	if (expectedSize != -1 && static_cast<int>(array.size()) != expectedSize) {
		throw std::runtime_error(
			std::string("ERROR: ") + context + " must have exactly "
			+ std::to_string(expectedSize) + " elements.\n");
	}
	// Parse eagerly so validation catches syntax and unknown symbols.
	parseArray(array, params, context);
}

SymEngine::DenseMatrix JsonExpression::toRowMatrix(
	const std::vector<RCP<const Basic>>& exprs)
{
	DenseMatrix matrix(1, static_cast<unsigned>(exprs.size()));
	for (size_t i = 0; i < exprs.size(); ++i) {
		matrix.set(0, static_cast<unsigned>(i), exprs[i]);
	}
	return matrix;
}
