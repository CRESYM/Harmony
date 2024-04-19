#ifndef EVAL_PARAMETER_H
#define EVAL_PARAMETER_H

#include <iostream>
#include <cmath>
#include <unordered_map>
#include <vector>
#include <utility>
#include <string>

#include "Cable.h"
#include "overhead_line.h"

class Eval_parameter : public Element{
public:
	std::pair<std::vector<std::vector<Complex>>, std::vector<std::vector<Complex>>> eval_parameters(const Cable& c, const Complex& s);
	std::pair<std::vector<std::vector<Complex>>, std::vector<std::vector<Complex>>> eval_parameters(const overhead_Line& t, const Complex& s);
};

#endif //EVAL_PARAMETER_H

