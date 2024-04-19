#include "Kron.h"
#include "symengine_n.h"
#include <algorithm>

void Kron::eliminate(std::vector<std::vector<Complex>>& matrix, const std::vector<int>& no_eliminate) {
	int n = matrix.size();
	std::vector<int> eliminate;
	for (int i = 0; i < n; ++i) {
		if (std::find(no_eliminate.begin(), no_eliminate.end(), i) == no_eliminate.end()) {
			eliminate.push_back(i);
		}
	}

	std::vector<std::vector<Complex>> newMatrix(n, std::vector<Complex>(n));

	// Reorder the rows and columns
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n; ++j) {
			newMatrix[i][j] = matrix[no_eliminate[i]][no_eliminate[j]];
		}
	}

	// Calculate the eliminated part
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n; ++j) {
			if (std::find(eliminate.begin(), eliminate.end(), i) != eliminate.end() &&
				std::find(eliminate.begin(), eliminate.end(), j) != eliminate.end()) {
				Complex sum(0.0, 0.0); // Initialize sum with zero
				for (int k = 0; k < n; ++k) {
					sum += matrix[i][j + n] * matrix[j + n][j + n] * matrix[j + n][i];
				}
				newMatrix[i][j] -= sum;
			}
		}
	}

	// Update the matrix with the result
	matrix = newMatrix;
}
