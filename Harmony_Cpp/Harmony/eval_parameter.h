#ifndef EVAL_PARAMETER_H
#define EVAL_PARAMETER_H

//class CMapBasicBasic;

#include <iostream>
#include <vector>

#include "Cable.h"
#include "overhead_line.h"
#include <symbol.h>
#include <basic.h>
#include <real_double.h>
#include <complex.h>

using namespace SymEngine;

class Eval_parameter : public Element {
public:
	std::pair<std::vector<std::vector<std::complex<double>>>, std::vector<std::vector<std::complex<double>>>> eval_parameters(const Cable& c, const std::complex<double>& s_param);

private:
	RCP<const Basic> substitute_symbol(const RCP<const Basic>& expr, const std::string& symbol_name, double value) {
		RCP<const Symbol> symbol = SymEngine::symbol(symbol_name);
		RCP<const Basic> value_expr = real_double(value);
		map_basic_basic subs_map;
		subs_map[symbol] = value_expr;
		return expr->subs(subs_map);
	}

	double eval_basic(const RCP<const Basic>& expr) {
		try {
			return eval_double(*expr);
		}
		catch (const SymEngineException& e) {
			std::cerr << "SymEngineException: " << e.what() << std::endl;
			throw;
		}
		catch (const std::exception& e) {
			std::cerr << "Standard exception: " << e.what() << std::endl;
			throw;
		}
	}
};

#endif //EVAL_PARAMETER_H

