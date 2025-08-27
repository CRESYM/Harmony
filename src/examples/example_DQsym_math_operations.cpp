#include "Examples.h"

#include "../Solver/DQsym/DQsym.h"

void example_DQsym_math_operations()
{
	// Example usage of DQsym class for mathematical operations
	DQsym dqSym;
	vector<complex<double>> vec1 = { {1, 2}, {3, 4}, {5, 6} };
	vector<complex<double>> vec2 = { {7, 8}, {9, 10} };
	vector<complex<double>> result = dqSym.add(vec1, vec2);
	cout << "Result of addition:" << endl;
	for (const auto& val : result) {
		cout << val << " ";
	}
	cout << endl;
}