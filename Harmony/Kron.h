#ifndef KRON_H
#define KRON_H

#include <vector>
#include "symengine_n.h"

class Kron {
public:
	static void eliminate(std::vector<std::vector<Complex>>& matrix, const std::vector<int>& no_eliminate);
};

#endif // KRON_H
