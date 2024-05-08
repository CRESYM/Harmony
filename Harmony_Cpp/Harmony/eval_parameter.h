#ifndef EVAL_PARAMETER_H
#define EVAL_PARAMETER_H

//class CMapBasicBasic;

#include <iostream>
#include <vector>

#include "Cable.h"
#include "overhead_line.h"
#include <symbol.h>
#include <basic.h>
#include <complex.h>


class Eval_parameter : public Element {
public:
	std::pair<std::vector<std::vector<std::complex<double>>>, std::vector<std::vector<std::complex<double>>>> eval_parameters(const Cable& c, const std::complex<double>& s_param);
};

#endif //EVAL_PARAMETER_H

